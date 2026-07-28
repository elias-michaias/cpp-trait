// t6.cpp -- Impls<Derived>: registration-driven mixin aggregation.
//
// Demonstrates:
//   * defining a small trait library and registering each trait so that
//     `struct T : Impls<T>` auto-inherits every registered `Mixin`,
//   * a real method-name conflict (Shape::scale vs Transform::scale). The
//     expected way to resolve such conflicts is to skip the mixin sugar and
//     call the trait's fully-qualified free function, e.g.
//       Shape::scale(&c, 2);     // geometric scale (r *= f)
//       Transform::scale(&c, 2); // affine scale (independent)
//     Both behaviours stay reachable; nothing needs to be hidden or pulled
//     into scope. The free functions are the canonical trait API -- mixin
//     methods are ergonomic sugar for the conflict-free case.
//   * the Mixin's `if constexpr (requires { ... })` dispatch quietly
//     rejecting `Metric::perimeter(c)` for Circle (no `Metric::Impl<Circle>`
//     specialised) -- asking Circle for its perimeter is a compile error,
//     exactly the right response, rather than crashing name lookup.

#include "../trait.hpp"

#include <cstdio>

// ---------------------------------------------------------------------------
//  Trait library
// ---------------------------------------------------------------------------

trait(Shape, (Self), (
  (int,  area,  (Self)),
  (void, scale, (Self *, int))     // geometric scaling: r *= f
))

trait(Drawable, (Self), (
  (void, draw, (Self))
))

trait(Transform, (Self), (
  (void, scale,  (Self *, int)),   // affine scale -- NB: also named `scale`
  (void, rotate, (Self *, double))
))

trait(Metric, (Self), (
  (double, perimeter, (Self))
))

trait(Named, (Self), (
  (const char *, name, (Self))
))

// Register each trait so Impls<D> pulls its Mixin into the chain.
register_trait(Shape)
register_trait(Drawable)
register_trait(Transform)
register_trait(Metric)
register_trait(Named)

// ---------------------------------------------------------------------------
//  Circle -- satisfies Shape, Drawable, Transform, Named (NOT Metric)
// ---------------------------------------------------------------------------

struct Circle : Impls<Circle> {
  int r;
  // No using-declaration: `scale` is ambiguous between Shape::Mixin and
  // Transform::Mixin. Both behaviours are reachable through their trait's
  // qualified free function -- see main().
};

template <> struct Shape::Impl<Circle> {
  static int area(Circle c) { return c.r * c.r; }
  static void scale(Circle *c, int f) { c->r *= f; }
};

template <> struct Drawable::Impl<Circle> {
  static void draw(Circle c) { printf("circle(r=%d)\n", c.r); }
};

template <> struct Transform::Impl<Circle> {
  // Affine scale: pretend the radius grows by `f` but do nothing so the
  // demo output is unchanged. (We only want to show the call resolves.)
  static void scale(Circle *, int) {}
  static void rotate(Circle *, double) {}
};

template <> struct Named::Impl<Circle> {
  static const char *name(Circle) { return "circle"; }
};

// ---------------------------------------------------------------------------
//  Square -- satisfies Shape only
// ---------------------------------------------------------------------------

// Square only *specialises* Shape, but `Impls<Square>` still inherits
// Transform::Mixin unconditionally (see the note on `register_trait`), so
// `Transform::Mixin::scale` is also visible here. Just like Circle, the
// `scale` name is ambiguous and must be called with a qualified free
// function -- `Shape::scale(&q, f)` below.
struct Square : Impls<Square> {
  int s;
};

template <> struct Shape::Impl<Square> {
  static int area(Square q) { return q.s * q.s; }
  static void scale(Square *q, int f) { q->s *= f; }
};

template <> struct Named::Impl<Square> {
  static const char *name(Square) { return "square"; }
};

// ---------------------------------------------------------------------------
//  Rect -- satisfies Shape AND Metric (and is Named) -- no conflicts here
// ---------------------------------------------------------------------------

struct Rect : Impls<Rect> {
  int w, h;
};

template <> struct Shape::Impl<Rect> {
  static int area(Rect r) { return r.w * r.h; }
  static void scale(Rect *r, int f) { r->w *= f; r->h *= f; }
};

template <> struct Metric::Impl<Rect> {
  static double perimeter(Rect r) { return 2.0 * (r.w + r.h); }
};

template <> struct Named::Impl<Rect> {
  static const char *name(Rect) { return "rect"; }
};

// ---------------------------------------------------------------------------
//  driver
// ---------------------------------------------------------------------------

int main() {
  Circle c{.r = 5};
  printf("%d\n",   c.area());     // 25   (Shape, via Mixin -- no conflict)
  Shape::scale(&c, 2);           // qualified: geometric scale (r *= f)
  printf("%d\n",   c.area());     // 100
  c.draw();                        // circle(r=10)   (Drawable, via Mixin)
  printf("%s\n",   c.name());     // circle         (Named,    via Mixin)

  // The *other* scale, also qualified, also unambiguous -- both behaviours
  // stay reachable when a name collides.
  Transform::scale(&c, 2);       // affine (no-op here)
  Transform::rotate(&c, 45.0);

  // Circle doesn't specialise Metric, so `c.perimeter()` would be a
  // compile error -- exactly the right response. The Rect below does.
  // (Uncommenting `c.perimeter();` should fail to compile.)

  Square q{.s = 3};
  printf("%d\n",   q.area());     // 9
  Shape::scale(&q, 3);           // qualified: Shape is the only `scale` here
  printf("%d\n",   q.area());     // 81
  printf("%s\n",   q.name());     // square

  Rect r{.w = 4, .h = 5};
  printf("%d\n",   r.area());     // 20
  printf("%g\n",   r.perimeter());// 18   (Metric, via Mixin -- no conflict)
  printf("%s\n",   r.name());     // rect
}

// ---------------------------------------------------------------------------
//  Static checks
// ---------------------------------------------------------------------------

static_assert(Shape::Trait<Circle>);
static_assert(Drawable::Trait<Circle>);
static_assert(Transform::Trait<Circle>);
static_assert(Named::Trait<Circle>);
static_assert(!Metric::Trait<Circle>);  // no Impl<Circle> specialised

static_assert(Shape::Trait<Square>);
static_assert(Named::Trait<Square>);
static_assert(!Drawable::Trait<Square>);
static_assert(!Transform::Trait<Square>);
static_assert(!Metric::Trait<Square>);

static_assert(Shape::Trait<Rect>);
static_assert(Metric::Trait<Rect>);
static_assert(Named::Trait<Rect>);
static_assert(!Drawable::Trait<Rect>);
