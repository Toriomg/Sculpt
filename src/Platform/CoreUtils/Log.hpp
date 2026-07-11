// Logging facade over spdlog; exports CORE_LOG_* macros for engine use and LOG_* for client code.
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
    static void Separator() { 
        s_CoreLogger->sinks()[0]->log(spdlog::details::log_msg{ "", spdlog::level::info, "===================================================================\n" });
    }
private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;
};


// Core log macros
#define CORE_LOG_TRACE(...)    ::Log::GetCoreLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::trace, __VA_ARGS__)
#define CORE_LOG_INFO(...)     ::Log::GetCoreLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::info, __VA_ARGS__)
#define CORE_LOG_WARN(...)     ::Log::GetCoreLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::warn, __VA_ARGS__)
#define CORE_LOG_ERROR(...)    ::Log::GetCoreLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::err, __VA_ARGS__)
#define CORE_LOG_CRITICAL(...) ::Log::GetCoreLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::critical, __VA_ARGS__)

#define LOG_SEPARATOR() Log::Separator();

// Client log macros
#define LOG_TRACE(...)         ::Log::GetClientLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::trace, __VA_ARGS__)
#define LOG_INFO(...)          ::Log::GetClientLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...)          ::Log::GetClientLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...)         ::Log::GetClientLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(...)      ::Log::GetClientLogger()->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::critical, __VA_ARGS__)