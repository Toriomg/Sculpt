// Static input polling API with a swappable platform-specific InputImpl; initialized by Application
// with a GlfwInput.
#pragma once
#include "KeyCodes.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include "Platform/System/Window/Window.hpp"
#include <memory>

class Window;  // Forward-declare

// The public-facing static class
class Input {
public:
    static void Init(Window* window);
    static void Shutdown();

    // To be called once per frame by the Application
    static void OnUpdate();

    // Keyboard polling
    static bool IsKeyPressed(KeyCode key);
    // Mouse polling
    static bool IsMouseButtonPressed(MouseCode button);
    static Vec2 GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();
    static std::string GetKeyName(int keycode);
    // This inner class is the key to platform abstraction
    class InputImpl {
    public:
        virtual ~InputImpl()                                    = default;
        virtual void OnUpdateImpl()                             = 0;
        virtual bool IsKeyPressedImpl(KeyCode key)              = 0;
        virtual bool IsMouseButtonPressedImpl(MouseCode button) = 0;
        virtual Vec2 GetMousePositionImpl()                     = 0;
        virtual std::string GetKeyNameImpl(int keycode) const   = 0;
    };

private:
    // The static instance that holds the platform-specific implementation
    static std::unique_ptr<InputImpl> s_Instance;
};
