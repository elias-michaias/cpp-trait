// clang-format off

// t3.cpp – Strict vs. duck-typed traits
//
// Demonstrates:
//   1. Strict trait rejects impls whose signatures don't match exactly
//      (const/ref param variants, long vs int, etc.)
//   2. Duck-typed trait accepts the same impls via implicit conversions
//   3. Multi-param ducktyped_trait (Into<Self,T>)
//   4. Dyn dynamic dispatch for both 1-param and 2-param traits
//   5. Mixin method generation

#include <cstdio>
#include <type_traits>
#include "../trait.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// 1.  Strict trait: parameter type must match exactly.
//
//     STRICT_TRAIT_REQ4 checks both the call expression AND the
//     function-pointer address:
//
//       { &Impl<T>::name } -> std::same_as< R(*)(Params...) >;
//
//     So a param declared as `const T &` where the trait says `T` fails,
//     even though the call itself would be valid.
// ─────────────────────────────────────────────────────────────────────────────
trait(Transform, (Self), (
  (int, apply, (Self *, int))
))

struct Tile { int val; };

// Exact impl – every type in the signature matches the trait declaration.
template <> struct Transform::Impl<Tile> {
  static int apply(Tile *t, int n) { t->val += n; return t->val; }
};

// "Const" impl – takes `const Tile *` instead of `Tile *`.
// The call still compiles (Tile* → const Tile* is implicit), but the strict
// pointer check fails because the function-pointer types differ.
struct ConstTile { int val; };
template <> struct Transform::Impl<ConstTile> {
  static int apply(const ConstTile *t, int n) { return t->val + n; }
};

// "Wide" impl – takes `long` instead of `int` for the second parameter.
// Again the call compiles (int → long), but the strict check rejects it.
struct WideTile { int val; };
template <> struct Transform::Impl<WideTile> {
  static int apply(WideTile *t, long n) { t->val += (int)n; return t->val; }
};

static_assert( Transform::Trait<Tile>);      // ✓  exact match
static_assert(!Transform::Trait<ConstTile>); // ✗  const T* ≠ T*  (strict)
static_assert(!Transform::Trait<WideTile>);  // ✗  long ≠ int      (strict)

// ─────────────────────────────────────────────────────────────────────────────
// 2.  Duck-typed trait: only the call expression is validated.
//
//     DUCK_TRAIT_REQ4 only checks:
//
//       { Impl<T>::name(declval<Params>()...) } -> std::same_as<R>;
//
//     So `const T *` is acceptable where `T *` is declared (Tile* → const Tile*),
//     and `long` param is acceptable where `int` is declared (int → long).
// ─────────────────────────────────────────────────────────────────────────────
ducktyped_trait(DuckTransform, (Self), (
  (int, apply, (Self *, int))
))

template <> struct DuckTransform::Impl<Tile> {
  static int apply(Tile *t, int n) { t->val += n; return t->val; }
};
template <> struct DuckTransform::Impl<ConstTile> {
  static int apply(const ConstTile *t, int n) { return t->val + n; } // duck: T* → const T*
};
template <> struct DuckTransform::Impl<WideTile> {
  static int apply(WideTile *t, long n) { t->val += (int)n; return t->val; } // duck: int → long
};

static_assert(DuckTransform::Trait<Tile>);      // ✓
static_assert(DuckTransform::Trait<ConstTile>); // ✓  duck: T* → const T* OK
static_assert(DuckTransform::Trait<WideTile>);  // ✓  duck: int → long OK

// ─────────────────────────────────────────────────────────────────────────────
// 3.  Strict multi-param trait: Into<Self, T>
//     The trait says `into(Self)` (by value); an impl that takes `const Self &`
//     satisfies the duck variant but not the strict one.
// ─────────────────────────────────────────────────────────────────────────────
trait(StrictInto, (Self, T), (
  (T, into, (Self))
))

struct Celsius { float v; };
struct Kelvin  { float v; };

// Exact – by value, as declared.
template <> struct StrictInto::Impl<Celsius, float> {
  static float into(Celsius c) { return c.v; }
};

// Const-ref – `const Kelvin &` instead of `Kelvin`.
// Callable check passes (Kelvin rvalue binds to const Kelvin&),
// but the pointer check fails: float(*)(const Kelvin&) ≠ float(*)(Kelvin).
template <> struct StrictInto::Impl<Kelvin, float> {
  static float into(const Kelvin &k) { return k.v; }
};

static_assert( StrictInto::Trait<Celsius, float>); // ✓ exact match
static_assert(!StrictInto::Trait<Kelvin,  float>); // ✗ const ref ≠ by value (strict)

// ─────────────────────────────────────────────────────────────────────────────
// 4.  Duck-typed multi-param trait: same impls are accepted.
// ─────────────────────────────────────────────────────────────────────────────
ducktyped_trait(Into, (Self, T), (
  (T, into, (Self))
))

template <> struct Into::Impl<Celsius, float> {
  static float into(Celsius c) { return c.v; }
};
template <> struct Into::Impl<Kelvin, float> {
  static float into(const Kelvin &k) { return k.v; } // duck: const ref OK
};

static_assert(Into::Trait<Celsius, float>); // ✓
static_assert(Into::Trait<Kelvin,  float>); // ✓ duck: const Kelvin& OK

// Dyn works for both type arguments
static_assert(Into::Trait<Into::Dyn<float>, float>);

// ─────────────────────────────────────────────────────────────────────────────
// 5.  Dyn (type-erased) dispatch for strict Transform
// ─────────────────────────────────────────────────────────────────────────────
static_assert(Transform::Trait<Transform::Dyn>);

// ─────────────────────────────────────────────────────────────────────────────
// 6.  Mixin – method forwarding from the trait namespace
//     struct X : Trait::Mixin lets you call X::method() directly.
// ─────────────────────────────────────────────────────────────────────────────
struct MixinTile : Transform::Mixin { int val; };
template <> struct Transform::Impl<MixinTile> {
  static int apply(MixinTile *t, int n) { t->val += n; return t->val; }
};
static_assert(Transform::Trait<MixinTile>);

// ─────────────────────────────────────────────────────────────────────────────
// main – runtime verification
// ─────────────────────────────────────────────────────────────────────────────
int main() {
  // Strict Transform via Dyn (Self* param → pass &td)
  Tile tile{10};
  Transform::Dyn td = tile;
  printf("%d\n", Transform::apply(&td, 5));  // 15

  // Duck Transform: WideTile (long param) still works at runtime
  WideTile wt{10};
  DuckTransform::apply(&wt, 5);
  printf("%d\n", wt.val);  // 15

  // Into: duck accepts const-ref impl
  Celsius c{100.f};
  Kelvin  k{373.15f};
  printf("%g\n", Into::into<float>(c));      // 100
  printf("%g\n", Into::into<float>(k));      // 373.15

  // Into::Dyn – heterogeneous list of convertibles
  Into::Dyn<float> dyn = c;
  printf("%g\n", Into::into<float>(dyn));    // 100
  dyn = k;
  printf("%g\n", Into::into<float>(dyn));    // 373.15

  // Mixin
  MixinTile mt{};
  mt.val = 0;
  printf("%d\n", mt.apply(7));  // 7
}
