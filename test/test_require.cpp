#include "moko3/moko3.hpp"

#include <sstream>

inline std::vector<std::string> output;

struct tlistener : moko3::test_listener_i {
  void on_test_start(moko3::testinfo const& i) noexcept {
    output.push_back(std::format("started {}", i.name));
  }
  void on_test_end(moko3::testinfo const& i) noexcept {
    output.push_back(std::format("ended {}", i.name));
  }

  void on_test_case_start(moko3::testinfo const& i) noexcept {
    output.push_back(std::format("case started {}", i.name));
  }
  void on_test_case_end(const moko3::test_run_info& i) noexcept {
    if (i.failed) {
      output.push_back(std::format("case failed {}", i.casename));
    } else {
      output.push_back(std::format("case ended {}", i.casename));
    }
  }
};

REGISTER_TEST_LISTENER(tlistener);

TEST("A") {
  SECTION("B") {
    REQUIRE(false);
  }
  SECTION("C") {
  }
}

TEST("B") {
  (void)GENERATE(1);
}

TEST("C") {
  // REQUIRE - выражение, else относится к внешнему if
  if (true)
    REQUIRE(true);
  else
    FAIL("unreachable");
  REQUIRE_NOTHROW((void)0);
  REQUIRE_THROWS(throw 1);
}

TEST("D") {
  REQUIRE_NOTHROW(throw 1);
}

// падение теста внутри не перехватывается
TEST("E") {
  REQUIRE_NOTHROW(REQUIRE(false));
}

TEST("F") {
  REQUIRE_THROWS((void)0);
}

TEST("G") {
  REQUIRE_THROWS(REQUIRE(false));
}

int main() {
  auto& box = moko3::get_testbox();
  int i = box.run_tests();
  REQUIRE(i == 5);
  // clang-format off
  std::vector<std::string> expected{
      "started A", "case started A", "case failed A::B", "ended A",  // must stop after first failure in test
      "started B",  "case started B", "case ended B::G0", "ended B",
      "started C", "case started C", "case ended C",   "ended C",
      "started D", "case started D", "case failed D",  "ended D",
      "started E", "case started E", "case failed E", "ended E",
      "started F", "case started F", "case failed F",  "ended F",
      "started G", "case started G", "case failed G",    "ended G",
  };
  // clang-format on
  REQUIRE(expected == output);
}
