#pragma once

#include <clocale>
#include <locale>
#include <stdexcept>
#include <string>

// The first comma-decimal locale this platform has installed, or an empty string when it has none
// of them. The names cover Windows ("Portuguese_Brazil.1252") and glibc or macOS ("pt_BR.UTF-8").
inline std::string installedCommaDecimalLocale() {
    for (const char* name : {"Portuguese_Brazil.1252", "pt_BR.UTF-8", "pt_BR.utf8", "de_DE.UTF-8",
                             "de_DE.utf8", "fr_FR.UTF-8", "German_Germany.1252"}) {
        try {
            const std::locale probe(name);
            return name;
        } catch (const std::runtime_error&) {
            continue;
        }
    }
    return {};
}

// Sets the C library's LC_NUMERIC category, the one atof() and printf() follow, for its lifetime.
// The destructor restores the previous setting, also when an assertion or an exception leaves the
// scope early, so no later test runs under the changed locale.
class ScopedCNumericLocale {
   public:
    explicit ScopedCNumericLocale(const std::string& name)
        : _previous(std::setlocale(LC_NUMERIC, nullptr)) {
        _active = std::setlocale(LC_NUMERIC, name.c_str()) != nullptr;
    }
    ~ScopedCNumericLocale() { std::setlocale(LC_NUMERIC, _previous.c_str()); }
    ScopedCNumericLocale(const ScopedCNumericLocale&) = delete;
    ScopedCNumericLocale& operator=(const ScopedCNumericLocale&) = delete;

    bool active() const { return _active; }

   private:
    std::string _previous;
    bool _active = false;
};

// Sets the global C++ locale for its lifetime; for a named locale that also sets the whole C
// locale. The destructor restores both.
class ScopedGlobalLocale {
   public:
    explicit ScopedGlobalLocale(const std::string& name)
        : _previousC(std::setlocale(LC_ALL, nullptr)),
          _previousCpp(std::locale::global(std::locale(name))) {}
    ~ScopedGlobalLocale() {
        std::locale::global(_previousCpp);
        std::setlocale(LC_ALL, _previousC.c_str());
    }
    ScopedGlobalLocale(const ScopedGlobalLocale&) = delete;
    ScopedGlobalLocale& operator=(const ScopedGlobalLocale&) = delete;

   private:
    std::string _previousC;
    std::locale _previousCpp;
};
