// clang-format off

// t4.cpp – Static traits: associated types, associated templates, and HOF type safety
//
// Demonstrates:
//   1. static_trait with associated type (Iterator::Item)
//      - concept enforces presence of the associated type
//      - missing associated type fails the concept
//   2. static_trait with associated template (Functor::Mapped<U>)
//      - concept enforces the template alias exists
//   3. Higher-order function (map) that accepts any well-typed callable
//      - correct callables: return type and argument type are verified
//      - wrong arg type rejected via Impl::map (SFINAE in return type signature)
//      - non-callable rejected via Impl::map
//   4. HOF with fixed return type (fold)
//      - wrong arg types rejected via Impl::fold (requires clause in signature)
//      - wrong arity rejected
//   5. HOF with multi-arg function (zip_with)
//      - wrong arg type rejected
//      - wrong arity rejected
//   6. static_ducktyped_trait: duck-typing for static traits
//      - const-ref param accepted where value is declared (same as non-static duck)
//
// NOTE ON HOF REJECTION TESTING:
//   The macro-generated free function wrappers (e.g. Functor::map) use
//   `decltype(auto)` return types. Errors in the function BODY during return-type
//   deduction are hard errors, not substitution failures – so they cannot be
//   safely captured by a `requires { ... }` expression. Instead, rejection tests
//   go through Impl::method directly, where SFINAE lives in the function signature
//   (immediate context), and requires-expressions work correctly.

#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include "../trait.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Helper containers
// ─────────────────────────────────────────────────────────────────────────────
template <typename T> struct Box   { T value; };
template <typename T> struct Maybe { bool has; T value; };

// ─────────────────────────────────────────────────────────────────────────────
// 1.  Associated type: static_trait(Iterator, (Self), ((type, Item)))
//
//     The concept only enforces that Impl<Self>::Item exists.
//     An impl that omits the typedef fails the concept.
// ─────────────────────────────────────────────────────────────────────────────
static_trait(Iterator, (Self), (
  (type, Item)
))

struct CounterIter { int cur; };
template <> struct Iterator::Impl<CounterIter> {
  using Item = int;                      // associated type present
  static int next(CounterIter it) { return it.cur; }
};

struct BadIter { int x; };
template <> struct Iterator::Impl<BadIter> {
  // NOTE: no `using Item = ...;` — concept fails
  static int next(BadIter) { return 0; }
};

static_assert( Iterator::Trait<CounterIter>); // ✓ has Item
static_assert(!Iterator::Trait<BadIter>);      // ✗ missing Item typedef

// ─────────────────────────────────────────────────────────────────────────────
// 2.  Associated template + HOF map:
//
//     Functor<Self> requires:
//       - associated type     value_type
//       - associated template Mapped<U>
//       - hof map(Self, fn: value_type → U) → Mapped<U>
//
//     The concept validates value_type and Mapped.
//     HOF rejection goes through Impl::map directly (SFINAE in return type).
// ─────────────────────────────────────────────────────────────────────────────
static_trait(Functor, (Self), (
  (type, value_type),
  (template, Mapped, (U)),
  hof((template, Mapped, (U)), map, (Self, fn(U, value_type)))
))

// Box<T> impl
template <typename T> struct Functor::Impl<Box<T>> {
  using value_type = T;
  template <typename U> using Mapped = Box<U>;

  // SFINAE via return type: ill-formed when F is not callable with T.
  // This makes Impl::map a reliable target for requires-expression rejection tests.
  template <class F>
  static auto map(Box<T> b, F&& fn)
      -> Mapped<std::remove_cvref_t<std::invoke_result_t<F&, T>>> {
    return { std::invoke(std::forward<F>(fn), b.value) };
  }
};

// Maybe<T> impl
template <typename T> struct Functor::Impl<Maybe<T>> {
  using value_type = T;
  template <typename U> using Mapped = Maybe<U>;

  template <class F>
  static auto map(Maybe<T> m, F&& fn)
      -> Mapped<std::remove_cvref_t<std::invoke_result_t<F&, T>>> {
    if (!m.has) return { false, {} };
    return { true, std::invoke(std::forward<F>(fn), m.value) };
  }
};

// Type with no value_type → concept fails
struct NotAFunctor {};
template <> struct Functor::Impl<NotAFunctor> { /* no value_type, no Mapped */ };

