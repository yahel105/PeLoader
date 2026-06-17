#pragma once
#include "Utility.h"


class PeLoaderError : public std::runtime_error {
public:
	explicit PeLoaderError(const std::string& msg, LogLevel logLevel = LogLevel::Error) :std::runtime_error(msg), m_logLevel(logLevel) {}
	const LogLevel m_logLevel;
};