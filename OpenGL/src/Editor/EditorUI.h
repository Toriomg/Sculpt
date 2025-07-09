// src/Editor/EditorUI.h
#pragma once

#include <memory>

// Forward-declare GLFWwindow to keep the header clean
struct GLFWwindow;

class EditorUI {
public:
    // Takes the window context it needs to attach to
    EditorUI(GLFWwindow* window);
    ~EditorUI();

    // Call this at the beginning of the main loop
    void BeginFrame();

    // Call this at the end of the main loop
    void EndFrame();

private:
    // Making these non-copyable is good practice for manager classes
    EditorUI(const EditorUI&) = delete;
    EditorUI& operator=(const EditorUI&) = delete;
};