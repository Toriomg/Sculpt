#pragma once

#include "Event.h"

class KeyPressedEvent : public Event {
public:
    KeyPressedEvent(int button) : m_Button(button) {}

    int GetMouseButton() const { return m_Button; }


    static EventType GetStaticType() { return EventType::KeyPressed; }
    virtual EventType GetEventType() const override { return GetStaticType(); }
    virtual const char* GetName() const override { return "KeyPressed"; }
private:
    int m_Button;
};

class KeyReleasedEvent : public Event {
public:
    KeyReleasedEvent(int button) : m_Button(button) {}

    int GetMouseButton() const { return m_Button; }

    static EventType GetStaticType() { return EventType::KeyReleased; }
    virtual EventType GetEventType() const override { return GetStaticType(); }
    virtual const char* GetName() const override { return "KeyReleased"; }
private:
    int m_Button;
};

class KeyRepeatedEvent : public Event {
public:
    KeyRepeatedEvent(int button) : m_Button(button) {}

    int GetMouseButton() const { return m_Button; }

    static EventType GetStaticType() { return EventType::KeyRepeated; }
    virtual EventType GetEventType() const override { return GetStaticType(); }
    virtual const char* GetName() const override { return "KeyRepeated"; }
private:
    int m_Button;
};
