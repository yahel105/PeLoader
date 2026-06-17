#include "Logger.h"
#include "Utility.h"


Logger& Logger::instance()
{
    static Logger s_instance;
    return s_instance;
};

void Logger::log(const LogLevel level, std::string_view msg)
{
    std::ostream& output{ m_logToFile ? m_file : std::cout };
    output << "[" << toString(level) << "] " << msg << "\n";
};

void Logger::setLogToFile(const std::filesystem::path path)
{
    m_logToFile = true;
    if (!m_file.is_open())
    {
        m_file.open(path, std::ios::app);
    }
}

const std::string Logger::toString(LogLevel level)
{
    switch (level) {
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warn:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    }
    return "?";
};