static_assert( Functor::Trait<Box<int>>);         // ✓
static_assert( Functor::Trait<Box<std::string>>); // ✓
static_assert( Functor::Trait<Maybe<int>>);        // ✓
static_assert(!Functor::Trait<NotAFunctor>);       // ✗ missing value_type + Mapped

// Return-type verification for well-typed callables
static_assert(std::same_as<
  decltype(Functor::map(Box<int>{1}, [](int x) { return x + 1; })),
  Box<int>>);
static_assert(std::same_as<
  decltype(Functor::map(Box<int>{1}, [](int x) { return std::to_string(x); })),
  Box<std::string>>);
static_assert(std::same_as<
  decltype(Functor::map(Maybe<int>{true, 3}, [](int x) { return x * 2; })),
  Maybe<int>>);

// ── HOF rejection for map (via Impl::map inside a concept – proper SFINAE) ───
// A concept wrapper places F in a template parameter context so substitution
// failure in invoke_result_t<F&, int> evaluates to false rather than hard error.
struct TakesString { std::size_t operator()(std::string s) const { return s.size(); } };

template <typename F>
concept BoxIntMappable = requires(Box<int> b, F f) {
  Functor::Impl<Box<int>>::map(b, f);
};

static_assert( BoxIntMappable<decltype([](int x) { return x + 1; })>); // ✓ int → int
static_assert(!BoxIntMappable<TakesString>);                            // ✗ expects string
static_assert(!BoxIntMappable<int>);                                    // ✗ not callable

// ─────────────────────────────────────────────────────────────────────────────
// 3.  HOF with fixed return type: fold
//
//     Foldable<Self> requires:
//       - associated type value_type
//       - hof fold(Self, int init, fn: (int, value_type) → int) → int
//
//     The `requires` clause on Impl::fold puts the constraint in the
//     immediate context, so it SFINAE-propagates into requires-expressions.
// ─────────────────────────────────────────────────────────────────────────────
static_trait(Foldable, (Self), (
  (type, value_type),
  hof(int, fold, (Self, int, fn(int, int, value_type)))
))

template <typename T> struct Foldable::Impl<Box<T>> {
  using value_type = T;

  // `requires` clause in signature = immediate context = SFINAE-friendly.
  template <class F>
      requires std::invocable<F&, int, T>
  static auto fold(Box<T> b, int init, F&& fn)
      -> std::invoke_result_t<F&, int, T> {
    return std::invoke(std::forward<F>(fn), init, b.value);
  }
};

static_assert(Foldable::Trait<Box<int>>);

// Well-typed fold
static_assert(std::same_as<
  decltype(Foldable::fold(Box<int>{5}, 0, [](int acc, int v) { return acc + v; })),
  int>);

// ── HOF rejection for fold (via concept wrapper – requires clause in signature) ─
struct WrongFoldFn { int operator()(int, std::string) const { return 0; } };
struct UnaryFoldFn { int operator()(int x) const { return x; } };

template <typename F>
concept BoxIntFoldable = requires(Box<int> b, F f) {
  Foldable::Impl<Box<int>>::fold(b, 0, f);
};

static_assert( BoxIntFoldable<decltype([](int acc, int v) { return acc + v; })>); // ✓
// Wrong arg types (string where int expected) → rejected.
static_assert(!BoxIntFoldable<WrongFoldFn>);   // ✗ takes (int, string)
// Wrong arity (1 arg instead of 2) → rejected.
static_assert(!BoxIntFoldable<UnaryFoldFn>);   // ✗ unary
// Non-callable → rejected.
static_assert(!BoxIntFoldable<const char*>);   // ✗ not a function

// ─────────────────────────────────────────────────────────────────────────────
// 4.  Multi-arg HOF: zip_with
//
//     Zip<Self> requires:
//       - associated type     value_type
//       - associated template Mapped<U>
//       - hof zip_with(Self, Self, fn: (value_type, value_type) → U) → Mapped<U>
//
//     Tests both wrong-arity and wrong-type rejection.
// ─────────────────────────────────────────────────────────────────────────────
static_trait(Zip, (Self), (
  (type, value_type),
  (template, Mapped, (U)),
  hof((template, Mapped, (U)), zip_with, (Self, Self, fn(U, value_type, value_type)))
))

