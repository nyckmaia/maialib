#pragma once

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _MSC_VER
#define OS_FUNCTION_SIGNATURE __FUNCSIG__
#else
#define OS_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG_DEBUG(msg)                                                                           \
    std::cout << "[DEBUG] " << __FILENAME__ << ":" << __LINE__ << " - " << OS_FUNCTION_SIGNATURE \
              << ": " << msg << std::endl
#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#define LOG_WARN(msg) std::cout << "[WARN] " << msg << std::endl
#define LOG_ERROR(msg)                                                                           \
    throw std::runtime_error(std::string("[maiacore] ") + msg + "\nSource File: " +              \
                             std::string(__FILENAME__) + " - Line " + std::to_string(__LINE__) + \
                             "\nFunction: " + std::string(OS_FUNCTION_SIGNATURE))
