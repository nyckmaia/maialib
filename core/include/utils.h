#include <iostream>
#include <numeric>

template <typename T>
void ignore(T &&) {}

constexpr unsigned int hash(const char *s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off];
}

constexpr int factorial(const int n) { return (n == 0) || (n == 1) ? 1 : n * factorial(n - 1); }

constexpr bool isFloatEqual(float A, float B, float epsilon = 0.005f) {
    return (std::fabs(A - B) < epsilon);
}
