#include "../trait.hpp"
#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

// Original-style traits remain intact.
trait(Shape, (Self), ((int, area, (Self)), (void, scale, (Self *, int))))

    ducktyped_trait(Into, (Self, T), ((T, into, (Self))))

    // Updated higher-order static traits.
    static_trait(Functor, (Self),
                 ((type, value_type), (template, Mapped, (U)),
                  hof((template, Mapped, (U)), map,
                      (Self, fn(U, value_type))), ))

        static_trait(Zip, (Self),
                     ((type, value_type), (template, Mapped, (U)),
                      hof((template, Mapped, (U)), zip_with,
                          (Self, Self, fn(R, value_type, value_type))), ))

            static_trait(Compare, (Self),
                         ((type, value_type),
                          hof(bool, less_than,
                              (Self, Self,
                               fn(bool, value_type, value_type))), ))

                static_trait(Test, (T),
                             ((type, Factor),
                              hof(T, test, (T, fn(T, Factor))), ))

                    template <typename T>
                    struct Box {
  T inner;
};

template <typename T> struct Maybe {
  std::optional<T> inner;
};

struct Circle {
  int r;
};

struct Rect {
  int x, y;
};

struct MyFloat {
  float v;
};

// Shape impls
template <> struct Shape::Impl<Circle> {
  static int area(Circle c) { return c.r * c.r; }
  static void scale(Circle *c, int f) { c->r *= f; }
};

template <> struct Shape::Impl<Rect> {
  static int area(Rect r) { return r.x * r.y; }
  static void scale(Rect *r, int f) {
    r->x *= f;
    r->y *= f;
  }
};

// Into impls
template <> struct Into::Impl<MyFloat, int> {
  static int into(const MyFloat &f) { return static_cast<int>(f.v); }
};

template <> struct Into::Impl<float, int> {
  static int into(const float &f) { return static_cast<int>(f); }
};

// Helper operations
static int add1(int x) { return x + 1; }
static std::string to_string2(int x) { return std::to_string(x); }
static bool is_less(int a, int b) { return a < b; }

// Higher-order trait impls
template <typename T> struct Functor::Impl<Box<T>> {
  using value_type = T;

  template <typename U> using Mapped = Box<U>;

  template <class F>
  static auto map(Box<T> b, F &&fn)
      -> Mapped<std::remove_cvref_t<std::invoke_result_t<F &, T>>> {
    using Out = std::remove_cvref_t<std::invoke_result_t<F &, T>>;
    return Mapped<Out>{std::invoke(std::forward<F>(fn), b.inner)};
  }
};

template <typename T> struct Zip::Impl<Box<T>> {
  using value_type = T;

  template <typename U> using Mapped = Box<U>;

  template <class F>
  static auto zip_with(Box<T> a, Box<T> b, F &&fn)
      -> Mapped<std::remove_cvref_t<std::invoke_result_t<F &, T, T>>> {
    using Out = std::remove_cvref_t<std::invoke_result_t<F &, T, T>>;
    return Mapped<Out>{std::invoke(std::forward<F>(fn), a.inner, b.inner)};
  }
};

template <typename T> struct Compare::Impl<Box<T>> {
  using value_type = T;

  template <class F> static bool less_than(Box<T> a, Box<T> b, F &&fn) {
    return static_cast<bool>(
        std::invoke(std::forward<F>(fn), a.inner, b.inner));
  }
};

template <> struct Test::Impl<int> {
  using Factor = bool;
  static int test(int l, bool r) { return (l && r) ? l : 0; }
};

static_assert(Shape::Trait<Circle>);
static_assert(Shape::Trait<Rect>);
static_assert(Into::Trait<MyFloat, int>);
static_assert(Into::Trait<float, int>);
static_assert(Functor::Trait<Box<int>>);
static_assert(Zip::Trait<Box<int>>);
static_assert(Compare::Trait<Box<int>>);
static_assert(Test::Trait<int>);

static_assert(
    std::same_as<decltype(Functor::map(Box<int>{41}, add1)), Box<int>>);
static_assert(
    std::same_as<decltype(Functor::map(
                     Box<int>{41}, [](auto x) { return std::to_string(x); })),
                 Box<std::string>>);
static_assert(
    std::same_as<decltype(Zip::zip_with(Box<int>{2}, Box<int>{3},
                                        [](auto a, auto b) { return a + b; })),
                 Box<int>>);
static_assert(
    std::same_as<
        decltype(Compare::less_than(Box<int>{2}, Box<int>{3}, is_less)), bool>);
static_assert(std::same_as<decltype(Test::test(2, true)), int>);

int main() {
  Circle c{.r = 5};
  std::cout << Shape::area(c) << "\n";
  Shape::scale(&c, 2);
  std::cout << Shape::area(c) << "\n";

  Rect r{3, 4};
  std::cout << Shape::area(r) << "\n";

  MyFloat f{3.7f};
  std::cout << Into::into<int>(f) << "\n";
  std::cout << Into::into<int>(4.9f) << "\n";

  Box<int> b{41};
  auto b2 = Functor::map(b, add1);
  std::cout << b2.inner << "\n";

  auto b3 = Functor::map(b, [](auto x) { return std::to_string(x); });
  std::cout << b3.inner << "\n";

  auto z = Zip::zip_with(Box<int>{2}, Box<int>{3},
                         [](auto a, auto b) { return a + b; });
  std::cout << z.inner << "\n";

  std::cout << std::boolalpha
            << Compare::less_than(Box<int>{2}, Box<int>{3}, is_less) << "\n";
  std::cout << Test::test(2, true) << "\n";
}
