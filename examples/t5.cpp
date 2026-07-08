// clang-format off

// t5.cpp – Mixin functionality to the limit: dot notation, C++20 CRTP, C++23 deducing-this
//
// trait(T, ...) generates a nested Mixin type with dot-notation forwarding methods.
// Two paths are supported (auto-selected by the macro):
//
//   C++23 / Clang  (deducing-this):
//     struct Foo  : Shape::Mixin          { ... };   // 1-param trait
//     struct Foo  : Into::Mixin<int>       { ... };   // 2-param trait, extra params explicit
//
//   C++20  (CRTP):
//     struct Foo  : Shape::Mixin<Foo>      { ... };   // 1-param trait
//     struct Foo  : Into::Mixin<Foo, int>  { ... };   // 2-param trait
//
// The Mixin auto-detects whether to call the free function with the object by
// value/ref (e.g. area(Self)) or by pointer (e.g. scale(Self *, int)):
//
//   if constexpr (requires { ::Trait::func(self, ...); })
//     → calls by value / reference
//   else
//     → calls by pointer (&self)
//
// Demonstrates:
//   1. Value-result method (area)  vs. pointer-mutating method (scale)
//   2. Multi-param Mixin: Into::Mixin<T> generates a typed .into()
//   3. Multiple Mixin inheritance on one struct
//   4. Mixin + Dyn coexistence: same value usable both ways
//   5. Template struct with Mixin (generic container satisfying a trait)
//   6. Dyn's own Mixin methods (.area(), .into()) on type-erased values
//   7. Mixin is opt-in: trait satisfaction lives in Impl, not Mixin

#include <array>
#include <iostream>
#include <string>
#include <type_traits>
#include "../trait.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Traits used throughout
// ─────────────────────────────────────────────────────────────────────────────

// 1-param strict trait: value method + pointer method
trait(Shape, (Self), (
  (int,  area,  (Self)),
  (void, scale, (Self *, int))
))

// 1-param strict trait: value-returning only
trait(Printable, (Self), (
  (std::string, display, (Self))
))

// 2-param duck-typed trait: conversion
ducktyped_trait(Into, (Self, T), (
  (T, into, (Self))
))

// ─────────────────────────────────────────────────────────────────────────────
// 1.  Single-trait Mixin — value-result AND pointer-mutating methods
//
//     The Mixin method for area(Self) calls ::Shape::area(self).
//     The Mixin method for scale(Self*, int) calls ::Shape::scale(&self, f).
//     Both are accessed uniformly via dot notation.
// ─────────────────────────────────────────────────────────────────────────────

// C++23 / Clang syntax (no template argument on Mixin):
struct Circle : Shape::Mixin {
  int r;
};
template <> struct Shape::Impl<Circle> {
  static int  area (Circle c)         { return c.r * c.r; }
  static void scale(Circle *c, int f) { c->r *= f; }
};

// C++20 CRTP syntax (same struct, uncomment to use with GCC -std=c++20):
//   struct Circle : Shape::Mixin<Circle> { int r; };

struct Rect : Shape::Mixin {
  int x, y;
};
template <> struct Shape::Impl<Rect> {
  static int  area (Rect r)           { return r.x * r.y; }
  static void scale(Rect *r, int f)   { r->x *= f; r->y *= f; }
};

static_assert(Shape::Trait<Circle>);
static_assert(Shape::Trait<Rect>);
static_assert(Shape::Trait<Shape::Dyn>); // Dyn also satisfies (it has its own Impl)

// ─────────────────────────────────────────────────────────────────────────────
// 2.  Multi-param Mixin: Into::Mixin<T>
//
//     The generated `.into()` method is already typed to T.
//     C++23: struct Foo : Into::Mixin<T>
//     C++20: struct Foo : Into::Mixin<Foo, T>
// ─────────────────────────────────────────────────────────────────────────────
struct Celsius : Into::Mixin<float> {
  float v;
};
template <> struct Into::Impl<Celsius, float> {
  static float into(Celsius c) { return c.v; }
};

