#include <numeric>
#include <iostream>

#define OUT(...) std::cout << #__VA_ARGS__ << " = " << __VA_ARGS__ << '\n'

template <typename T>
void ignore(T &&)
{ }

constexpr unsigned int hash(const char *s, int off = 0) 
{
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];
}

