#pragma once

#include <string_view>

bool charEqualsCaseSensitive(char a, char b);
bool charEqualsCaseInsensitive(char a, char b);
char charToLower(char a);
bool stringEquals(std::string_view lhs, std::string_view rhs, bool caseSensitive = true);
