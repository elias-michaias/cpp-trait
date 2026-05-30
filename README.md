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

This library is designed to enhance **"C-style C++"** with more robust polymorphism capabilities that perform well, read well, and play well with C's memory paradigm.
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

- [t1.cpp](https://github.com/elias-michaias/cpp-trait/blob/main/examples/t1.cpp)
