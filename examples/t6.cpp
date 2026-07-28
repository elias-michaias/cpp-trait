// clang-format off
// t6.cpp -- Impls<D>: registration-driven mixin aggregation + multi-param traits.
//
// Demonstrates:
//   * defining a small trait library -- each `trait(...)` / `ducktyped_trait(...)`
//     reserves its Impls layer automatically, so `struct T : Impls<T>` auto-
//     inherits every registered Mixin through a LINEAR chain of layers.
//   * conflict resolution via method hiding + if-constexpr fallback:
//     when two registered traits share a method name (Shape::scale and
//     Transform::scale), the LAST-registered trait's layer HIDES the
//     earlier one. The layer method tries its own trait first, then falls
//     back to the previous layer.
//   * types that don't specialise a trait's Impl just don't have viable
//     `if constexpr` branches for that trait's methods -- calling
//     `c.perimeter()` on Circle (no Metric impl) is a compile error.
//
// Multi-param trait tests:
//   * 2-param traits (Into, Add, Comparable) registered via
//     TRAIT_MAYBE_REGISTER: the extra type params move onto each layer
//     method as a template head, so `meter.add(5)` deduces T=int from
//     the argument and dispatches through the Impls chain.
//   * Dyn for 2-param traits: Dyn<T> supports member dispatch, free
//     function dispatch, and satisfies the trait concept.
//   * qualified free function dispatch through both concrete types and Dyn.

#include "../trait.hpp"

#include <cstdio>

// ---------------------------------------------------------------------------
//  Trait library
// ---------------------------------------------------------------------------

// -- 1-param strict traits ---------------------------------------------------

trait(Shape, (Self), (
  (int,  area,  (Self)),
  (void, scale, (Self *, int))
))

trait(Drawable, (Self), (
  (void, draw, (Self))
))

trait(Transform, (Self), (
  (void, scale,  (Self *, int)),
  (void, rotate, (Self *, double))
))

trait(Metric, (Self), (
  (double, perimeter, (Self))
))

trait(Named, (Self), (
  (const char *, name, (Self))
))

// -- 2-param duck-typed traits -----------------------------------------------

// Into: parametric return type -- T is what you get out.
ducktyped_trait(Into, (Self, T), (
  (T, into, (Self))
))

// Add: parametric argument type -- T is the right-hand operand.
// Return type is T (not Self) because in a 2-param trait, Self is the
// deducing-this receiver and is not available as a return type in the
// generated Mixin/Dyn methods.  T IS a method-level template parameter.
ducktyped_trait(Add, (Self, T), (
  (T, add, (Self, T))
))

// Comparable: parametric argument type, bool return.
ducktyped_trait(Comparable, (Self, T), (
  (bool, eq, (Self, T))
))

// All traits above (including 2-param) are auto-registered by their
// `trait(...)` / `ducktyped_trait(...)` macros.  The layer methods for
// 2-param traits become per-method templates:
//   template <typename T> T    into(this auto &self);
//   template <typename T> T    add (this auto &self, T p1);
//   template <typename T> bool eq  (this auto &self, T p1);
// These participate in the same layer chain as 1-param trait methods.

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
//  Meter -- satisfies Shape, Into<float>, Add<int>, Named
//
//  Exercises multi-param traits through the Impls chain:
//    meter.into<float>()   -- parametric return type (T=float)
//    meter.add(5)          -- parametric argument type (T=int, deduced), returns T
//    meter.area()          -- 1-param trait (Shape)
// ---------------------------------------------------------------------------

struct Meter : Impls<Meter> {
  int v;
};

template <> struct Shape::Impl<Meter> {
  static int area(Meter m) { return m.v; }
  static void scale(Meter *m, int f) { m->v *= f; }
};

template <> struct Into::Impl<Meter, float> {
  static float into(Meter m) { return static_cast<float>(m.v); }
};

template <> struct Add::Impl<Meter, int> {
  static int add(Meter m, int n) { return m.v + n; }
};

template <> struct Named::Impl<Meter> {
  static const char *name(Meter) { return "meter"; }
};

// ---------------------------------------------------------------------------
//  Box -- satisfies Into<double>, Add<double>, Comparable<Box>
//
//  Exercises 2-param traits where both Self and T are involved:
//    box.into<double>()  -- parametric return type
//    box.add(2.5)        -- parametric argument type, T=double deduced, returns T
//    a.eq(b)             -- parametric argument type where T == Self
// ---------------------------------------------------------------------------

struct Box : Impls<Box> {
  double side;
};

template <> struct Into::Impl<Box, double> {
  static double into(Box b) { return b.side; }
};

template <> struct Add::Impl<Box, double> {
  static double add(Box a, double b) { return a.side + b; }
};

template <> struct Comparable::Impl<Box, Box> {
  static bool eq(Box a, Box b) { return a.side == b.side; }
};

// ---------------------------------------------------------------------------
//  driver
// ---------------------------------------------------------------------------

