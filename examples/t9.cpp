// clang-format off
// t9 – cpp2-style ergonomic trait declarations
//
// Demonstrates the macro DSL from cpp2_trait.hpp where each method reads
// like a function declaration: `fn name(params) -> return_type`.
//
//   cpp2(
//       trait Shape as (
//           fn area  as ((Self)        -> int),
//           fn scale as ((Self *, int) -> void)
//       )
//   )

#include <cstdio>
#include "../trait.hpp"
#include "../cpp2_trait.hpp"

cpp2(
    trait Shape as (
        fn area  as ((Self)        -> int),
        fn scale as ((Self *, int) -> void)
    )

    duck_trait Show as (
        fn show as ((Self const &) -> void)
    )
)

CPP2_END

//----------------------------------------------------------------------
//  implementations
//----------------------------------------------------------------------

struct Circle : Shape::Mixin {
    int r;
};

template <> struct Shape::Impl<Circle> {
    static int  area (Circle c)         { return c.r * c.r; }
    static void scale(Circle *c, int f) { c->r *= f; }
};

template <> struct Show::Impl<Circle> {
    static void show(const Circle &c) {
        printf("Circle(r=%d)\n", c.r);
    }
};

struct Rect {
    int x, y;
};

template <> struct Shape::Impl<Rect> {
    static int  area (Rect r)           { return r.x * r.y; }
    static void scale(Rect *r, int f)   { r->x *= f; r->y *= f; }
};

template <> struct Show::Impl<Rect> {
    static void show(const Rect &r) {
        printf("Rect(%d x %d)\n", r.x, r.y);
    }
};

//----------------------------------------------------------------------
//  main
//----------------------------------------------------------------------

int main() {
    Circle c;
    c.r = 5;
    printf("%d\n", Shape::area(c));   // 25
    Shape::scale(&c, 2);
    printf("%d\n", Shape::area(c));   // 100

    Shape::Dyn sd = c;
    printf("%d\n", Shape::area(sd));  // 100

    Rect r{3, 4};
    Shape::Dyn arr[2] = {c, r};
    printf("%d %d\n",
           Shape::area(arr[0]),      // 100
           Shape::area(arr[1]));     // 12

    // Show trait (duck-typed, single-param)
    Show::show(c);                   // Circle(r=10)
    Show::show(r);                   // Rect(3 x 4)
    Show::Dyn sd2 = c;
    Show::show(sd2);                 // Circle(r=10) via vtable

    // Deducing-this method syntax (C++23)
    c.scale(2);
    printf("%d\n", c.area());        // 400
    c.show();                        // Circle(r=20)
}