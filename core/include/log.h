#include <string>
#include <exception>

#define LOG_INFO(FormatLiteral, ...) fprintf (stderr, "[INFO] [%s:%d] " FormatLiteral "\n", __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_WARN(FormatLiteral, ...) fprintf (stderr, "[WARN] [%s:%d] " FormatLiteral "\n", __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(FormatLiteral, ...) fprintf (stderr, "[ERROR] [%s:%d] " FormatLiteral "\n", __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__)

/*
// #define TRY(f) \
//   std::string msg = "[maiacore]"; \
//   msg.append(__LINE__); \
//   msg.append(f); \
//   throw std::runtime_error(msg);

*/
