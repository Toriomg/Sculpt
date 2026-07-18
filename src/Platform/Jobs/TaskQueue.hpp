// Deferred-work queue: Submit() enqueues a callback, ProcessCompletions() runs them on the main
// thread each frame.
#pragma once
#include <functional>
#include <string>

class TaskQueue {
public:
    static void Init();
    static void Shutdown();

    // Enqueue a callback to be called on the main thread during ProcessCompletions().
    static void Submit(std::function<void()> work);

    // Call all pending callbacks. Must be called once per frame on the main thread.
    static void ProcessCompletions();
};