template <typename T> struct Zip::Impl<Box<T>> {
  using value_type = T;
  template <typename U> using Mapped = Box<U>;

  template <class F>
  static auto zip_with(Box<T> a, Box<T> b, F&& fn)
      -> Mapped<std::remove_cvref_t<std::invoke_result_t<F&, T, T>>> {
    return { std::invoke(std::forward<F>(fn), a.value, b.value) };
  }
};

static_assert(Zip::Trait<Box<int>>);

// Well-typed zip
static_assert(std::same_as<
  decltype(Zip::zip_with(Box<int>{2}, Box<int>{3},
                         [](int a, int b) { return a + b; })),
  Box<int>>);
static_assert(std::same_as<
  decltype(Zip::zip_with(Box<int>{2}, Box<int>{3},
                         [](int a, int b) { return a > b; })),
  Box<bool>>);

// ── HOF rejection for zip_with (via concept wrapper) ─────────────────────────
struct UnaryZipFn { int operator()(int x) const { return x; } };
struct StringZipFn { std::string operator()(std::string a, std::string b) const { return a + b; } };

template <typename F>
concept BoxIntZippable = requires(Box<int> a, Box<int> b, F f) {
  Zip::Impl<Box<int>>::zip_with(a, b, f);
};

// Wrong arity (1 arg instead of 2) → rejected.
static_assert(!BoxIntZippable<UnaryZipFn>);    // ✗ unary
// Wrong arg type (string instead of int) → rejected.
static_assert(!BoxIntZippable<StringZipFn>);   // ✗ takes strings

// ─────────────────────────────────────────────────────────────────────────────
// 5.  static_ducktyped_trait: duck-typing for static traits
//
//     Like ducktyped_trait but static (no Dyn/vtable).
//     Duck check: only validates call expression, not exact function-pointer type.
//     So `const int&` param is accepted where plain `int` is declared,
//     because calling with `declval<int>()` binds to `const int&`.
// ─────────────────────────────────────────────────────────────────────────────
static_ducktyped_trait(Remap, (Self), (
  (int, remap, (Self, int))
))

struct Stretcher { int factor; };

// Exact impl
template <> struct Remap::Impl<Stretcher> {
  static int remap(Stretcher s, int v) { return s.factor * v; }
};

// Const-ref impl: takes `const int&` instead of `int`
struct ConstStretcher { int factor; };
template <> struct Remap::Impl<ConstStretcher> {
  static int remap(ConstStretcher s, const int& v) { return s.factor * v; } // duck: const ref OK
};

static_assert(Remap::Trait<Stretcher>);      // ✓ exact match
static_assert(Remap::Trait<ConstStretcher>); // ✓ duck: const int& ← int rvalue

// ─────────────────────────────────────────────────────────────────────────────
// main – runtime verification
// ─────────────────────────────────────────────────────────────────────────────
int main() {
  // Functor::map over Box
  Box<int> b{5};
  auto b2 = Functor::map(b, [](int x) { return x * 2; });
  std::cout << b2.value << "\n";  // 10

  auto b3 = Functor::map(b, [](int x) { return std::to_string(x); });
  std::cout << b3.value << "\n";  // 5

  // Functor::map over Maybe (present)
  Maybe<int> m{true, 7};
  auto m2 = Functor::map(m, [](int x) { return x + 1; });
  std::cout << m2.has << " " << m2.value << "\n";  // 1 8

  // Functor::map over Maybe (absent – maps to empty)
  Maybe<int> empty{false, 0};
  auto m3 = Functor::map(empty, [](int x) { return x + 1; });
  std::cout << m3.has << "\n";  // 0

  // Foldable::fold
  Box<int> bf{10};
  int sum = Foldable::fold(bf, 100, [](int acc, int v) { return acc + v; });
  std::cout << sum << "\n";  // 110

  // Zip::zip_with
  auto z = Zip::zip_with(Box<int>{3}, Box<int>{4}, [](int a, int b) { return a * b; });
  std::cout << z.value << "\n";  // 12

  // Duck static: const-ref impl still works at runtime
  ConstStretcher cs{3};
  std::cout << Remap::remap(cs, 7) << "\n";  // 21
}

