# cpp-trait

Traits for C++ (20)

```c++
trait(Shape, (T), (
  (int, area, (T)),
  (void, scale, (T *, int))
))

struct Circle { int r; };

template<> 
struct Shape::Impl<Circle> {
  static int area(Circle c) { return c.r * c.r; }
  static void scale(Circle *c, int f) { c->r *= f; }
};

Circle c{5};
std::cout << Shape::area(c) << "\n"; // 25
Shape::scale(&c, 2);
std::cout << Shape::area(c) << "\n"; // 100
```

## Goals

This library is designed to enhance "C-style C++" with more robust polymorphism capabilities that perform well, read well, and play well with C's memory paradigm.
That being said, Rust and Haskell are key inspirations for this library's approach to principled ad-hoc polymorphism.
If the library has done its job, writing trait-centric APIs in C should feel like you have the compositional and type-level power of Rust with the control of C.
As such, the following goals are north stars for this library:

- Readable APIs
- Single header
- Minimal reliance on C++ `std`
- No heap allocation
- Respect pointer semantics
- Static functions preferred to instance functions
- Recursive static dispatch works exactly as you would expect
- Static dispatch first, dynamic dispatch allowed via `Trait::Dyn` type
- Concepts for trait constraints instead of arcane SFINAE errors

## Non-goals

This library is not designed for traditional C++ OOP, but is rather an alternative to it.
Additionally, this library has no intention of replicating Rust in C++ for Rust's sake.
The following are not supported, and will never be supported:

- Automatic memory management
- Any implementation of Rust/C++ move semantics
- Compatibility with OOP hierarchies

## Probably non-goals

I don't see a reason to implement these, but I'm not saying no forever:

- Dot notation (e.g. `obj.method()`)

## Examples

```c++
#include <array>
#include <iostream>
#include "../trait.hpp"

// 1-param: Shape
trait(Shape, (T), (
  (int, area, (T)),
  (void, scale, (T *, int))
))

struct Circle {
  int r;
};

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
  std::cout << Into::into<int>(f2) << "\n"; // 4
  // also via Dyn directly:
  std::cout << Into::into<int>(f) << "\n"; // 3
  std::cout << Into::into<int>(f3) << "\n"; // 4
                                      
  std::array<Into::Dyn<int>, 2> list = {id, f3};

  f3 = f;
}
```
