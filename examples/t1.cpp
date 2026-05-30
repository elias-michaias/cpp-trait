// clang-format off

#include <array>
#include <iostream>
#include "../trait.hpp"

// 1-param: Shape
trait(Shape, (T), (
  (int, area, (T)),
  (void, scale, (T *, int))
))

struct Circle { int r; };

template <> struct Shape::Impl<Circle> {
  static int area(Circle c) { return c.r * c.r; }
  static void scale(Circle *c, int f) { c->r *= f; }
};

struct Rect {
  int x, y;
};
template <> struct Shape::Impl<Rect> {
  static int area(const Rect &r) { return r.x * r.y; }
  static void scale(Rect *r, int f) {
    r->x *= f;
    r->y *= f;
  }
};

template <Shape::Trait T, std::size_t N> struct Shape::Impl<std::array<T, N>> {
  static int area(const std::array<T, N> &a) {
    int t = 0;
    for (auto &i : a)
      t += Shape::area(i);
    return t;
  }
  static void scale(std::array<T, N> *a, int f) {
    for (auto &i : *a)
      Shape::scale(&i, f);
  }
};

static_assert(Shape::Trait<Circle>);
static_assert(Shape::Trait<Rect>);
static_assert(Shape::Trait<std::array<Rect, 2>>);
static_assert(Shape::Trait<Shape::Dyn>);
static_assert(Shape::Trait<std::array<Shape::Dyn, 3>>);

// 2-param: Into<T, R>
trait(Into, (T, V), (
  (V, into, (const T &))
))

struct MyFloat {
  float v;
};

template <> struct Into::Impl<MyFloat, int> {
  static int into(const MyFloat &f) { return (int)f.v; }
};

template <> struct Into::Impl<float, int> {
  static int into(const float &f) { return (int)f; }
};

static_assert(Into::Trait<MyFloat, int>);
static_assert(Into::Trait<Into::Dyn<int>, int>);

int main() {
  // 1-param: Shape
  Circle c{5};
  std::cout << Shape::area(c) << "\n"; // 25
  Shape::scale(&c, 2);
  std::cout << Shape::area(c) << "\n"; // 100

  Shape::Dyn sd(c);
  std::cout << Shape::area(sd) << "\n"; // 100
  Shape::scale(&sd, 3);
  std::cout << Shape::area(sd) << "\n"; // 900

  Rect r{3, 4};
  std::array<Shape::Dyn, 2> arr = {c, r};
  std::cout << Shape::area(arr[0]) << " " << Shape::area(arr[1]) << "\n"; // 900 12

  // 2-param: Into
  MyFloat f{3.7f};
  float f2 = 4.9;
  Into::Dyn<int> id(f);
  Into::Dyn<int> f3 = f2;
  std::cout << Into::into<int>(id) << "\n"; // 3
  std::cout << Into::into<int>(f2) << "\n"; // 3
  // also via Dyn directly:
  std::cout << Into::into<int>(f) << "\n"; // 3
  std::cout << Into::into<int>(f3) << "\n"; // 3
                                      
  std::array<Into::Dyn<int>, 2> list = {id, f3};

  f3 = f;
}
