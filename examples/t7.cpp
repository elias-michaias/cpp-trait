// t7.cpp -- C++20 fallback path: trait mechanism without method syntax.
//
// When compiled with `-std=c++20` (or any pre-C++23 standard), trait.hpp
// silently drops the deducing-this Mixin methods. Everything else --
// concepts, qualified free functions, `Impl<T>` specialisations, `Dyn`
// fat pointers, vtable dispatch -- still works. This file exercises that
// subset and is the explicit test that the C++20 fallback compiles and
// runs correctly.
//
// Compile with: clang++ -std=c++20 (or g++ -std=c++20).

#include "../trait.hpp"

#include <cstdio>

// 1-param trait.
trait(Shape, (Self), (
  (int,  area,  (Self)),
  (void, scale, (Self *, int))
))

// 2-param duck-typed trait. (Auto-registration still happens; the Mixin
// is just empty on C++20, so inheriting it via `Impls` contributes no
// methods -- which is fine because we use free-function calls here.)
ducktyped_trait(Into, (Self, T), (
  (T, into, (Self))
))

// Plain struct -- no Mixin base needed on C++20, since there are no
// methods to inherit. Trait satisfaction lives entirely in `Impl`.
struct Circle {
  int r;
};

template <> struct Shape::Impl<Circle> {
  static int  area (Circle c)         { return c.r * c.r; }
  static void scale(Circle *c, int f) { c->r *= f; }
};

struct Rect {
  int x, y;
};

template <> struct Shape::Impl<Rect> {
  static int  area (Rect r)           { return r.x * r.y; }
  static void scale(Rect *r, int f)   { r->x *= f; r->y *= f; }
};

struct MyFloat {
  float v;
};

template <> struct Into::Impl<MyFloat, int> {
  static int into(const MyFloat &f) { return (int)f.v; }
};

template <> struct Into::Impl<float, int> {
  static int into(const float &f) { return (int)f; }
};

// Concepts still work.
static_assert(Shape::Trait<Circle>);
static_assert(Shape::Trait<Rect>);
static_assert(Shape::Trait<Shape::Dyn>);
static_assert(Into::Trait<MyFloat, int>);
static_assert(Into::Trait<float, int>);
static_assert(Into::Trait<Into::Dyn<int>, int>);

// `Impls<D>` is still usable as a base -- it just contributes no methods on
// C++20. Inheriting it is harmless and forward-compatible: the same
// struct recompiled under C++23 would pick up method syntax for free.
struct Square : Impls<Square> {
  int s;
};
template <> struct Shape::Impl<Square> {
  static int  area (Square q)         { return q.s * q.s; }
  static void scale(Square *q, int f) { q->s *= f; }
};
static_assert(Shape::Trait<Square>);

int main() {
  // Everything goes through the qualified free functions -- the
  // canonical trait API. No `.area()` / `.scale()` / `.into()` here.
  Circle c{.r = 5};
  printf("%d\n", Shape::area(c));     // 25
  Shape::scale(&c, 2);
  printf("%d\n", Shape::area(c));     // 100

  Rect r{3, 4};
  printf("%d\n", Shape::area(r));     // 12

  // Dyn fat pointers still work -- the vtable layer doesn't depend on
  // deducing this.
  Shape::Dyn arr[2] = {c, r};
  printf("%d %d\n", Shape::area(arr[0]), Shape::area(arr[1])); // 100 12
  Shape::scale(&arr[0], 3);
  printf("%d\n", Shape::area(arr[0])); // 900

  // 2-param trait via qualified free function.
  MyFloat f{.v = 3.7f};
  float f2 = 4.9f;
  printf("%d\n", Into::into<int>(f));  // 3
  printf("%d\n", Into::into<int>(f2)); // 4

  Into::Dyn<int> id(f);
  Into::Dyn<int> f3 = f2;
  printf("%d\n", Into::into<int>(id)); // 3
  printf("%d\n", Into::into<int>(f3)); // 4

  // `Square` works the same way -- `Impls` is an empty base here, but
  // trait satisfaction still flows through `Impl<Square>`.
  Square q{.s = 3};
  printf("%d\n", Shape::area(q));     // 9
  Shape::scale(&q, 3);
  printf("%d\n", Shape::area(q));     // 81
}
