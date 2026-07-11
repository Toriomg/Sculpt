#pragma once

class Time
{
public:
    // Call this once at the beginning of the application
    static void Init();

    // Call this once per frame, at the beginning of your main loop
    static void Update();

    // Returns the total time elapsed since the application started (in seconds)
    inline static float GetTime() { return s_Time; }

    // Returns the time it took to complete the last frame (in seconds)
    // Use this for all frame-rate independent calculations (movement, physics, etc.)
    inline static float GetDeltaTime() { return s_DeltaTime; }

    // Returns the current frames per second (FPS)
    // Useful for displaying performance metrics, not for game logic
    inline static float GetFPS() { return (s_DeltaTime > 0.0f) ? (1.0f / s_DeltaTime) : 0.0f; }

private:
    // Making the constructor private enforces that this is a static-only class
    Time() = default;

    static float s_LastFrameTime;
    static float s_Time;
    static float s_DeltaTime;
};