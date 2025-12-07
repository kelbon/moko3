#pragma once

#include <type_traits>
#include <concepts>

namespace moko3 {

// Проверяет корректность мув/копи конструкторов, присваивания, swap у типа T
//
// `empty` - объект, представляющий дефолтно сконструированный (move-out) стейт
// для тривиальных объектов его имеет смысл выставить в то же самое, что и `x` (н-р int)
//
// `Goodtype` подразумевает, что T{} ~== moveout T. Редкие типы ведут себя не так. Пример - std::string
// (move-out SSO string по стандарту валидно .size() != 0)
//
// `x` изменяется во время функции, но после функции остаётся тем же что и был (если тип правильный)
//
// `SelfMoveAssignHandled` true означает, что x = std::move(x) будет вести себя корректно.
// но даже для стандартных типов на некоторых библиотеках (std::vector gnu stl...) это не всегда так
template <bool Goodtype = true, bool SelfMoveAssignHandled = true, typename T>
void test_copy_move(T& x, const T& empty, auto is_equivalent) {
  static_assert(std::is_same_v<T, std::decay_t<T>>);

  // защита от типов аля auto_ptr с T(T&) != T(const T&)
  auto is_eq = [&](const T& a, const T& b) {
    bool r = is_equivalent(a, b);
    REQUIRE(r == is_equivalent(b, a));
    return r;
  };
  auto is_moveout = [&](const T& a) { return is_eq(a, empty); };

  REQUIRE(is_eq(x, x));
  REQUIRE(is_eq(empty, empty));

  if constexpr (Goodtype && std::default_initializable<T> && !std::is_trivially_copyable_v<T>) {
    T v{};
    REQUIRE(is_eq(v, v));
    REQUIRE(is_eq(v, empty));
  }

  const bool ismo = is_eq(x, empty);
  REQUIRE(is_eq(x, empty) == is_eq(empty, x));

  x = x;  // self copy assign
  REQUIRE(is_eq(x, x));

  const T xcopy = x;  // copy ctor

  REQUIRE(ismo == is_moveout(xcopy));
  REQUIRE(is_eq(xcopy, x));
  REQUIRE(is_eq(x, x));
  REQUIRE(is_eq(xcopy, xcopy));

  if constexpr (SelfMoveAssignHandled) {
    x = std::move(x);  // self move assign
    REQUIRE(is_eq(x, x));
    REQUIRE(ismo == is_moveout(x));
  }

  T xmv = std::move(x);  // move ctor
  if constexpr (Goodtype) {
    REQUIRE(ismo == is_moveout(xmv));
    REQUIRE(is_moveout(x));
  }
  REQUIRE(is_eq(xmv, xcopy));

  x = std::move(xmv);  // move assign

  REQUIRE(ismo == is_moveout(x));
  if constexpr (Goodtype)
    REQUIRE(is_moveout(xmv));
  REQUIRE(is_eq(x, xcopy));
  REQUIRE(is_eq(xmv, xmv));
  REQUIRE(is_eq(x, x));

  T xe = xmv;  // copy ctor on moveout

  if constexpr (Goodtype)
    REQUIRE(is_moveout(xe));
  REQUIRE(is_eq(xe, xmv));
  REQUIRE(is_eq(xe, xe));
  REQUIRE(is_eq(xmv, xmv));

  // должно быть изначальным x

  xe = x;  // copy assign

  REQUIRE(ismo == is_moveout(xe));
  REQUIRE(is_eq(xe, xe));
  REQUIRE(is_eq(x, x));
  REQUIRE(is_eq(xe, x));

  T ecopy = empty;
  REQUIRE(is_eq(ecopy, empty));

  using std::swap;

  swap(x, x);

  REQUIRE(ismo == is_moveout(x));
  REQUIRE(is_eq(x, x));

  swap(x, ecopy);

  REQUIRE(is_eq(ecopy, xcopy));
  REQUIRE(is_eq(x, empty));

  swap(ecopy, x);

  REQUIRE(is_eq(ecopy, empty));
  REQUIRE(is_eq(x, x));
  REQUIRE(is_eq(x, xcopy));
}

template <bool Goodtype = true, bool SelfMoveAssignHandled = true, typename T>
void test_copy_move(T& x, const T& empty) {
  return test_copy_move<Goodtype, SelfMoveAssignHandled>(x, empty,
                                                         [](const T& a, const T& b) { return a == b; });
}

}  // namespace moko3
