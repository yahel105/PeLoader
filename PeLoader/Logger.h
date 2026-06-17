#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include "Utility.h"


class Logger
{
public:
	static Logger& instance();
	void log(LogLevel level, std::string_view msg);
	void setLogToFile(const std::filesystem::path path = "log.txt");

	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
private:
	Logger() = default;
	bool m_logToFile{ false };
	static const std::string toString(LogLevel level);
	std::ofstream m_file;
};

