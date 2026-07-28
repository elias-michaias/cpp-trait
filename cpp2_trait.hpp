#pragma once

#include "trait.hpp"

// cpp2-style ergonomic syntax for trait declarations.
//
// Mirrors the syntax from
//   https://github.com/maksym-pasichnyk/trait/blob/master/demo.cpp
// using `as` keyword + `->` for an ergonomic, Rust-like DSL:
//
//   cpp2(
//       trait Shape as (
//           fn area  as ((Self)        -> int),
//           fn scale as ((Self *, int) -> void)
//       )
//   )
//
// Macro mechanics:
//   * `trait` is object-like and expands to `TRAIT_CPP2_1 (`, where
//     TRAIT_CPP2_1 is a function-like macro that defaults type params
//     to `(Self)`. So `trait Shape as (...)` becomes
//     `TRAIT_CPP2_1 (` `Shape` `, AS_UNPACK(...)` `)`
//           = `TRAIT_CPP2_1(Shape, ...)` -- a 2-arg call.
//   * `as` is object-like `, AS_UNPACK`. `AS_UNPACK(x)` returns `x )`,
//     so the surrounding parens from `as (...)` close the macro call.
//   * `fn` is `(`: a method like `fn area as ((Self) -> int)` becomes
//     `(area, AS_UNPACK ((Self) -> int))` = `(area, (Self) -> int)`.
//     This 2-tuple is the new (Name, FuncSig) format that trait.hpp's
//     arity-dispatch macros now accept.
//   * Multi-param traits use `trait_multi(...)`/`duck_trait_multi(...)`
//     which keep an explicit type params argument.
//
// At COMPILE time trait.hpp carries the template helpers (sig_ret,
// sig_invoke, sig_vp_first_t, sig_first_t, sig_full_ptr_t) that extract
// the return type and parameter types from `auto (P) -> R` so the
// preprocessor never needs to split on `->`.

//----------------------------------------------------------------------
//  DSL keywords: `as`, `fn` are object/function macros that compose the
//  2-tuple method syntax. These are active inside the entire translation
//  unit after `cpp2_trait.hpp` is pulled in. Use CPP2_END (or just leave
//  them defined for the rest of the TU -- they only collide if you also
//  write C++ keyword-style code that contains bare identifiers named
//  `as` or `fn`).
//----------------------------------------------------------------------
#define LPAREN (
#define RPAREN )
#define AS_UNPACK(...) __VA_ARGS__ RPAREN
#define as , AS_UNPACK
#define fn LPAREN

//----------------------------------------------------------------------
//  Save the existing function-like `trait` / `ducktyped_trait` macros
//  and re-export under names that won't collide with the DSL ones.
//----------------------------------------------------------------------
#pragma push_macro("trait")
#pragma push_macro("ducktyped_trait")
#undef trait
#undef ducktyped_trait

#define trait_impl_fn(...)        TRAIT_EXPAND_1(__VA_ARGS__)
#define duck_trait_impl_fn(...)   DUCKTYPED_TRAIT_EXPAND_1(__VA_ARGS__)

// 1-param helpers: the user writes `trait Name as (Methods)` and we
// default the type params tuple to `(Self)` before delegating to the
// real trait machinery. Variadic here is important: each `fn name as (...)`
// produces a `(name, sig)` parenthesized tuple, and the comma *between*
// methods ends up at TRAIT_CPP2_1's call-separator depth, so each method
// arrives as its own variadic arg. Rejoining into a tuple happens here:
#define TRAIT_CPP2_1(Name, ...)        trait_impl_fn(Name, (Self), (__VA_ARGS__))
#define DUCK_TRAIT_CPP2_1(Name, ...)   duck_trait_impl_fn(Name, (Self), (__VA_ARGS__))

// Object-like `trait`/`duck_trait` that start a call to the wrappers.
#define trait        TRAIT_CPP2_1 LPAREN
#define duck_trait   DUCK_TRAIT_CPP2_1 LPAREN

// Multi-param: separate entry points that take the type-params tuple as
// the second arg (no `as`-trick for them since they need 3 args).
#define trait_multi(Name, TP, ...)        trait_impl_fn(Name, TP, (__VA_ARGS__))
#define duck_trait_multi(Name, TP, ...)   duck_trait_impl_fn(Name, TP, (__VA_ARGS__))

//----------------------------------------------------------------------
//  cpp2 scope wrapper (just strips outer parens -- the DSL keywords are
//  global to the TU but you'll typically wrap your declarations inside
//  one for visual grouping).
//----------------------------------------------------------------------
#define cpp2(...) __VA_ARGS__

// Restore prior `trait`/`ducktyped_trait` and drop DSL keywords. Use
// after your cpp2 declarations to keep the rest of the file "normal".
#define CPP2_END                                       \
  _Pragma("pop_macro(\"ducktyped_trait\")")            \
  _Pragma("pop_macro(\"trait\")")                      \
  _Pragma("pop_macro(\"fn\")")                         \
  _Pragma("pop_macro(\"as\")")                         \
  _Pragma("pop_macro(\"AS_UNPACK\")")                  \
  _Pragma("pop_macro(\"RPAREN\")")                     \
  _Pragma("pop_macro(\"LPAREN\")")