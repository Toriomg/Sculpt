#include "Platform/Jobs/TaskQueue.hpp"
#include "Platform/CoreUtils/Log.hpp"
#include <atomic>
#include <condition_variable>
#include <flat_map>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

namespace {

    struct TaskEntry {
        TaskHandle handle;
        std::unique_ptr<ITask> task;
        std::atomic<TaskStatus> status{TaskStatus::Pending};

        TaskEntry(TaskHandle h, std::unique_ptr<ITask> t) : handle(h), task(std::move(t)) { }
    };

    struct TaskQueueData {
        std::vector<std::jthread> workers;

        std::mutex pendingMutex;
        std::condition_variable_any pendingCV;
        std::queue<std::shared_ptr<TaskEntry>> pendingQueue;

        std::mutex completionMutex;
        std::queue<std::shared_ptr<TaskEntry>> completionQueue;

        // Accessed only from the main thread — no synchronization needed.
        std::flat_map<TaskHandle, std::shared_ptr<TaskEntry>> registry;

        std::atomic<uint64_t> nextHandle{1};
    };

    std::optional<TaskQueueData>
        s_Data;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace

void TaskQueue::Init(unsigned int threadCount) {
    s_Data.emplace();

    if (threadCount == 0) { threadCount = std::max(1u, std::thread::hardware_concurrency()); }

    s_Data->workers.reserve(threadCount);
    for (unsigned int i = 0; i < threadCount; ++i) {
        s_Data->workers.emplace_back([](std::stop_token const& stopToken) {
            while (true) {
                std::shared_ptr<TaskEntry> entry;
                {
                    std::unique_lock lock(s_Data->pendingMutex);
                    bool const hasWork = s_Data->pendingCV.wait(
                        lock, stopToken, [] { return !s_Data->pendingQueue.empty(); });
                    if (!hasWork) {
                        return;  // stop requested with nothing left to process
                    }
                    entry = std::move(s_Data->pendingQueue.front());
                    s_Data->pendingQueue.pop();
                }

                entry->status.store(TaskStatus::Running, std::memory_order_relaxed);
                try {
                    entry->task->Execute();
                    entry->status.store(TaskStatus::Completed, std::memory_order_release);
                } catch (std::exception const& ex) {
                    entry->status.store(TaskStatus::Failed, std::memory_order_release);
                    CORE_LOG_ERROR("TaskQueue: '{}' failed: {}", entry->task->GetName(),
                                   ex.what());  // NOLINT(bugprone-lambda-function-name)
                } catch (...) {
                    entry->status.store(TaskStatus::Failed, std::memory_order_release);
                    CORE_LOG_ERROR(
                        "TaskQueue: '{}' threw unknown exception",
                        entry->task->GetName());  // NOLINT(bugprone-lambda-function-name)
                }

                {
                    std::scoped_lock const lock(s_Data->completionMutex);
                    s_Data->completionQueue.push(std::move(entry));
                }
            }
        });
    }

    CORE_LOG_INFO("TaskQueue: initialized with {} worker thread(s)", threadCount);
}

void TaskQueue::Shutdown() {
    if (!s_Data) { return; }

    for (auto& w : s_Data->workers) { w.request_stop(); }
    s_Data->pendingCV.notify_all();
    s_Data->workers.clear();  // join all — no threads access s_Data after this

    // Finalize tasks that completed during the last frames but weren't yet drained.
    ProcessCompletions();

    s_Data.reset();
    CORE_LOG_INFO("TaskQueue: shutdown");
}

std::expected<TaskHandle, std::string> TaskQueue::SubmitTask(std::unique_ptr<ITask> task) {
    if (!s_Data) { return std::unexpected(std::string("TaskQueue not initialized")); }

    TaskHandle handle = s_Data->nextHandle.fetch_add(1, std::memory_order_relaxed);
    auto entry        = std::make_shared<TaskEntry>(handle, std::move(task));

    CORE_LOG_TRACE("TaskQueue: submitting '{}' (handle {})", entry->task->GetName(), handle);

    s_Data->registry.emplace(handle, entry);

    {
        std::scoped_lock const lock(s_Data->pendingMutex);
        s_Data->pendingQueue.push(std::move(entry));
    }
    s_Data->pendingCV.notify_one();

    return handle;
}

void TaskQueue::ProcessCompletions() {
    if (!s_Data) { return; }

    std::queue<std::shared_ptr<TaskEntry>> done;
    {
        std::scoped_lock const lock(s_Data->completionMutex);
        std::swap(done, s_Data->completionQueue);
    }

    while (!done.empty()) {
        auto& entry = done.front();
        if (entry->status.load(std::memory_order_acquire) == TaskStatus::Completed) {
            entry->task->Finalize();
        }
        s_Data->registry.erase(entry->handle);
        done.pop();
    }
}

TaskStatus TaskQueue::GetStatus(TaskHandle handle) {
    if (!s_Data || handle == k_InvalidTaskHandle) { return TaskStatus::Failed; }
    auto it = s_Data->registry.find(handle);
    if (it == s_Data->registry.end()) {
        return TaskStatus::Completed;  // already finalized and removed from registry
    }
    return it->second->status.load(std::memory_order_acquire);
}

bool TaskQueue::IsComplete(TaskHandle handle) {
    TaskStatus const s = GetStatus(handle);
    return s == TaskStatus::Completed || s == TaskStatus::Failed;
}
