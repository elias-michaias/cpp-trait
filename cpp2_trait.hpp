#pragma once

#include "trait.hpp"

// cpp2-style ergonomic syntax for trait declarations.
//
//   traitdef(
//       dynamic Shape with (
//           fn area  as ((Self)        -> int),
//           fn scale as ((Self *, int) -> void)
//       )
//   )
//
// == Keywords ==
//
//   `traitdef(...)` — scope wrapper (just passes through its body).
//
//   `with` — function-like.  Consumes `(...)` as its own argument list
//   and rewraps: `with(methods...)` → `, (methods...) )`.  The trailing
//   `)` closes whichever `(` came before `with` (e.g. the one from
//   `dynamic` → `TRAIT_CPP2_1 LPAREN`).
//
//   `as` — object-like `, AS_UNPACK`.  `AS_UNPACK(x)` returns `x)`, so
//   `as (...) ` produces `, ... )` — the trailing `)` closes whichever
//   open `(` came before `as`.
//
//     fn area as ((Self) -> int)
//     →  ( area , (Self) -> int )      (fn = LPAREN, as closes it)
//
//   `of` — comma-only, inserts an argument separator inside the
//   enclosing macro call.  Used for multi-param traits:
//
//     dynamic Into of (T) with (fn into as ((Self) -> T))
//     →  Into , (T) , (fn into as (...))   → 3-arg dispatch
//
//   `assoctype Name`              →  (type, Name)          — 2-tuple
//   `assoctemplate Name as (U)`   →  (template, Name, U)   — 3-tuple
//   `fn Name as ((P) -> R)`       →  (Name, Sig)           — 2-tuple (HOF in static traits)
//   `callable((args) -> ret)`     →  (callable, (args) -> ret)  — callable marker inside fn params

//----------------------------------------------------------------------
//  Save the existing function-like macros and re-export.
//----------------------------------------------------------------------
#pragma push_macro("trait")
#pragma push_macro("static_trait")
#pragma push_macro("ducktyped_trait")
#pragma push_macro("static_ducktyped_trait")
#pragma push_macro("with")
#pragma push_macro("assoctemplate")
#pragma push_macro("assoctype")
#pragma push_macro("of")
#pragma push_macro("fn")
#pragma push_macro("callable")
#pragma push_macro("as")
#pragma push_macro("AS_UNPACK")
#pragma push_macro("RPAREN")
#pragma push_macro("LPAREN")
#pragma push_macro("dynamic")
#pragma push_macro("static")
#undef trait
#undef static_trait
#undef ducktyped_trait
#undef static_ducktyped_trait
#undef with
#undef assoctemplate
#undef assoctype
#undef of
#undef fn
#undef callable
#undef as
#undef AS_UNPACK
#undef RPAREN
#undef LPAREN
#undef dynamic
#undef static

//----------------------------------------------------------------------
//  DSL keywords — object-like macros for an ergonomic Rust-like API.
//----------------------------------------------------------------------

// LPAREN/RPAREN are already defined in trait.hpp; define them here so
// the push/undef above doesn't matter.
#define LPAREN (
#define RPAREN )

// `AS_UNPACK(x)` returns `x)`.  `#define as , AS_UNPACK` turns
// `Name as (args)` into `Name , args)`, closing whichever open `(`
// preceded `as`.
#define AS_UNPACK(...) __VA_ARGS__ RPAREN
#define as , AS_UNPACK

// `fn Name as (Sig)` → `(Name, Sig)` (fn = LPAREN opens the tuple).
// In static traits, `fn` produces a higher-order function (HOF) that
// gets a variadic forwarding function; `callable(...)` below marks
// which parameter is the callable (with `-> ret` for its return type).
#define fn LPAREN

// `with(...)` — function-like.  Consumes `(...)` as its own argument
// list and rewraps: `with(methods...)` → `, (methods...) )`.  The
// trailing `)` closes whichever `(` came before `with` (e.g. the one
// from `dynamic` → `TRAIT_CPP2_1 LPAREN`).
#define with(...) , (__VA_ARGS__) RPAREN

