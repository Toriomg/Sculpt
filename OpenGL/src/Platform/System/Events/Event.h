#pragma once

enum class EventType {
    None = 0,
    WindowClose, WindowResize,
	KeyPressed, KeyReleased, KeyRepeated,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

// Base Event class
class Event {
public:
    virtual ~Event() = default;
    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    bool Handled = false; // Flag to see if an event has been consumed
};

// A helper to simplify event dispatching
class EventDispatcher {
public:
    EventDispatcher(Event& event) : m_Event(event) {}

    template<typename T, typename F>
    bool Dispatch(const F& func) {
        if (m_Event.GetEventType() == T::GetStaticType()) {
            m_Event.Handled = func(static_cast<T&>(m_Event));
            return true;
        }
        return false;
    }
private:
    Event& m_Event;
};