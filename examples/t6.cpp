// t6.cpp -- Impls<D>: registration-driven mixin aggregation.
//
// Demonstrates:
//   * defining a small trait library -- each `trait(...)` reserves its
//     Impls layer automatically (no separate registration line), so
//     `struct T : Impls<T>` auto-inherits every registered `Mixin` through
//     a LINEAR chain of layers.
//   * conflict resolution via method hiding + if-constexpr fallback:
//     when two registered traits share a method name (Shape::scale and
//     Transform::scale), the LAST-registered trait's layer HIDES the
//     earlier one. The layer method tries its own trait first, then falls
//     back to the previous layer. So:
//       - Square only implements Shape → q.scale(3) works unqualified:
//         Transform's layer tries Transform::scale, fails (no Impl),
//         falls back to Shape's layer, which calls Shape::scale.
//       - Circle implements both → c.scale(2) calls Transform::scale
//         (the topmost layer wins). To reach Shape::scale, use the
//         qualified free function: Shape::scale(&c, 2).
//   * types that don't specialise a trait's Impl just don't have viable
//     `if constexpr` branches for that trait's methods -- calling
//     `c.perimeter()` on Circle (no Metric impl) is a compile error.

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

// Each 1-param trait above is auto-registered by its `trait(...)` macro.
// (2- or 3-param traits are silently skipped -- their Mixin is a template.)

// ---------------------------------------------------------------------------
//  Circle -- satisfies Shape, Drawable, Transform, Named (NOT Metric)
// ---------------------------------------------------------------------------

struct Circle : Impls<Circle> {
  int r;
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
//  Square -- satisfies Shape only (NOT Transform)
// ---------------------------------------------------------------------------

// Square only specialises Shape, not Transform. But thanks to the linear
// layer chain, `q.scale(3)` still works unqualified: Transform's layer
// tries Transform::scale, fails (no Impl<Square>), and falls back to
// Shape's layer, which calls Shape::scale. No qualified call needed.
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
  Shape::scale(&c, 2);           // qualified: Circle implements both Shape
  printf("%d\n",   c.area());     // 100  AND Transform, so `c.scale(2)` goes
  c.draw();                        // circle(r=10)   (Drawable, via Mixin)
  printf("%s\n",   c.name());     // circle         (Named,    via Mixin)

  Transform::scale(&c, 2);       // the other scale, also qualified
  Transform::rotate(&c, 45.0);

  // Circle doesn't specialise Metric, so `c.perimeter()` would be a
  // compile error -- exactly the right response. The Rect below does.

  Square q{.s = 3};
  printf("%d\n",   q.area());     // 9
  q.scale(3);                     // unqualified! Square only implements Shape,
  printf("%d\n",   q.area());     // 81   so the layer fallback hits Shape::scale
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
