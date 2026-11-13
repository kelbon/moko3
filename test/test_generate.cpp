#include "moko3/macros.hpp"

#include <array>
#include <map>
#ifdef __cpp_lib_ranges_cartesian_product
  #include <ranges>
#endif

// GENERATE: только один на тест / секцию и только до вложенных секций.
// Это обеспечивается флагом `can_generate`: если он false, GENERATE вызывает abort

inline std::map<std::string, std::vector<int>> results;

#define RUN(NAME, ...) results[NAME].push_back(__VA_ARGS__)

TEST("CAN GENERATE") {
  REQUIRE(_section->can_generate);
  (void)GENERATE(1, 2);
  // второй GENERATE запрещён
  REQUIRE(!_section->can_generate);
  SECTION("A") {
    // у вложенной секции свой GENERATE
    REQUIRE(_section->can_generate);
    (void)GENERATE(1, 2);
    REQUIRE(!_section->can_generate);
  }
  REQUIRE(!_section->can_generate);
}

TEST("CANNOT GENERATE AFTER SECTION") {
  REQUIRE(_section->can_generate);
  SECTION("A") {
    REQUIRE(_section->can_generate);
    SECTION("A1") {
    }
    REQUIRE(!_section->can_generate);
  }
  // запрещён, даже если секция не выполнялась в этом запуске
  REQUIRE(!_section->can_generate);
  SECTION("B") {
  }
  REQUIRE(!_section->can_generate);
}

// секция в цикле вызывает свой GENERATE на каждой итерации и получает одно и то же значение
TEST("SECTION IN LOOP") {
  for (int n = 0; n < 3; ++n) {
    SECTION("A") {
      REQUIRE(_section->can_generate);
      RUN("SECTION IN LOOP", GENERATE(1, 2));
      REQUIRE(!_section->can_generate);
    }
    REQUIRE(!_section->can_generate);
  }
}

TEST("IN EACH SECTION") {
  int i = GENERATE(1, 2);
  SECTION("A") {
    int j = GENERATE(10, 20);
    SECTION("A1") {
      int k = GENERATE(100, 200);
      RUN("IN EACH SECTION", i + j + k);
    }
  }
  SECTION("B") {
    int j = GENERATE(30, 40, 50);
    RUN("IN EACH SECTION", i + j);
  }
}

// GENERATE вложенной секции может зависеть от GENERATE родителя (в том числе количеством значений)
TEST("DEPENDS ON PARENT") {
  int i = GENERATE(1, 2, 3);
  SECTION("A") {
    int j = 0;
    if (i == 1)
      j = GENERATE(10, 20);
    else if (i == 2)
      j = GENERATE(30, 40, 50);
    RUN("DEPENDS ON PARENT", i + j);
  }
}

TEST("RANGE") {
  std::vector<int> v{1, 2, 3};
  int i = GENERATE_RANGE(v);
  REQUIRE(!_section->can_generate);
  SECTION("A") {
    int j = GENERATE_RANGE(std::array{10, 20});
    RUN("RANGE", i + j);
  }
  SECTION("B") {
    RUN("RANGE", i);
  }
}

inline std::vector<std::string> range_strings_result;

TEST("RANGE STRINGS") {
  std::string s = GENERATE_RANGE(std::vector<std::string>{"hello", "world"});
  range_strings_result.push_back(s);
}

// количество значений зависит от GENERATE родителя
TEST("RANGE DEPENDS ON PARENT") {
  int n = GENERATE(1, 2, 3);
  SECTION("A") {
    std::vector<int> v;
    for (int i = 0; i < n; ++i)
      v.push_back(n * 10 + i);
    RUN("RANGE DEPENDS ON PARENT", GENERATE_RANGE(v));
  }
}

#ifdef __cpp_lib_ranges_cartesian_product

TEST("RANGE CARTESIAN PRODUCT") {
  std::array ints{1, 2};
  std::vector<int> tens{10, 20, 30};
  auto [i, j] = GENERATE_RANGE(std::views::cartesian_product(ints, tens));
  RUN("RANGE CARTESIAN PRODUCT", i + j);
}

#endif

static int check(const std::string& name, const std::vector<int>& expected) {
  if (results[name] == expected)
    return 0;
  std::cerr << "TEST " << name << " FAILED, got:";
  for (int v : results[name])
    std::cerr << ' ' << v;
  std::cerr << '\n';
  return 1;
}

int main(int argc, char* argv[]) {
  auto& box = moko3::get_testbox();
  box.parse_config(argc, argv);
  int bad = box.run_tests();
  bad += check("SECTION IN LOOP", {1, 1, 1, 2, 2, 2});
  bad += check("IN EACH SECTION", {111, 211, 121, 221, 31, 41, 51, 112, 212, 122, 222, 32, 42, 52});
  bad += check("DEPENDS ON PARENT", {11, 21, 32, 42, 52, 3});
  bad += check("RANGE", {11, 21, 1, 12, 22, 2, 13, 23, 3});
  bad += check("RANGE DEPENDS ON PARENT", {10, 20, 21, 30, 31, 32});
  if (range_strings_result != std::vector<std::string>{"hello", "world"}) {
    std::cerr << "TEST RANGE STRINGS FAILED\n";
    ++bad;
  }
#ifdef __cpp_lib_ranges_cartesian_product
  bad += check("RANGE CARTESIAN PRODUCT", {11, 21, 31, 12, 22, 32});
#endif
  return bad;
}
