#pragma once
#include <glhead.h>
#include "glhead.h"
#include <memory>
#include <string_view>

#include "Platform/System/Events/MouseEvent.h"
#include "Platform/System/Events/WindowEvent.h"
#include "Platform/System/Events/KeyboardEvent.h"

class Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;

    virtual ~Window() = default;

    virtual void OnUpdate() = 0;

    virtual uint32_t GetWidth()  const = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsVSync() const = 0;

    virtual void* GetNativeWindow() const = 0;

    static std::unique_ptr<Window> Create(
        std::string_view title = "3D Modeler",
        uint32_t width  = 1600,
        uint32_t height = 900
    );
};
