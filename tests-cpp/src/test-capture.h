#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// The first line of the std::runtime_error message that action() throws, or an empty string when
// action() throws nothing. The first line is the message proper: LOG_ERROR appends the source
// location, the function signature and a stack trace, which can contain the very words a test
// looks for. Asserting on the message, rather than with a bare EXPECT_THROW, tells a guard's own
// rejection apart from any other std::runtime_error the same call could raise.
template <typename Action>
std::string thrownFirstLine(Action&& action) {
    try {
        action();
    } catch (const std::runtime_error& error) {
        const std::string what = error.what();
        return what.substr(0, what.find('\n'));
    }
    return {};
}

// Redirects std::cout, where LOG_WARN writes, into a buffer for its lifetime. The destructor
// restores the original buffer, also when an exception escapes the captured call.
class StdoutCapture {
   public:
    StdoutCapture() : _previous(std::cout.rdbuf(_buffer.rdbuf())) {}
    ~StdoutCapture() { std::cout.rdbuf(_previous); }
    StdoutCapture(const StdoutCapture&) = delete;
    StdoutCapture& operator=(const StdoutCapture&) = delete;

    std::string str() const { return _buffer.str(); }

   private:
    std::stringstream _buffer;
    std::streambuf* _previous;
};
