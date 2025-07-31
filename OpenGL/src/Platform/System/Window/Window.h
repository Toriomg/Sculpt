#pragma once
#include <glhead.h>
#include "glhead.h"

// Events

#include "Platform/System/Events/MouseEvent.h"
#include "Platform/System/Events/WindowEvent.h"
#include "Platform/Events/KeyboardEvent.h"

class Window {
public:
    // Using a std::function to create an event callback system.
    // The Application will set this callback to receive events from the window.
    using EventCallbackFn = std::function<void(Event&)>; // Placeholder for your Event class

    virtual ~Window() = default;

    virtual void OnUpdate() = 0;

    virtual unsigned int GetWidth() const = 0;
    virtual unsigned int GetHeight() const = 0;

    // Window attributes
    virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsVSync() const = 0;

    // Returns the native, underlying window pointer (e.g., GLFWwindow*).
    // This is needed for libraries like ImGui.
    virtual void* GetNativeWindow() const = 0;

    virtual std::string GetKeyName(int keycode) const = 0;

    // This is a "factory method". It's a static function that creates
    // the appropriate platform-specific window instance.
    static Window* Create(const std::string& title = "3D Modeler", unsigned int width = 1600, unsigned int height = 900);
};
