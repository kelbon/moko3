#include "moko3/macros.hpp"

struct tlistener : moko3::test_listener_i {
  int started = 0;
  int ended = 0;
  std::vector<std::string> tests;
  std::vector<std::string> tests_ended;
  std::vector<std::string> cases;
  std::vector<std::string> cases_ended;

  void on_start() noexcept {
    ++started;
  }

  void on_end() noexcept {
    ++ended;
  }

  void on_test_start(moko3::testinfo const& i) noexcept {
    tests.push_back(i.name);
  }
  void on_test_end(moko3::testinfo const& i) noexcept {
    tests_ended.push_back(i.name);
  }

  void on_test_case_start(moko3::testinfo const& i) noexcept {
    cases.push_back(i.name);
  }
  void on_test_case_end(const moko3::test_run_info& i) noexcept {
    cases_ended.push_back(i.testname);
  }
};

TEST("A") {
  SECTION("C") {
  }
}

TEST("B") {
  (void)GENERATE(1, 2);
}

REGISTER_TEST_LISTENER(tlistener);

int main() {
  int i = moko3::get_testbox().run_tests();
  REQUIRE(i == 0);
  tlistener& l = dynamic_cast<tlistener&>(moko3::get_testbox().get_listener());

  REQUIRE(l.started == 1);
  REQUIRE(l.ended == 1);
  std::vector<std::string> expected_tests{
      "A",
      "B",
  };
  REQUIRE(l.tests == expected_tests);
  REQUIRE(l.tests_ended == expected_tests);
  std::vector<std::string> expected_cases{
      "A",
      "B",
      "B",
  };
  REQUIRE(l.cases == expected_cases);
  std::vector<std::string> expected_cases_ended{
      "A::C",
      "B::G0",
      "B::G1",
  };
  REQUIRE(l.cases_ended == expected_cases_ended);
}
