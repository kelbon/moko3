#include "moko3/macros.hpp"

inline std::vector<std::string> metatest_result;

inline std::vector<int> metatest1_result;

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
  std::vector<std::string> expected_metatest_result{
      // run 0
      "I = 1",
      "1",
      "2",
      "5",
      "5",
      "5",
      "5",
      "5",
      "4",
      // run 1
      "I = 1",
      "1",
      "2",
      "6",
      "6",
      "6",
      "6",
      "6",
      "4",
      // run 2
      "I = 1",
      "1",
      "3",
      "7",
      "4",
      // run 0 with new generation value
      "I = 2",
      "1",
      "2",
      "5",
      "5",
      "5",
      "5",
      "5",
      "4",
      // run 1 with new generationn value
      "I = 2",
      "1",
      "2",
      "6",
      "6",
      "6",
      "6",
      "6",
      "4",
      // run 2 with new generation value
      "I = 2",
      "1",
      "3",
      "7",
      "4",
  };

  return expected_metatest_result == metatest_result;
}

REGISTER_TEST_LISTENER(moko3::gtest_listener);

int main(int argc, char* argv[]) try {
  auto& box = moko3::get_testbox();
  box.parse_config(argc, argv);
  int i = box.run_tests();
  bool b = metatest_passed() && metatest1_passed() && metatest_simple_section_passed();
  if (!b)
    return -1;
  return i;
} catch (std::exception& e) {
  std::cerr << "running tests failed with err: " << e.what() << '\n';
  return -1;
}
