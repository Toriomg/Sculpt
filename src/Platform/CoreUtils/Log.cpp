#include "Log.hpp"

// Define the static member variables
std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

void Log::Init() {
    // --- Create the Sinks (output destinations) ---
    // A console sink that prints with color, and a file sink
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.emplace_back(
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("3DModeler.log", true));

    // --- Configure the Sinks' Pattern ---
    // [Timestamp] [Logger Name] [Log Level]: Message
    // Example: [2023-10-27 15:30:00.123] [APP] [info]: Hello World!
    logSinks[0]->set_pattern("%^[%n] [%s:%#]: %v%$");
    logSinks[1]->set_pattern("[%l] [%n] [%s:%#]: %v");

    // --- Create the Loggers ---
    // Core logger for the engine itself
    s_CoreLogger = std::make_shared<spdlog::logger>("ENGINE", begin(logSinks), end(logSinks));
    spdlog::register_logger(s_CoreLogger);
    s_CoreLogger->set_level(spdlog::level::trace);  // Set minimum level to log
    s_CoreLogger->flush_on(spdlog::level::trace);

    // Client logger for the final application (your modeler)
    s_ClientLogger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
    spdlog::register_logger(s_ClientLogger);
    s_ClientLogger->set_level(spdlog::level::trace);
    s_ClientLogger->flush_on(spdlog::level::trace);
}