// `of` — comma-only, inserts an argument separator in the enclosing
// macro call.  `dynamic Name of (T) with (...)` produces the 3-arg call
// `TRAIT_CPP2_1(Name, (T), (...))`.
#define of ,

// `assoctype Name`  →  (type, Name)  (2-tuple, no `as` needed).
#define assoctype (type,

// `assoctemplate Name as (U)`  →  (template, Name, U)  (3-tuple, single paren).
#define assoctemplate (template,

// `callable((args) -> ret)`  →  (callable, (args) -> ret)
// Marks a callable parameter inside `fn` params, with its return type
// specified via `->`.
#define callable(...) (callable, __VA_ARGS__)

//----------------------------------------------------------------------
//  Trait implementation helpers
//----------------------------------------------------------------------

#define trait_impl_fn(...)              TRAIT_EXPAND_1(__VA_ARGS__)
#define duck_trait_impl_fn(...)         DUCKTYPED_TRAIT_EXPAND_1(__VA_ARGS__)

// HOF-aware static_trait (replicated from trait.hpp line 1580).
#define static_trait_impl_fn(Name, TP, MethodsTuple)                             \
  namespace Name {                                                               \
  template <TYPENAME_LIST(TP)> struct Impl;                                      \
  template <TYPENAME_LIST(TP)>                                                   \
  concept Trait = requires {                                                     \
    FOR_EACH_WITH(STATIC_TRAIT_REQ_ITEM, TP, EXPAND(UNWRAP_I MethodsTuple))       \
  };                                                                             \
  FOR_EACH_WITH(STATIC_TRAIT_FUNC_ITEM, TP, EXPAND(UNWRAP_I MethodsTuple))       \
  }                                                                              \
  _static_trait_register_impl(Name, TP, __COUNTER__, UNWRAP_I MethodsTuple)

#define static_duck_trait_impl_fn(Name, TP, MethodsTuple)                        \
  namespace Name {                                                               \
  template <TYPENAME_LIST(TP)> struct Impl;                                      \
  template <TYPENAME_LIST(TP)>                                                   \
  concept Trait = requires {                                                     \
    FOR_EACH_WITH(STATIC_TRAIT_REQ_ITEM, TP, EXPAND(UNWRAP_I MethodsTuple))       \
  };                                                                             \
  FOR_EACH_WITH(STATIC_TRAIT_FUNC_ITEM, TP, EXPAND(UNWRAP_I MethodsTuple))       \
  }                                                                              \
  _static_trait_register_impl(Name, TP, __COUNTER__, UNWRAP_I MethodsTuple)

//----------------------------------------------------------------------
//  Dispatch: use VA_COUNT to distinguish arity.
//  `with` produces Name , (methods)                          → 2 args
//  `of` produces  Name , (TP) , (methods)                    → 3 args
//  No IS_PAREN needed — arity alone tells us which case.
//----------------------------------------------------------------------
#define TRAIT_CPP2_1(...) \
    CAT(TRAIT_CPP2_IMPL_, VA_COUNT(__VA_ARGS__))(__VA_ARGS__)
// `Methods` comes from `with(...)` expansion which already wraps in parens:
//   with(m1, m2, m3)  →  , (m1, m2, m3) )
// So `Methods` = `(m1, m2, m3)` — pass directly without extra wrapping.
#define TRAIT_CPP2_IMPL_2(Name, Methods) \
    trait_impl_fn(Name, (Self), Methods)
#define TRAIT_CPP2_IMPL_3(Name, TP, Methods) \
    trait_impl_fn(Name, (Self, UNWRAP_I TP), Methods)

#define DUCK_TRAIT_CPP2_1(...) \
    CAT(DUCK_TRAIT_CPP2_IMPL_, VA_COUNT(__VA_ARGS__))(__VA_ARGS__)
