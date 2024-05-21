#pragma once

#include "Filters.h"

template<std::size_t N>
constexpr std::array<char, N> constexpr_to_array(const char (&str)[N]) {
    std::array<char, N> result{};
    std::copy_n(str, N, result.begin());
    return result;
}

template<std::size_t N>
constexpr auto constexpr_make_filter_pattern(const std::array<char, N> &str) {
    std::array<char, N+2> result{'*','.'};
    for (std::size_t i = 0; i < N; ++i) {
        result[i+2] = str[i];
    }
    return result;
}

template<Filter... filters>
consteval auto constexpr_get_filter_patterns() {
    std::array<const char*, sizeof...(filters)> result{filters.pattern.data()...};
    return result;
}

template<Filter... filters>
constexpr auto constexpr_get_filter_patterns_string() {
    std::array<char, constexpr_get_filter_patterns_string_size<filters...>()> result{};
    std::size_t i = 0;

    auto process = [&i, &result](const char* ext) {
        if (i) { result[i++] = ','; result[i++] = ' '; }
        do { result[i++] = *ext++; } while (*ext);
    };

    (process(filters.pattern.data()), ...);
    result[i++] = '\0';

    return result;
}

static constexpr Filter<constexpr_to_array("pid")> pidFilter;
static constexpr Filter<constexpr_to_array("pal")> palFilter;
