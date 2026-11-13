#pragma once

// on_scope_exit / on_scope_failure macros
#include "zal/zal.hpp"
#include "moko3/testbox.hpp"
#include "moko3/require_eq.hpp"

/////////////////////////////////// UTILITY ////////////////////////////////

// depends only on __LINE__,
// its useful for big one-line macros with several usages of same name
#define MOKO3_UNIQUE_NAME LOGIC_GUARDS_CONCAT(moko3, __LINE__, __LINE__)
#define MOKO3_UNIQUE_NAME2 LOGIC_GUARDS_CONCAT(moko3_2, __LINE__, __LINE__)
// для SECTION, не должно совпадать с MOKO3_UNIQUE_NAME##suffix
#define MOKO3_SECTION_NAME LOGIC_GUARDS_CONCAT(moko3_section, __LINE__, __LINE__)

// __VA_ARGS__ must be FN, which accept `top_level_section* _section` and return test_run_info
// name `_section` important for SECTION / GENERATE macros
#define MOKO3_REGISTER_TEST(NAME, ...) \
  static inline const ::moko3::test_registrator MOKO3_UNIQUE_NAME2(NAME, __VA_ARGS__)

// registers global listener for all tests
// NAME must be default constructible type, inherited from moko3::test_listener_i
// listener must be registered before test run
// '...' accepts arguments for listener constructor
#define REGISTER_TEST_LISTENER(NAME, ...) \
  ::moko3::test_listener_registrator MOKO3_UNIQUE_NAME(::std::make_unique<NAME>(__VA_ARGS__))

/////////////////////////////////// SECTIONS ////////////////////////////////

// "forks" test, each test or section runs once for each section in it,
// only one section is active on scope level
// May be used in tests, other sections.
// Note: works in loops correctly
// `...` accepts optional additional part of name for declaring sevral SECTION on same line (e.g. in macro)
#define SECTION(NAME, ...)                                                                          \
  static const ::moko3::section_key MOKO3_UNIQUE_NAME##__VA_ARGS__{.name = NAME};                   \
  if (::moko3::section_info* MOKO3_SECTION_NAME = _section->child(&MOKO3_UNIQUE_NAME##__VA_ARGS__); \
      _section->enter_section(MOKO3_SECTION_NAME))                                                  \
    if (::zal::scope_exit _exit_guard{[&] { _section->leave_section(MOKO3_SECTION_NAME); }}; true)  \
      if (auto* _section = MOKO3_SECTION_NAME; true)

// `forks` test and runs all of its cases with each value from __VA_ARGS__
// may be used in SECTION too, invokes test with this section for each value
//
// example (runs "my test" 3 times):
//  TEST("my test") {
//    int i = GENERATE(1, 2, 3);
//  }
// Must be called before any other SECTION/GENERATE in its section
#define GENERATE(...) _section->next_generated_value(::std::to_array({__VA_ARGS__}), __LINE__)

// same as GENERATE, but accepts range of values (random access and sized, e.g. std::vector).
// Range may depend on GENERATE of parent section
//
// example (runs "my test" for each pair):
//  TEST("my test") {
//    auto [i, s] = GENERATE_RANGE(std::views::cartesian_product(ints, strings));
//  }
#define GENERATE_RANGE(...) _section->next_generated_value(__VA_ARGS__, __LINE__)

// produces COUNT of random size_t values. Same values for same --seed & test
// similar to GENERATE(rand1, rand2, ... COUNT times)
#define GENERATE_RANDOM(COUNT) \
  ::moko3::get_testbox().generate_random(*_section, _section->next_generated_index(COUNT, __LINE__))

/////////////////////////////////// ASSERTIONS ////////////////////////////////

#define REQUIRE(...) (bool(__VA_ARGS__) ? (void)0 : FAIL(#__VA_ARGS__))

// as REQUIRE(A == B), but prints values of A and B on failure (uses std::format / moko3_to_string)
#define REQUIRE_EQ(A, B) ::moko3::noexport::require_eq(A, B, #A, #B)
#define REQUIRE_NOT_EQ(A, B) ::moko3::noexport::require_not_eq(A, B, #A, #B)

#define FAIL(MSG) throw ::moko3::test_failed(::std::string(MSG))

#define REQUIRE_NOTHROW(...)                                                   \
  do {                                                                         \
    try {                                                                      \
      __VA_ARGS__;                                                             \
    } catch (::moko3::test_exception&) {                                       \
      throw;                                                                   \
    } catch (::std::exception & e) {                                           \
      FAIL(::std::format("expected to be nothrow, but throwed {}", e.what())); \
    } catch (...) {                                                            \
      FAIL("expected to be nothrow, but throwed unknown exception");           \
    }                                                                          \
  } while (false)

// any exception accepted
#define REQUIRE_THROWS(...)                              \
  do {                                                   \
    try {                                                \
      __VA_ARGS__;                                       \
      FAIL("expected to throw, but nothing was thrown"); \
    } catch (::moko3::test_exception&) {                 \
      throw;                                             \
    } catch (...) {                                      \
    }                                                    \
  } while (false)

// creates and registers TEST. Code in test may use SECTION / GENERATE macros
#define TEST(NAME)                                         \
  void MOKO3_UNIQUE_NAME(::moko3::section_info* _section); \
  MOKO3_REGISTER_TEST(                                     \
      NAME, +[](::moko3::section_info* _section) {         \
        MOKO3_UNIQUE_NAME(_section);                       \
        return ::moko3::test_run_info{};                   \
      });                                                  \
  void MOKO3_UNIQUE_NAME(::moko3::section_info* _section)

// declares and implements `main` which accepts and parses cli arguments, then runs tests
#define MOKO3_MAIN                       \
  int main(int argc, char* argv[]) {     \
    auto& box = ::moko3::get_testbox();  \
    box.parse_config(argc, argv);        \
    return box.run_tests() == 0 ? 0 : 1; \
  }
