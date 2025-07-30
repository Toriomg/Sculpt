#pragma once

#include "Event.h"

class MouseButtonPressedEvent : public Event {
public:
    MouseButtonPressedEvent(int button) : m_Button(button) {}

    int GetMouseButton() const { return m_Button; }

    // Static type for the dispatcher
    static EventType GetStaticType() { return EventType::MouseButtonPressed; }
    virtual EventType GetEventType() const override { return GetStaticType(); }
    virtual const char* GetName() const override { return "MouseButtonPressed"; }
private:
    int m_Button;
};

class MouseButtonReleasedEvent : public Event {
public:
    MouseButtonReleasedEvent(int button) : m_Button(button) {}

    int GetMouseButton() const { return m_Button; }

    // Static type for the dispatcher
    static EventType GetStaticType() { return EventType::MouseButtonReleased; }
    virtual EventType GetEventType() const override { return GetStaticType(); }
    virtual const char* GetName() const override { return "MouseButtonReleased"; }
private:
    int m_Button;
};