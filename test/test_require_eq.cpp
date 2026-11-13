#include "moko3/macros.hpp"

#include <limits>

namespace ns {

struct printable {
  int i = 0;

  bool operator==(const printable&) const = default;
};

inline std::string moko3_to_string(const printable& p) {
  return std::format("printable{}", p.i);
}

struct unprintable {};

}  // namespace ns

inline std::vector<std::string> failed;

struct tlistener : moko3::test_listener_i {
  void on_test_case_end(const moko3::test_run_info& i) noexcept {
    if (i.failed)
      failed.push_back(i.casename);
  }
};

REGISTER_TEST_LISTENER(tlistener);

TEST("to_string") {
  using moko3::moko3_to_string;
  // найдена через ADL
  REQUIRE(moko3_to_string(ns::printable{5}) == "printable5");
  REQUIRE(moko3_to_string(ns::unprintable{}) == "{?}");
#if __cpp_lib_format_ranges > 202002L
  REQUIRE(moko3_to_string(42) == "42");
#else
  REQUIRE(moko3_to_string(42) == "{?}");
#endif
}

TEST("compare") {
  REQUIRE_EQ(ns::printable{1}, ns::printable{1});
  REQUIRE_NOT_EQ(ns::printable{1}, ns::printable{2});
  REQUIRE_EQ(std::string("abc"), "abc");
  // std::cmp_equal
  REQUIRE_EQ(std::vector<int>(3).size(), 3);
  REQUIRE_NOT_EQ(-1, 4294967295u);
  // std::strong_order
  REQUIRE_EQ(1.5f, 1.5);
  REQUIRE_NOT_EQ(0.0, -0.0);
  double nan = std::numeric_limits<double>::quiet_NaN();
  REQUIRE_EQ(nan, nan);
}

TEST("failed EQ") {
  REQUIRE_EQ(ns::printable{1}, ns::printable{2});
}

TEST("failed NOT_EQ") {
  REQUIRE_NOT_EQ(1, 1);
}

int main() {
  int i = moko3::get_testbox().run_tests();
  REQUIRE(i == 2);
  REQUIRE(failed == std::vector<std::string>{"failed EQ", "failed NOT_EQ"});
}
