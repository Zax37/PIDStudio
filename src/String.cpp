#include "String.h"

#include <algorithm>
#include <cctype>

bool charEqualsCaseSensitive(char a, char b)
{
    return a == b;
}

bool charEqualsCaseInsensitive(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
        std::tolower(static_cast<unsigned char>(b));
}

char charToLower(char a) {
    return std::tolower(a);
}

bool stringEquals(std::string_view lhs, std::string_view rhs, bool caseSensitive)
{
    return std::ranges::equal(lhs, rhs, caseSensitive ? charEqualsCaseSensitive : charEqualsCaseInsensitive);
}
