#include "moko3/macros.hpp"

#include <map>

using values_t = std::vector<std::minstd_rand::result_type>;

inline std::map<std::string, values_t> results;

TEST("A") {
  results["A"].push_back(GENERATE_RANDOM(10));
  // это тоже GENERATE
  REQUIRE(!_section->can_generate);
}

TEST("B") {
  SECTION("S1") {
    results["B::S1"].push_back(GENERATE_RANDOM(10));
  }
  SECTION("S2") {
    results["B::S2"].push_back(GENERATE_RANDOM(10));
  }
}

static std::map<std::string, values_t> run(size_t seed) {
  results.clear();
  moko3::get_testbox().set_random(seed);
  REQUIRE(moko3::get_testbox().run_tests() == 0);
  return results;
}

int main() {
  moko3::get_testbox().register_listener(nullptr);
  auto r = run(42);
  REQUIRE(r["A"].size() == 10 && r["B::S1"].size() == 10 && r["B::S2"].size() == 10);
  // разные тесты и секции - разные значения
  REQUIRE(r["A"] != r["B::S1"] && r["B::S1"] != r["B::S2"]);
  // тот же сид - те же значения
  REQUIRE(run(42) == r);
  REQUIRE(run(43) != r);

  char arg0[] = "test", arg1[] = "--seed", arg2[] = "50";
  char* args[] = {arg0, arg1, arg2, nullptr};
  moko3::get_testbox().parse_config(3, args);
  REQUIRE(moko3::get_testbox().random_seed() == 50);
}
