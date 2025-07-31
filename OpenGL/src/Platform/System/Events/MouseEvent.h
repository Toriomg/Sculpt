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

class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y) : m_X(x), m_Y(y) {}
    float GetX() const { return m_X; }
    float GetY() const { return m_Y; }
    // Static type for the dispatcher
    static EventType GetStaticType() { return EventType::MouseMoved; }
    virtual EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "MouseMoved"; }
    private:
		float m_X, m_Y;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}
    float GetXOffset() const { return m_XOffset; }
    float GetYOffset() const { return m_YOffset; }
    // Static type for the dispatcher
    static EventType GetStaticType() { return EventType::MouseScrolled; }
    virtual EventType GetEventType() const override { return GetStaticType(); }
    virtual const char* GetName() const override { return "MouseScrolled"; }
private:
		float m_XOffset, m_YOffset;
};