struct Score : Into::Mixin<int> {
  double raw;
};
template <> struct Into::Impl<Score, int> {
  static int into(Score s) { return static_cast<int>(s.raw); }
};

static_assert(Into::Trait<Celsius, float>);
static_assert(Into::Trait<Score, int>);
static_assert(Into::Trait<Into::Dyn<float>, float>); // Dyn satisfies too

// ─────────────────────────────────────────────────────────────────────────────
// 3.  Multiple Mixin inheritance on one struct
//
//     A struct can inherit from any number of Mixin types;
//     each contributes its own dot-notation methods independently.
// ─────────────────────────────────────────────────────────────────────────────
struct Widget : Shape::Mixin, Printable::Mixin {
  int side;
  std::string label;
};
template <> struct Shape::Impl<Widget> {
  static int  area (Widget w)          { return w.side * w.side; }
  static void scale(Widget *w, int f)  { w->side *= f; }
};
template <> struct Printable::Impl<Widget> {
  static std::string display(Widget w) {
    return "[Widget " + w.label + " side=" + std::to_string(w.side) + "]";
  }
};

static_assert(Shape::Trait<Widget>);
static_assert(Printable::Trait<Widget>);

// ─────────────────────────────────────────────────────────────────────────────
// 4.  Mixin + Dyn coexistence
//
//     The SAME value can be used via .area() (dot notation) AND converted
//     to a Shape::Dyn fat pointer for heterogeneous collections.
//     Shape::Dyn itself inherits from Shape::Mixin, so Dyn objects also
//     support dot notation.
// ─────────────────────────────────────────────────────────────────────────────

// Dyn::Mixin methods verified by trait check above.

// ─────────────────────────────────────────────────────────────────────────────
// 5.  Template struct with Mixin (generic container satisfying Shape)
//
//     A type that wraps any Shape::Trait type and itself satisfies Shape,
//     forwarding through to the inner element.  Inheriting from Shape::Mixin
//     gives the container its own .area() and .scale() dot notation.
// ─────────────────────────────────────────────────────────────────────────────
template <Shape::Trait T>
struct Scaled : Shape::Mixin {
  T inner;
  int factor;
};

template <Shape::Trait T>
struct Shape::Impl<Scaled<T>> {
  static int  area (Scaled<T> s)           { return Shape::area(s.inner) * s.factor; }
  static void scale(Scaled<T> *s, int f)   { Shape::scale(&s->inner, f); }
};

static_assert(Shape::Trait<Scaled<Circle>>);
static_assert(Shape::Trait<Scaled<Rect>>);

// ─────────────────────────────────────────────────────────────────────────────
// 6.  Mixin is opt-in: trait satisfaction lives entirely in Impl
//
//     A type that implements Shape but does NOT inherit from Mixin still
//     satisfies Shape::Trait and can be used with all free functions and
//     Shape::Dyn.  No Mixin inheritance required for trait satisfaction.
// ─────────────────────────────────────────────────────────────────────────────
struct Triangle {           // no Mixin base
  int base, height;
};
template <> struct Shape::Impl<Triangle> {
  static int  area (Triangle t)          { return t.base * t.height / 2; }
  static void scale(Triangle *t, int f)  { t->base *= f; t->height *= f; }
};

static_assert(Shape::Trait<Triangle>);          // ✓ satisfies without Mixin
// Triangle t; t.area();  ← would be a compile error (no Mixin methods)

// ─────────────────────────────────────────────────────────────────────────────
// 7.  Dyn's own Mixin methods: dot notation on type-erased values
//
//     Shape::Dyn inherits Shape::Mixin, so a fat pointer also supports
//     .area() and .scale().  Same for Into::Dyn<T>.
// ─────────────────────────────────────────────────────────────────────────────

// (Tested at runtime below)

