#pragma once

#include <array>

template<std::size_t N>
constexpr std::array<char, N> constexpr_to_array(const char (&str)[N]);

template<std::size_t N>
constexpr auto constexpr_make_filter_pattern(const std::array<char, N> &str);

template <std::array ext>
struct Filter {
    std::array<char, ext.size()+2> pattern = constexpr_make_filter_pattern(ext);
};

template<Filter... filters>
consteval auto constexpr_get_filter_patterns();

template <Filter... filters>
constexpr std::size_t constexpr_get_filter_patterns_string_size() {
    return (filters.pattern.size() + ... + (sizeof...(filters) - 1));
}

template<Filter... filters>
constexpr auto constexpr_get_filter_patterns_string();

#include "Filters.tpp"
