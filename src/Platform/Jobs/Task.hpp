// Abstract unit of work for TaskQueue; Execute runs on a worker thread, Finalize on the main thread.
#pragma once
#include <cstdint>
#include <string_view>

using TaskHandle = uint64_t;
constexpr TaskHandle k_InvalidTaskHandle = 0;

enum class TaskStatus : uint8_t {
    Pending,
    Running,
    Completed,
    Failed
};

class ITask {
public:
    virtual ~ITask() = default;
    // Runs on a worker thread — no OpenGL calls allowed here.
    virtual void Execute() = 0;
    // Runs on the main thread after Execute completes — GL calls are safe here.
    virtual void Finalize() {}
    virtual std::string_view GetName() const { return "Task"; }
};
