#pragma once

#include <compare>
#include <concepts>
#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace moko3 {

// ADL customization point, uses std::format if possible since C++23
template <typename T>
std::string moko3_to_string(const T& v) {
  // __cpp_lib_format_ranges появилось тогда же когда и formattable
#if __cpp_lib_format_ranges >= 202207L
  if constexpr (std::formattable<T, char>)
    return std::format("{}", v);
#endif
  (void)v;
  return "{?}";
}

namespace noexport {

template <typename T>
concept cmp_integer =
    std::integral<T> && !std::same_as<T, bool> && !std::same_as<T, char> && !std::same_as<T, wchar_t> &&
    !std::same_as<T, char8_t> && !std::same_as<T, char16_t> && !std::same_as<T, char32_t>;

template <typename T>
concept c_string = std::same_as<std::decay_t<T>, const char*> || std::same_as<std::decay_t<T>, char*>;

template <typename A, typename B>
bool is_equal(const A& a, const B& b) {
  return bool(a == b);
}

template <cmp_integer A, cmp_integer B>
bool is_equal(const A& a, const B& b) {
  return std::cmp_equal(a, b);
}

// 0.0 != -0.0, NaN == NaN
template <std::floating_point A, std::floating_point B>
bool is_equal(const A& a, const B& b) {
  using C = std::common_type_t<A, B>;
  return std::is_eq(std::strong_order(C(a), C(b)));
}

template <std::floating_point A, std::integral B>
bool is_equal(const A&, const B&) = delete;

template <std::integral A, std::floating_point B>
bool is_equal(const A&, const B&) = delete;

// сравнение указателей, а не строк
template <c_string A, c_string B>
bool is_equal(const A&, const B&) = delete;

// throws test_failed
[[noreturn]] void require_eq_failed(std::string_view macro, std::string_view astr, std::string_view bstr,
                                    std::string a, std::string b, std::source_location);

template <typename A, typename B>
void require_eq(const A& a, const B& b, std::string_view astr, std::string_view bstr,
                std::source_location l = std::source_location::current()) {
  if (!is_equal(a, b))
    require_eq_failed("REQUIRE_EQ", astr, bstr, moko3_to_string(a), moko3_to_string(b), l);
}

template <typename A, typename B>
void require_not_eq(const A& a, const B& b, std::string_view astr, std::string_view bstr,
                    std::source_location l = std::source_location::current()) {
  if (is_equal(a, b))
    require_eq_failed("REQUIRE_NOT_EQ", astr, bstr, moko3_to_string(a), moko3_to_string(b), l);
}

}  // namespace noexport

}  // namespace moko3