#define DUCK_TRAIT_CPP2_IMPL_2(Name, Methods) \
    duck_trait_impl_fn(Name, (Self), Methods)
#define DUCK_TRAIT_CPP2_IMPL_3(Name, TP, Methods) \
    duck_trait_impl_fn(Name, (Self, UNWRAP_I TP), Methods)

#define STATIC_TRAIT_CPP2_1(...) \
    CAT(STATIC_TRAIT_CPP2_IMPL_, VA_COUNT(__VA_ARGS__))(__VA_ARGS__)
#define STATIC_TRAIT_CPP2_IMPL_2(Name, Methods) \
    static_trait_impl_fn(Name, (Self), Methods)
#define STATIC_TRAIT_CPP2_IMPL_3(Name, TP, Methods) \
    static_trait_impl_fn(Name, (Self, UNWRAP_I TP), Methods)

#define STATIC_DUCK_TRAIT_CPP2_1(...) \
    CAT(STATIC_DUCK_TRAIT_CPP2_IMPL_, VA_COUNT(__VA_ARGS__))(__VA_ARGS__)
#define STATIC_DUCK_TRAIT_CPP2_IMPL_2(Name, Methods) \
    static_duck_trait_impl_fn(Name, (Self), Methods)
#define STATIC_DUCK_TRAIT_CPP2_IMPL_3(Name, TP, Methods) \
    static_duck_trait_impl_fn(Name, (Self, UNWRAP_I TP), Methods)

// Object-like entry points.
#define dynamic           TRAIT_CPP2_1 LPAREN
#define duck_trait        DUCK_TRAIT_CPP2_1 LPAREN
#define static_trait      STATIC_TRAIT_CPP2_1 LPAREN
#define static_duck_trait STATIC_DUCK_TRAIT_CPP2_1 LPAREN

//----------------------------------------------------------------------
//  cpp2 tuple formats and dispatch overrides for static traits.
//
//  Our cpp2 DSL produces these method tuples from the keywords above:
//    assoctype Name              →  (type, Name)              — 2-tuple
//    assoctemplate Name as (U)   →  (template, Name, U)        — 3-tuple
//    fn Name as ((P) -> R)       →  (Name, Sig)                — 2-tuple (HOF)
//
//  The original trait.hpp dispatch in STATIC_TRAIT_REQ/FUNC_ITEM_2
//  assumes 2-tuples are `(template, Name)` or `(type, Name)` where
//  Kind is a known keyword.  In our DSL, `fn` also produces a 2-tuple
//  but with Kind = the function name (e.g. `map`).  We must distinguish
//  assoctype (Kind = `type`) from fn HOF (Kind = function name).
//
//  IS_TYPE(x) returns 1 when x is the token `type`, 0 otherwise.
//----------------------------------------------------------------------
#define IS_TYPE(x) CHECK(CAT(IS_TYPE_, x))
#define IS_TYPE_type PROBE(~)

// ---------- STATIC_TRAIT_REQ_ITEM_2 (concept requirements) ----------
//    assoctype (Kind=type)  → typename Impl<>::Name;
//    fn HOF    (Kind=name)  → (no concept requirement)

#undef STATIC_TRAIT_REQ_ITEM_2
#define STATIC_TRAIT_REQ_ITEM_2(TP, Kind, Name) \
    CAT(STATIC_TRAIT_REQ_ITEM_2_CPP2_, IS_TYPE(Kind))(TP, Kind, Name)

#define STATIC_TRAIT_REQ_ITEM_2_CPP2_1(TP, Kind, Name) \
    typename Impl<ALL_ARGS(TP)>::Name;

#define STATIC_TRAIT_REQ_ITEM_2_CPP2_0(TP, Kind, Name) \
    /* fn HOF: no concept requirement (forwarding function validates) */

// ---------- STATIC_TRAIT_FUNC_ITEM_2 (code generation) ----------
//    assoctype (Kind=type)  → (no function)
//    fn HOF    (Kind=name) → variadic forwarding function