int main() {
  // -- Original 1-param trait tests -----------------------------------------

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

  // -- 2-param traits via Impls layer chain ---------------------------------

  // Into: parametric return type -- T is specified at the call site.
  Meter m{.v = 42};
  printf("%g\n", m.into<float>());           // 42   (Into, parametric return)
  printf("%g\n", Into::into<float>(m));      // 42   (qualified free function)

  // Add: parametric argument type -- T is deduced from the argument.
  int m2 = m.add(8);
  printf("%d\n", m2);                        // 50   (Add, T deduced as int)
  printf("%d\n", Add::add(m, 8));            // 50   (qualified free function)

  // Comparable: parametric argument type, bool return.
  Box a{.side = 3.0}, b{.side = 3.0}, d{.side = 7.5};
  printf("%d\n", a.eq(b));                   // 1    (true, same side)
  printf("%d\n", a.eq(d));                   // 0    (false, different side)
  printf("%d\n", Comparable::eq(a, b));      // 1    (qualified free function)

  // Meter also has Shape and Named (1-param), mixing with 2-param traits.
  printf("%d\n", m.area());                  // 42   (Shape, 1-param)
  printf("%s\n", m.name());                  // meter (Named, 1-param)
  Shape::scale(&m, 3);
  printf("%d\n", m.area());                  // 126  (scaled)

  // Box also has Into<double> -- multi-param and 1-param coexistence.
  printf("%g\n", a.into<double>());          // 3    (Into<double>)
  printf("%g\n", Add::add(a, 2.5));          // 5.5  (Add<double>)

  // -- Dyn for 1-param traits ----------------------------------------------

  Shape::Dyn sd = m;
  printf("%d\n", sd.area());                 // 126  (Dyn dispatch, 1-param)
  sd.scale(2);
  printf("%d\n", sd.area());                 // 252

  Shape::Dyn sd2 = q;
  printf("%d\n", sd2.area());                // 81   (Square via Dyn)

  // -- Dyn for 2-param traits ----------------------------------------------
  //
  // Dyn<T> for 2-param traits works fully:
  //   - Construction: Dyn<T> ctor checks Trait<ConcreteType, T>.
  //   - Member dispatch: dyn.add(5) works when T is deducible from args.
  //     For parametric-return traits (Into), use dyn.into() -- the T is
  //     baked into the Dyn<T> class template parameter.
  //   - Free function dispatch: Add::add(dyn, 5) and Into::into<T>(dyn)
  //     work via the Impl<Dyn<T>, T> partial specialization.
  //   - Static asserts: Dyn<T> satisfies the trait concept.

  // Construction works:
  Add::Dyn<int> adyn = m;       // OK: Meter satisfies Add<_, int>
  Into::Dyn<float> idyn = m;    // OK: Meter satisfies Into<_, float>

  // Member dispatch through Dyn<T> for 2-param traits:
  printf("%d\n", adyn.add(5));              // 55  (Dyn dispatch, 2-param, T deduced from arg)
  printf("%g\n", idyn.into());              // 42  (Dyn dispatch, T baked into Dyn<float>)

  // Free function dispatch through Dyn<T>:
  printf("%d\n", Add::add(adyn, 5));        // 60  (free function on Dyn)
  printf("%g\n", Into::into<float>(idyn));  // 42  (free function on Dyn, explicit T)

  // Direct vtable access (low-level, also works):
  printf("%d\n", adyn.vtable->add(adyn.object, 5));   // 65

  // For comparison, 1-param Dyn dispatch works fully:
  printf("%d\n", sd.area());    // 252  (Dyn member call via vtable)

  // -- Static checks: 1-param traits ---------------------------------------

  static_assert(Shape::Trait<Circle>);
  static_assert(Drawable::Trait<Circle>);
  static_assert(Transform::Trait<Circle>);
  static_assert(Named::Trait<Circle>);
  static_assert(!Metric::Trait<Circle>);

  static_assert(Shape::Trait<Square>);
  static_assert(Named::Trait<Square>);
  static_assert(!Drawable::Trait<Square>);
  static_assert(!Transform::Trait<Square>);
  static_assert(!Metric::Trait<Square>);

  static_assert(Shape::Trait<Rect>);
  static_assert(Metric::Trait<Rect>);
  static_assert(Named::Trait<Rect>);
  static_assert(!Drawable::Trait<Rect>);

  // -- Static checks: 2-param traits ---------------------------------------

  static_assert(Into::Trait<Meter, float>);
  static_assert(Add::Trait<Meter, int>);
  static_assert(!Comparable::Trait<Meter, float>);

  static_assert(Into::Trait<Box, double>);
  static_assert(Add::Trait<Box, double>);
  static_assert(Comparable::Trait<Box, Box>);

  // Dyn satisfies the traits it type-erases (1-param):
  static_assert(Shape::Trait<Shape::Dyn>);

  // For 2-param traits, Dyn<T> should also satisfy the trait concept:
  static_assert(Add::Trait<Add::Dyn<int>, int>);
  static_assert(Into::Trait<Into::Dyn<float>, float>);
}
