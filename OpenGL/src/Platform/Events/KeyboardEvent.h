#pragma once
#include "glhead.h"
#include "Event.h"

class KeyEvent : public Event {
public:
    int GetKeyCode() const { return m_KeyCode; }

protected:
    // Protected constructor so you can't create a generic KeyEvent
    KeyEvent(int keycode)
        : m_KeyCode(keycode) {
    }

    int m_KeyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(int keycode, bool isRepeat = false)
        : KeyEvent(keycode), m_IsRepeat(isRepeat) {
    }

    bool IsRepeat() const { return m_IsRepeat; }

    std::string GetKeyCode() {
        std::stringstream ss;
        ss << "KeyPressedEvent: " << m_KeyCode << " (repeat = " << m_IsRepeat << ")";
        return ss.str();
    }

    static EventType GetStaticType() { return EventType::KeyPressed; }
    virtual EventType GetEventType() const override { return GetStaticType(); }
    virtual const char* GetName() const override { return "KeyPressed"; }
private:
    bool m_IsRepeat;
};

class KeyReleasedEvent : public KeyEvent {
public:
    KeyReleasedEvent(int keycode)
        : KeyEvent(keycode) {
    }

    std::string GetKeyCode() {
        std::stringstream ss;
        ss << "KeyReleasedEvent: " << m_KeyCode;
        return ss.str();
    }

    static EventType GetStaticType() { return EventType::KeyReleased; }
    virtual EventType GetEventType() const override { return GetStaticType(); }
    virtual const char* GetName() const override { return "KeyReleased"; }
};

