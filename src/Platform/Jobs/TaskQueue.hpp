// Thread pool that executes ITask work on background threads and finalizes results on the main
// thread each frame.
#pragma once
#include "Platform/Jobs/Task.hpp"
#include <concepts>
#include <expected>
#include <memory>
#include <string>

class TaskQueue {
public:
    // threadCount = 0 defaults to hardware_concurrency().
    static void Init(unsigned int threadCount = 0);
    static void Shutdown();

    // Construct a T(args...) and submit it for async execution.
    // Returns the task handle, or an error string if the queue is unavailable.
    template <std::derived_from<ITask> T, typename... Args>
    static std::expected<TaskHandle, std::string> Submit(Args&&... args) {
        return SubmitTask(std::make_unique<T>(std::forward<Args>(args)...));
    }

    // Drain the completion queue and call Finalize() on each task. Must be called once per frame on
    // the main thread.
    static void ProcessCompletions();

    static TaskStatus GetStatus(TaskHandle handle);
    static bool IsComplete(TaskHandle handle);

private:
    static std::expected<TaskHandle, std::string> SubmitTask(std::unique_ptr<ITask> task);
};