// ─────────────────────────────────────────────────────────────────────────────
// 8.  Array of a Shape::Trait type itself satisfies Shape
//     (same pattern as t1.cpp, but now tested with dot notation via Mixin)
// ─────────────────────────────────────────────────────────────────────────────
template <Shape::Trait T, std::size_t N>
struct Shape::Impl<std::array<T, N>> {
  static int  area (std::array<T, N> a) {
    int t = 0; for (auto &e : a) t += Shape::area(e); return t;
  }
  static void scale(std::array<T, N> *a, int f) {
    for (auto &e : *a) Shape::scale(&e, f);
  }
};

static_assert(Shape::Trait<std::array<Circle, 3>>);

// ─────────────────────────────────────────────────────────────────────────────
// main – runtime verification of all scenarios
// ─────────────────────────────────────────────────────────────────────────────
int main() {
  // ── 1. Single-trait Mixin: dot notation for value and pointer methods ────
  Circle c{.r = 5};
  std::cout << c.area() << "\n";  // 25  (value method)
  c.scale(2);
  std::cout << c.area() << "\n";  // 100 (pointer method mutated c)

  Rect r{.x = 3, .y = 4};
  std::cout << r.area() << "\n";  // 12
  r.scale(3);
  std::cout << r.area() << "\n";  // 108 (x=9, y=12)

  // Free-function style still works alongside Mixin
  std::cout << Shape::area(c) << "\n";  // 100

  // ── 2. Multi-param Mixin: typed .into() ─────────────────────────────────
  Celsius cel{.v = 37.5f};
  std::cout << cel.into() << "\n";  // 37.5  (float)

  Score sc{.raw = 9.8};
  std::cout << sc.into() << "\n";   // 9     (int, truncated)

  // ── 3. Multiple Mixin inheritance ───────────────────────────────────────
  Widget w{.side = 4, .label = "btn"};
  std::cout << w.area() << "\n";      // 16
  std::cout << w.display() << "\n";   // [Widget btn side=4]
  w.scale(2);
  std::cout << w.area() << "\n";      // 64
  std::cout << w.display() << "\n";   // [Widget btn side=8]

  // ── 4. Mixin + Dyn coexistence ───────────────────────────────────────────
  Shape::Dyn dyn = c;
  std::cout << Shape::area(dyn) << "\n";  // 100 (free function)
  std::cout << dyn.area() << "\n";         // 100 (Dyn's own Mixin method)
  dyn.scale(3);
  std::cout << dyn.area() << "\n";         // 900

  Into::Dyn<float> idyn = cel;
  std::cout << Into::into<float>(idyn) << "\n"; // 37.5 (free function)
  std::cout << idyn.into() << "\n";              // 37.5 (Dyn's own Mixin method)

  // ── 5. Template struct with Mixin ────────────────────────────────────────
  Scaled<Rect> sr{.inner = {.x = 2, .y = 3}, .factor = 5};
  std::cout << sr.area() << "\n";  // (2*3) * 5 = 30
  sr.scale(2);
  // scale modifies inner Rect (x=4, y=6); factor unchanged
  std::cout << sr.area() << "\n";  // (4*6) * 5 = 120

  // ── 6. Trait without Mixin still works everywhere ────────────────────────
  Triangle tri{6, 4};
  std::cout << Shape::area(tri) << "\n";  // 12
  Shape::scale(&tri, 2);
  std::cout << Shape::area(tri) << "\n";  // 48

  // Dyn over a non-Mixin type
  Shape::Dyn tdyn = tri;
  std::cout << tdyn.area() << "\n";  // 48

  // ── 7. Array Impl via free functions and Dyn ─────────────────────────────
  Circle c1{.r = 2}, c2{.r = 3}, c3{.r = 4};
  std::array<Circle, 3> arr = {c1, c2, c3};
  std::cout << Shape::area(arr) << "\n";  // 4+9+16 = 29
}
