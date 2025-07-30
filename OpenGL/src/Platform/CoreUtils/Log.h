#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/fmt/ostr.h"


class Log {
public:
    static void Init();

    // Static accessors to the loggers
    static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
    static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;
};

// Core log macros
#define CORE_LOG_TRACE(...)    ::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define CORE_LOG_INFO(...)     ::Log::GetCoreLogger()->info(__VA_ARGS__)
#define CORE_LOG_WARN(...)     ::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define CORE_LOG_ERROR(...)    ::Log::GetCoreLogger()->error(__VA_ARGS__)
#define CORE_LOG_CRITICAL(...) ::Log::GetCoreLogger()->critical(__VA_ARGS__)


// Client log macros
#define LOG_TRACE(...)         ::Log::GetClientLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)          ::Log::GetClientLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)          ::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)         ::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)      ::Log::GetClientLogger()->critical(__VA_ARGS__)