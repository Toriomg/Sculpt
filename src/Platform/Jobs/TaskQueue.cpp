#include "Platform/Jobs/TaskQueue.hpp"
#include "Platform/CoreUtils/Log.hpp"
#include <optional>
#include <vector>

namespace {

    std::optional<std::vector<std::function<void()>>>
        s_Pending;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace

void TaskQueue::Init() {
    s_Pending.emplace();
    CORE_LOG_INFO("TaskQueue: initialized");
}

void TaskQueue::Shutdown() {
    ProcessCompletions();
    s_Pending.reset();
    CORE_LOG_INFO("TaskQueue: shutdown");
}

void TaskQueue::Submit(std::function<void()> work) {
    if (s_Pending) { s_Pending->push_back(std::move(work)); }
}

void TaskQueue::ProcessCompletions() {
    if (!s_Pending || s_Pending->empty()) { return; }
    std::vector<std::function<void()>> batch;
    std::swap(batch, *s_Pending);
    for (auto const& fn : batch) { fn(); }
}
