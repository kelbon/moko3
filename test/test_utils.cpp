
#include <moko3/moko3.hpp>
#include <moko3/utils.hpp>

TEST("special member functions test test") {
  using moko3::test_copy_move;

  std::vector<int> vec;
  test_copy_move(vec, vec);
  REQUIRE(vec.empty());

  std::vector<int> vec2{1, 2, 3, 4, 5};
  test_copy_move</*Goodtype*/ true, /*self move assign handled*/ false>(vec2, vec);
  REQUIRE(vec2 == std::vector<int>{1, 2, 3, 4, 5});

  test_copy_move</*Goodtype=*/false, /*self move assign handled*/ false>(vec2, vec2);
  REQUIRE(vec2 == std::vector<int>{1, 2, 3, 4, 5});

  std::vector<int> badempty{1, 2};
  REQUIRE(badempty != vec2 && badempty != std::vector<int>{});
  test_copy_move</*Goodtype=*/false, /*self move assign handled*/ false>(vec2, badempty);
  REQUIRE(vec2 == std::vector<int>{1, 2, 3, 4, 5});

  int i = 42;
  test_copy_move(i, i);
  REQUIRE(i == 42);

  // non SSO
  std::string s(sizeof(s) * 2, 'a');
  std::string se;
  test_copy_move</*Goodtype=*/true, /*self move assign handled*/ false>(s, se);
  REQUIRE(s == std::string(sizeof(s) * 2, 'a'));

  // SSO
  std::string s1("a");
  test_copy_move</*Goodtype=*/false, /*self move assign handled*/ false>(s1, se);
  REQUIRE(s1 == "a");

  std::shared_ptr<int> p = std::make_shared<int>(5);
  std::shared_ptr<int> pe = nullptr;
  test_copy_move(p, pe);
  REQUIRE(p && *p == 5);
}

MOKO3_MAIN;