#undef STATIC_TRAIT_FUNC_ITEM_2
#define STATIC_TRAIT_FUNC_ITEM_2(TP, Kind, Name) \
    CAT(STATIC_TRAIT_FUNC_ITEM_2_CPP2_, IS_TYPE(Kind))(TP, Kind, Name)

#define STATIC_TRAIT_FUNC_ITEM_2_CPP2_1(TP, Kind, Name) \
    /* assoctype: no code */

// Variadic forwarding for HOF.  The self type is taken from FIRST(TP)
// rather than parsed from the Sig (which contains `->` that the
// preprocessor cannot split).  Variadic Args&&... handles any number
// of parameters:
//   map(a1, F&& fn)         — 1 self param
//   zip_with(a1, a2, F&&)   — 2 self params
#define STATIC_TRAIT_FUNC_ITEM_2_CPP2_0(TP, Kind, Name) \
    template <TYPENAME_LIST(TP), typename... Args>                             \
    decltype(auto) Kind(TYPE_SPEC(FIRST(TP)) a1, Args&&... args) {            \
        return Impl<ALL_ARGS(TP)>::Kind(a1, ::std::forward<Args>(args)...);   \
    }

// ---------- STATIC_TRAIT_REQ_ITEM_3 (concept requires) ----------
//  assoctemplate (Kind=template) → typename Impl<>::template Name<Params>;
//  (We use Params rather than `void` so the alias is instantiated
//   with the user-specified template argument, e.g. Mapped<U>.)

#undef STATIC_TRAIT_REQ_ITEM_3_KIND_1
#define STATIC_TRAIT_REQ_ITEM_3_KIND_1(TP, Kind, Name, Params) \
    typename Impl<ALL_ARGS(TP)>::template Name<Params>;

// ---------- STATIC_LAYER_METHOD4_2 (layer-chain dot syntax) ----------
//    assoctype (Kind=type)  → skip
//    fn HOF    (Kind=name)  → forwarding layer method

#undef STATIC_LAYER_METHOD4_2
#define STATIC_LAYER_METHOD4_2(NS, TP, N, Kind, Name) \
    CAT(STATIC_LAYER_METHOD4_2_CPP2_, IS_TYPE(Kind))(NS, TP, N, Name)

#define STATIC_LAYER_METHOD4_2_CPP2_1(NS, TP, N, Name) \
    /* assoctype: not a method */

#define STATIC_LAYER_METHOD4_2_CPP2_0(NS, TP, N, Name) \
    template <typename... _HofArgs> \
    auto Name(this auto &&self, _HofArgs&&... _hof_args) { \
        return ::NS::Name(self, std::forward<_HofArgs>(_hof_args)...); \
    }

//----------------------------------------------------------------------
//  traitdef scope wrapper
//----------------------------------------------------------------------
#define traitdef(...) __VA_ARGS__

// Restore prior macros and drop DSL keywords.
#define CPP2_END                                       \
  _Pragma("pop_macro(\"trait\")")                      \
  _Pragma("pop_macro(\"static_trait\")")               \
  _Pragma("pop_macro(\"ducktyped_trait\")")            \
  _Pragma("pop_macro(\"static_ducktyped_trait\")")     \
  _Pragma("pop_macro(\"with\")")                       \
  _Pragma("pop_macro(\"assoctemplate\")")              \
  _Pragma("pop_macro(\"assoctype\")")                  \
  _Pragma("pop_macro(\"of\")")                         \
  _Pragma("pop_macro(\"fn\")")                         \
  _Pragma("pop_macro(\"callable\")")                   \
  _Pragma("pop_macro(\"as\")")                         \
  _Pragma("pop_macro(\"AS_UNPACK\")")                  \
  _Pragma("pop_macro(\"RPAREN\")")                     \
  _Pragma("pop_macro(\"LPAREN\")")                     \
  _Pragma("pop_macro(\"dynamic\")")                   \
  _Pragma("pop_macro(\"static\")")                     \
  static_assert(true, "") /* swallow trailing semicolon */