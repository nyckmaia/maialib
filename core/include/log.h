#pragma once

#include <stdexcept>
#include <iosfwd>
#include <cstring>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG_DEBUG(msg) std::cout << "[DEBUG] " << __FILENAME__ << ":" << __LINE__ << " - " << __PRETTY_FUNCTION__ << ": " << msg << std::endl
#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#define LOG_WARN(msg) std::cout << "[WARN] " << msg << std::endl
#define LOG_ERROR(msg) throw std::runtime_error(std::string("[maiacore] ") + msg + "\nSource File: " + std::string(__FILENAME__) + " - Line " + std::to_string(__LINE__) + "\nFunction: " + std::string(__PRETTY_FUNCTION__))
