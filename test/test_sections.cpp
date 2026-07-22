#include "moko3/macros.hpp"

inline std::vector<std::string> metatest_result;

inline std::vector<int> metatest1_result;

inline int T1_run_count = 0;

TEST("T1") {
  ++T1_run_count;
}

inline int T2_run_count = 0;
inline int T2_sum = 0;

TEST("T2") {
  int i = GENERATE(1, 2, 3, 4, 5);
  ++T2_run_count;
  T2_sum += i;
}

TEST("METATEST1") {
  int i = GENERATE(1, 2);
  metatest1_result.push_back(i);
}

[[nodiscard]] bool metatest1_passed() {
  std::vector<int> expected{1, 2};
  return expected == metatest1_result;
}

inline size_t run_count = 0;
TEST("METATEST_SIMPLE_SECTION") {
  SECTION("A") {
    ++run_count;
  };
  ++run_count;
}

[[nodiscard]] bool metatest_simple_section_passed() {
  return run_count == 2;
}

TEST("METATEST") {
  int i = GENERATE(1, 2);
  metatest_result.push_back(std::format("I = {}", i));
  metatest_result.push_back("1");
  SECTION("S1") {
    metatest_result.push_back("2");
    for (int i = 0; i < 5; ++i) {
      SECTION("S1:1") {
        metatest_result.push_back("5");
      }

      SECTION("S1:2") {
        metatest_result.push_back("6");
      }
    }
  }
  SECTION("S2") {
    metatest_result.push_back("3");
    SECTION("S2:1") {
      metatest_result.push_back("7");
    }
  }
  metatest_result.push_back("4");
}

[[nodiscard]] bool metatest_passed() {
  // clang-format off
  std::vector<std::string> expected_metatest_result{
      // run 0
      "I = 1", "1", "2", "5", "5", "5", "5", "5", "4",
      // run 1
      "I = 1", "1", "2", "6", "6", "6", "6", "6", "4",
      // run 2
      "I = 1", "1", "3", "7", "4",
      // run 0 with new generation value
      "I = 2", "1", "2", "5", "5", "5", "5", "5", "4",
      // run 1 with new generationn value
      "I = 2", "1", "2", "6", "6", "6", "6", "6", "4",
      // run 2 with new generation value
      "I = 2", "1", "3", "7", "4",
  };
  // clang-format on
  return expected_metatest_result == metatest_result;
}

inline std::vector<std::string> metatest_generates_result;

#define POINT(FMT, ...) metatest_generates_result.push_back(std::format(FMT __VA_OPT__(, ) __VA_ARGS__))

TEST("METATEST GENERATES") {
  int i = GENERATE(1, 2);
  POINT("IN {}", i);
  SECTION("S1") {
    POINT("2");
    for (int i = 0; i < 5; ++i) {
      SECTION("->1") {
        int x = GENERATE(10, 11);
        POINT("P{}", x);
      }

      SECTION("->2") {
        POINT("6");
      }
    }
  }
  SECTION("S2") {
    int z = GENERATE(-1, -2, -3);
    POINT("P{}", z);
    SECTION("S2:1") {
      POINT("7");
    }
  }
  POINT("4");
}

bool metatest_generates_passed() {
  // clang-format off
  /*
  IN 1,2,51: 10,51: 10,51: 10,51: 10,51: 10,4,
  IN 1,2,6,6,6,6,6,4,
  IN 1,S2: -1,3,7,4,
  IN 1,S2: -2,3,7,4,
  IN 1,S2: -3,3,7,4,
  IN 2,2,51: 10,51: 10,51: 10,51: 10,51: 10,4,
  >>> вот тут не хватает S1: 11 прогона
  IN 2,2,6,6,6,6,6,4,
  IN 2,S2: -1,3,7,4,
  IN 2,S2: -2,3,7,4,
  IN 2,S2: -3,3,7,4,
  */
  std::vector<std::string> expected_metatest_result{
    // i = 1
    "IN 1", "2", "P10", "P10", "P10", "P10", "P10", "4",
    "IN 1", "2", "P11", "P11", "P11", "P11", "P11", "4",
    "IN 1", "2", "6", "6", "6", "6", "6", "4",
    "IN 1", "P-1", "7", "4",
    "IN 1", "P-2", "7", "4",
    "IN 1", "P-3", "7", "4",
    // i = 2
    "IN 2", "2", "P10", "P10", "P10", "P10", "P10", "4",
    "IN 2", "2", "P11", "P11", "P11", "P11", "P11", "4",
    "IN 2", "2", "6", "6", "6", "6", "6", "4",
    "IN 2", "P-1", "7", "4",
    "IN 2", "P-2", "7", "4",
    "IN 2", "P-3", "7", "4",
  };

  // clang-format on
  return metatest_generates_result == expected_metatest_result;
}

REGISTER_TEST_LISTENER(moko3::gtest_listener);

int main(int argc, char* argv[]) try {
  auto& box = moko3::get_testbox();
  box.parse_config(argc, argv);
  int i = box.run_tests();
  if (T1_run_count != 1)
    return -1;
  if (T2_run_count != 5 || T2_sum != (1 + 2 + 3 + 4 + 5))
    return -1;
  bool b = metatest_passed() && metatest1_passed() && metatest_simple_section_passed() &&
           metatest_generates_passed();
  if (!b)
    return -1;
  return i;
} catch (std::exception& e) {
  std::cerr << "running tests failed with err: " << e.what() << '\n';
  return -1;
}
