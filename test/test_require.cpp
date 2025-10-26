#include "moko3/moko3.hpp"

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

int main() {
  int i = moko3::get_testbox().run_tests();
  REQUIRE(i == 1);
  std::vector<std::string> expected{
      "started A", "case started A", "case failed A::B", "ended A",  // must stop after first failure in test
      "started B", "case started B", "case ended B::G0", "ended B",
  };
  REQUIRE(expected == output);
}
