#pragma once

// on_scope_exit / on_scope_failure macros
#include "zal/zal.hpp"
#include "moko3/testbox.hpp"

/////////////////////////////////// UTILITY ////////////////////////////////

// depends only on __LINE__,
// its useful for big one-line macros with several usages of same name
#define MOKO3_UNIQUE_NAME LOGIC_GUARDS_CONCAT(moko3, __LINE__, __LINE__)
#define MOKO3_UNIQUE_NAME2 LOGIC_GUARDS_CONCAT(moko3_2, __LINE__, __LINE__)

// __VA_ARGS__ must be FN, which accept `top_level_section* _section` and return test_run_info
// name `_section` important for SECTION / GENERATE macros
#define MOKO3_REGISTER_TEST(NAME, ...) \
  static inline const ::moko3::test_registrator MOKO3_UNIQUE_NAME2(NAME, __VA_ARGS__)

// registers global listener for all tests
// NAME must be default constructible type, inherited from moko3::test_listener_i
// listener must be registered before test run
#define REGISTER_TEST_LISTENER(NAME) \
  ::moko3::test_listener_registrator MOKO3_UNIQUE_NAME(::std::make_unique<NAME>())

/////////////////////////////////// SECTIONS ////////////////////////////////

// "forks" test, each test or section runs once for each section in it,
// only one section is active on scope level
// May be used in tests, other sections.
// Note: works in loops correctly
#define SECTION(NAME)                                                                              \
  static ::moko3::section_info MOKO3_UNIQUE_NAME{.name = NAME};                                    \
  _section->register_section(&MOKO3_UNIQUE_NAME);                                                  \
  if (_section->enter_section(&MOKO3_UNIQUE_NAME))                                                 \
    if (::zal::scope_exit _exit_guard{[&] { _section->leave_section(&MOKO3_UNIQUE_NAME); }}; true) \
      if (auto* _section = &MOKO3_UNIQUE_NAME; true)

// `forks` test and runs all of its cases with each value from __VA_ARGS__
// may be used only on top-level in test and only once for each test
//
// example (runs "my test" 3 times):
//  TEST("my test") {
//    int i = GENERATE(1, 2, 3);
//  }
#define GENERATE(...) _section->next_generated_value(::std::to_array({__VA_ARGS__}))

/////////////////////////////////// ASSERTIONS ////////////////////////////////

#define REQUIRE(...)      \
  if (!bool(__VA_ARGS__)) \
  FAIL(#__VA_ARGS__)

#define FAIL(MSG) throw ::moko3::test_failed(::std::string(MSG))

#define REQUIRE_NOTHROW(...)                                                 \
  try {                                                                      \
    __VA_ARGS__;                                                             \
  } catch (::std::exception & e) {                                           \
    FAIL(::std::format("expected to be nothrow, but throwed {}", e.what())); \
  }

// creates and registers TEST. Code in test may use SECTION / GENERATE macros
#define TEST(NAME)                                            \
  void MOKO3_UNIQUE_NAME(::moko3::top_lvl_section* _section); \
  MOKO3_REGISTER_TEST(                                        \
      NAME, +[](::moko3::top_lvl_section* _section) {         \
        MOKO3_UNIQUE_NAME(_section);                          \
        return ::moko3::test_run_info{};                      \
      });                                                     \
  void MOKO3_UNIQUE_NAME(::moko3::top_lvl_section* _section)

// declares and implements `main` which accepts and parses cli arguments, then runs tests
#define MOKO3_MAIN                      \
  int main(int argc, char* argv[]) {    \
    auto& box = ::moko3::get_testbox(); \
    box.parse_config(argc, argv);       \
    return box.run_tests();             \
  }
