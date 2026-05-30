// gen_interface.h
#ifndef TRAIT_GENERATION_H
#define TRAIT_GENERATION_H

#include <concepts>
#include <type_traits>
#include <utility>

namespace gen_interface_detail {

template <class Receiver, class T>
constexpr decltype(auto) receiver_from(void *p) {
  if constexpr (std::is_pointer_v<Receiver>)
    return static_cast<Receiver>(p);
  else
    return *static_cast<std::remove_reference_t<Receiver> *>(p);
}

} // namespace gen_interface_detail

//--------------------------------------------------------------------
//  FOR_EACH / FOR_EACH_WITH
//--------------------------------------------------------------------
#define PARENS ()
#define EXPAND(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) __VA_ARGS__

#define FOR_EACH(macro, ...)                                                   \
  __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...)                                        \
  macro(a1) __VA_OPT__(FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define FOR_EACH_WITH(macro, data, ...)                                        \
  __VA_OPT__(EXPAND(FEWH(macro, data, __VA_ARGS__)))
#define FEWH(macro, data, a1, ...)                                             \
  macro(data, a1) __VA_OPT__(FEWA PARENS(macro, data, __VA_ARGS__))
#define FEWA() FEWH

//--------------------------------------------------------------------
//  Arity / unwrap
//--------------------------------------------------------------------
#define VA_COUNT(...) VA_COUNT_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)
#define VA_COUNT_IMPL(_1, _2, _3, _4, _5, N, ...) N
#define UNWRAP_I(...) __VA_ARGS__
#define UNWRAP(x) UNWRAP_I x

#define FIRST_1(A) A
#define FIRST_2(A, ...) A
#define FIRST_3(A, ...) A
#define FIRST(P) FIRST_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define FIRST_I(N, ...) FIRST_II(N, __VA_ARGS__)
#define FIRST_II(N, ...) FIRST_##N(__VA_ARGS__)

//--------------------------------------------------------------------
//  Parameter helpers (Params = (Self, extras...))
//--------------------------------------------------------------------
#define TUPLE_TO_DECLVALS_1(T1) std::declval<T1>()
#define TUPLE_TO_DECLVALS_2(T1, T2) std::declval<T1>(), std::declval<T2>()
#define TUPLE_TO_DECLVALS_3(T1, T2, T3)                                        \
  std::declval<T1>(), std::declval<T2>(), std::declval<T3>()
#define TUPLE_TO_DECLVALS_4(T1, T2, T3, T4)                                    \
  std::declval<T1>(), std::declval<T2>(), std::declval<T3>(), std::declval<T4>()
#define TUPLE_TO_DECLVALS_5(T1, T2, T3, T4, T5)                                \
  std::declval<T1>(), std::declval<T2>(), std::declval<T3>(),                  \
      std::declval<T4>(), std::declval<T5>()
#define TUPLE_TO_DECLVALS(P) TUPLE_TO_DECLVALS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define TUPLE_TO_DECLVALS_I(N, ...) TUPLE_TO_DECLVALS_II(N, __VA_ARGS__)
#define TUPLE_TO_DECLVALS_II(N, ...) TUPLE_TO_DECLVALS_##N(__VA_ARGS__)

#define FUNC_PARAMS_1(S) S self
#define FUNC_PARAMS_2(S, T1) S self, T1 p1
#define FUNC_PARAMS_3(S, T1, T2) S self, T1 p1, T2 p2
#define FUNC_PARAMS_4(S, T1, T2, T3) S self, T1 p1, T2 p2, T3 p3
#define FUNC_PARAMS_5(S, T1, T2, T3, T4) S self, T1 p1, T2 p2, T3 p3, T4 p4
#define FUNC_PARAMS(P) FUNC_PARAMS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define FUNC_PARAMS_I(N, ...) FUNC_PARAMS_II(N, __VA_ARGS__)
#define FUNC_PARAMS_II(N, ...) FUNC_PARAMS_##N(__VA_ARGS__)

#define CALL_ARGS_1(S) self
#define CALL_ARGS_2(S, T1) self, p1
#define CALL_ARGS_3(S, T1, T2) self, p1, p2
#define CALL_ARGS_4(S, T1, T2, T3) self, p1, p2, p3
#define CALL_ARGS_5(S, T1, T2, T3, T4) self, p1, p2, p3, p4
#define CALL_ARGS(P) CALL_ARGS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define CALL_ARGS_I(N, ...) CALL_ARGS_II(N, __VA_ARGS__)
#define CALL_ARGS_II(N, ...) CALL_ARGS_##N(__VA_ARGS__)

#define CALL_EXTRA_ARGS_1(S)
#define CALL_EXTRA_ARGS_2(S, T1) , p1
#define CALL_EXTRA_ARGS_3(S, T1, T2) , p1, p2
#define CALL_EXTRA_ARGS_4(S, T1, T2, T3) , p1, p2, p3
#define CALL_EXTRA_ARGS_5(S, T1, T2, T3, T4) , p1, p2, p3, p4
#define CALL_EXTRA_ARGS(P) CALL_EXTRA_ARGS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define CALL_EXTRA_ARGS_I(N, ...) CALL_EXTRA_ARGS_II(N, __VA_ARGS__)
#define CALL_EXTRA_ARGS_II(N, ...) CALL_EXTRA_ARGS_##N(__VA_ARGS__)

#define VTABLE_EXTRA_PARAMS_1(S)
#define VTABLE_EXTRA_PARAMS_2(S, T1) , T1
#define VTABLE_EXTRA_PARAMS_3(S, T1, T2) , T1, T2
#define VTABLE_EXTRA_PARAMS_4(S, T1, T2, T3) , T1, T2, T3
#define VTABLE_EXTRA_PARAMS_5(S, T1, T2, T3, T4) , T1, T2, T3, T4
#define VTABLE_EXTRA_PARAMS(P)                                                 \
  VTABLE_EXTRA_PARAMS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define VTABLE_EXTRA_PARAMS_I(N, ...) VTABLE_EXTRA_PARAMS_II(N, __VA_ARGS__)
#define VTABLE_EXTRA_PARAMS_II(N, ...) VTABLE_EXTRA_PARAMS_##N(__VA_ARGS__)

#define VT_LAMBDA_EXTRA_PARAMS_1(S)
#define VT_LAMBDA_EXTRA_PARAMS_2(S, T1) , T1 p1
#define VT_LAMBDA_EXTRA_PARAMS_3(S, T1, T2) , T1 p1, T2 p2
#define VT_LAMBDA_EXTRA_PARAMS_4(S, T1, T2, T3) , T1 p1, T2 p2, T3 p3
#define VT_LAMBDA_EXTRA_PARAMS_5(S, T1, T2, T3, T4) , T1 p1, T2 p2, T3 p3, T4 p4
#define VT_LAMBDA_EXTRA_PARAMS(P)                                              \
  VT_LAMBDA_EXTRA_PARAMS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define VT_LAMBDA_EXTRA_PARAMS_I(N, ...)                                       \
  VT_LAMBDA_EXTRA_PARAMS_II(N, __VA_ARGS__)
#define VT_LAMBDA_EXTRA_PARAMS_II(N, ...)                                      \
  VT_LAMBDA_EXTRA_PARAMS_##N(__VA_ARGS__)

//--------------------------------------------------------------------
//  Type param helpers
//--------------------------------------------------------------------
#define TYPENAME_LIST(TP) TYPENAME_LIST_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define TYPENAME_LIST_I(N, ...) TYPENAME_LIST_II(N, __VA_ARGS__)
#define TYPENAME_LIST_II(N, ...) TYPENAME_LIST_##N(__VA_ARGS__)
#define TYPENAME_LIST_1(A) typename A
#define TYPENAME_LIST_2(A, B) typename A, typename B
#define TYPENAME_LIST_3(A, B, C) typename A, typename B, typename C

#define TEMPLATE_DECL(TP) TEMPLATE_DECL_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define TEMPLATE_DECL_I(N, ...) TEMPLATE_DECL_II(N, __VA_ARGS__)
#define TEMPLATE_DECL_II(N, ...) TEMPLATE_DECL_##N(__VA_ARGS__)
#define TEMPLATE_DECL_1(A)
#define TEMPLATE_DECL_2(A, B) template <typename B>
#define TEMPLATE_DECL_3(A, B, C) template <typename B, typename C>

#define IMPL_SPEC_HEAD(TP) IMPL_SPEC_HEAD_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define IMPL_SPEC_HEAD_I(N, ...) IMPL_SPEC_HEAD_II(N, __VA_ARGS__)
#define IMPL_SPEC_HEAD_II(N, ...) IMPL_SPEC_HEAD_##N(__VA_ARGS__)
#define IMPL_SPEC_HEAD_1(A) template <>
#define IMPL_SPEC_HEAD_2(A, B)
#define IMPL_SPEC_HEAD_3(A, B, C)

#define ANGLE_EXTRA_ARGS(TP)                                                   \
  ANGLE_EXTRA_ARGS_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define ANGLE_EXTRA_ARGS_I(N, ...) ANGLE_EXTRA_ARGS_II(N, __VA_ARGS__)
#define ANGLE_EXTRA_ARGS_II(N, ...) ANGLE_EXTRA_ARGS_##N(__VA_ARGS__)
#define ANGLE_EXTRA_ARGS_1(A)
#define ANGLE_EXTRA_ARGS_2(A, B) <B>
#define ANGLE_EXTRA_ARGS_3(A, B, C) <B, C>

#define ALL_ARGS(TP) ALL_ARGS_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define ALL_ARGS_I(N, ...) ALL_ARGS_II(N, __VA_ARGS__)
#define ALL_ARGS_II(N, ...) ALL_ARGS_##N(__VA_ARGS__)
#define ALL_ARGS_1(A) A
#define ALL_ARGS_2(A, B) A, B
#define ALL_ARGS_3(A, B, C) A, B, C

#define TAIL_ARGS(TP) TAIL_ARGS_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define TAIL_ARGS_I(N, ...) TAIL_ARGS_II(N, __VA_ARGS__)
#define TAIL_ARGS_II(N, ...) TAIL_ARGS_##N(__VA_ARGS__)
#define TAIL_ARGS_1(A)
#define TAIL_ARGS_2(A, B) B
#define TAIL_ARGS_3(A, B, C) B, C

#define COMMA_TAIL(TP) COMMA_TAIL_I(VA_COUNT(UNWRAP(TP)), TP)
#define COMMA_TAIL_I(N, TP) COMMA_TAIL_II(N, TP)
#define COMMA_TAIL_II(N, TP) COMMA_TAIL_##N(TP)
#define COMMA_TAIL_1(TP)
#define COMMA_TAIL_2(TP) , TAIL_ARGS(TP)
#define COMMA_TAIL_3(TP) , TAIL_ARGS(TP)

#define DYN_IMPL_SPEC_ARGS(TP) Dyn ANGLE_EXTRA_ARGS(TP) COMMA_TAIL(TP)

#define FUNC_TEMPLATE_HEAD(TP)                                                 \
  FUNC_TEMPLATE_HEAD_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define FUNC_TEMPLATE_HEAD_I(N, ...) FUNC_TEMPLATE_HEAD_II(N, __VA_ARGS__)
#define FUNC_TEMPLATE_HEAD_II(N, ...) FUNC_TEMPLATE_HEAD_##N(__VA_ARGS__)
#define FUNC_TEMPLATE_HEAD_1(A) template <Trait A>
#define FUNC_TEMPLATE_HEAD_2(A, B) template <typename B, Trait<B> A>
#define FUNC_TEMPLATE_HEAD_3(A, B, C)                                          \
  template <typename B, typename C, Trait<B, C> A>

#define DYN_CTOR_CONSTRAINT(TP)                                                \
  DYN_CTOR_CONSTRAINT_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define DYN_CTOR_CONSTRAINT_I(N, ...) DYN_CTOR_CONSTRAINT_II(N, __VA_ARGS__)
#define DYN_CTOR_CONSTRAINT_II(N, ...) DYN_CTOR_CONSTRAINT_##N(__VA_ARGS__)
#define DYN_CTOR_CONSTRAINT_1(A)                                               \
  template <typename A>                                                        \
    requires Trait<A> && (!std::same_as<std::remove_cvref_t<A>, Dyn>)
#define DYN_CTOR_CONSTRAINT_2(A, B)                                            \
  template <typename A>                                                        \
    requires Trait<A, B> && (!std::same_as<std::remove_cvref_t<A>, Dyn<B>>)
#define DYN_CTOR_CONSTRAINT_3(A, B, C)                                         \
  template <typename A>                                                        \
    requires Trait<A, B, C> &&                                                 \
             (!std::same_as<std::remove_cvref_t<A>, Dyn<B, C>>)

//--------------------------------------------------------------------
//  Duck‑typed operation macros (multiple overloads for Dyn)
//--------------------------------------------------------------------
#define DUCK_TRAIT_REQ4_TUPLE(TP, M) DUCK_TRAIT_REQ4_APPLY(TP, UNWRAP(M))
#define DUCK_TRAIT_REQ4_APPLY(TP, ...) DUCK_TRAIT_REQ4(TP, __VA_ARGS__)
#define DUCK_TRAIT_REQ4(TP, Ret, Name, Params)                                 \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}->std::same_as<Ret>;

#define FREE_FUNC4_TUPLE(TP, M) FREE_FUNC4_APPLY(TP, UNWRAP(M))
#define FREE_FUNC4_APPLY(TP, ...) FREE_FUNC4(TP, __VA_ARGS__)
#define FREE_FUNC4(TP, Ret, Name, Params)                                      \
  FUNC_TEMPLATE_HEAD(TP) Ret Name(FUNC_PARAMS(Params)) {                       \
    return Impl<ALL_ARGS(TP)>::Name(CALL_ARGS(Params));                        \
  }

#define VTABLE_MEMBER4_TUPLE(TP, M) VTABLE_MEMBER4_APPLY(TP, UNWRAP(M))
#define VTABLE_MEMBER4_APPLY(TP, ...) VTABLE_MEMBER4(TP, __VA_ARGS__)
#define VTABLE_MEMBER4(TP, Ret, Name, Params)                                  \
  Ret (*Name)(void *VTABLE_EXTRA_PARAMS(Params));

#define VT_ENTRY4_TUPLE(TP, M) VT_ENTRY4_APPLY(TP, UNWRAP(M))
#define VT_ENTRY4_APPLY(TP, ...) VT_ENTRY4(TP, __VA_ARGS__)
#define VT_ENTRY4(TP, Ret, Name, Params)                                       \
  .Name = [](void *p VT_LAMBDA_EXTRA_PARAMS(Params)) -> Ret {                  \
    using Receiver = FIRST(Params);                                            \
    return Impl<ALL_ARGS(TP)>::Name(                                           \
        ::gen_interface_detail::receiver_from<Receiver, FIRST(TP)>(p)          \
            CALL_EXTRA_ARGS(Params));                                          \
  },

#define IMPL_DYN_METHOD4_TUPLE(TP, M) IMPL_DYN_METHOD4_APPLY(TP, UNWRAP(M))
#define IMPL_DYN_METHOD4_APPLY(TP, ...) IMPL_DYN_METHOD4(TP, __VA_ARGS__)
#define IMPL_DYN_METHOD4(TP, Ret, Name, Params)                                \
  static Ret Name(Dyn ANGLE_EXTRA_ARGS(TP) &&                                  \
                  d VT_LAMBDA_EXTRA_PARAMS(Params)) {                          \
    return d.vtable->Name(d.object CALL_EXTRA_ARGS(Params));                   \
  }                                                                            \
  static Ret Name(Dyn ANGLE_EXTRA_ARGS(TP) &                                   \
                  d VT_LAMBDA_EXTRA_PARAMS(Params)) {                          \
    return d.vtable->Name(d.object CALL_EXTRA_ARGS(Params));                   \
  }                                                                            \
  static Ret Name(Dyn ANGLE_EXTRA_ARGS(TP) *                                   \
                  d VT_LAMBDA_EXTRA_PARAMS(Params)) {                          \
    return d->vtable->Name(d->object CALL_EXTRA_ARGS(Params));                 \
  }                                                                            \
  static Ret Name(const Dyn ANGLE_EXTRA_ARGS(TP) &                             \
                  d VT_LAMBDA_EXTRA_PARAMS(Params)) {                          \
    return d.vtable->Name(d.object CALL_EXTRA_ARGS(Params));                   \
  }                                                                            \
  static Ret Name(const Dyn ANGLE_EXTRA_ARGS(TP) *                             \
                  d VT_LAMBDA_EXTRA_PARAMS(Params)) {                          \
    return d->vtable->Name(d->object CALL_EXTRA_ARGS(Params));                 \
  }

//--------------------------------------------------------------------
//  Strict operation macros (exact signature for non‑Dyn types)
//--------------------------------------------------------------------
#define STRICT_TRAIT_REQ4_TUPLE(TP, M) STRICT_TRAIT_REQ4_APPLY(TP, UNWRAP(M))
#define STRICT_TRAIT_REQ4_APPLY(TP, ...) STRICT_TRAIT_REQ4(TP, __VA_ARGS__)
#define STRICT_TRAIT_REQ4(TP, Ret, Name, Params)                               \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}->std::same_as<Ret>;    \
  {&Impl<ALL_ARGS(TP)>::Name}->std::same_as<Ret (*)(UNWRAP(Params))>;

//--------------------------------------------------------------------
//  Static trait helpers (duck)
//--------------------------------------------------------------------
#define DUCK_STATIC_TRAIT_ITEM(TP, tuple)                                      \
  DUCK_STATIC_TRAIT_ITEM_I(TP, UNWRAP(tuple))
#define DUCK_STATIC_TRAIT_ITEM_I(TP, ...)                                      \
  DUCK_STATIC_TRAIT_ITEM_HELPER(TP, VA_COUNT(__VA_ARGS__), __VA_ARGS__)
#define DUCK_STATIC_TRAIT_ITEM_HELPER(TP, N, ...)                              \
  DUCK_STATIC_TRAIT_ITEM_II(TP, N, __VA_ARGS__)
#define DUCK_STATIC_TRAIT_ITEM_II(TP, N, ...)                                  \
  DUCK_STATIC_TRAIT_ITEM_##N(TP, __VA_ARGS__)

#define DUCK_STATIC_TRAIT_ITEM_2(TP, kword, Name)                              \
  typename Impl<ALL_ARGS(TP)>::Name;
#define DUCK_STATIC_TRAIT_ITEM_3(TP, Ret, Name, Params)                        \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}->std::same_as<Ret>;

#define DUCK_STATIC_TRAIT_FUNC(TP, tuple)                                      \
  DUCK_STATIC_TRAIT_FUNC_I(TP, UNWRAP(tuple))
#define DUCK_STATIC_TRAIT_FUNC_I(TP, ...)                                      \
  DUCK_STATIC_TRAIT_FUNC_HELPER(TP, VA_COUNT(__VA_ARGS__), __VA_ARGS__)
#define DUCK_STATIC_TRAIT_FUNC_HELPER(TP, N, ...)                              \
  DUCK_STATIC_TRAIT_FUNC_II(TP, N, __VA_ARGS__)
#define DUCK_STATIC_TRAIT_FUNC_II(TP, N, ...)                                  \
  DUCK_STATIC_TRAIT_FUNC_##N(TP, __VA_ARGS__)

#define DUCK_STATIC_TRAIT_FUNC_2(TP, kword, Name) /* nothing */
#define DUCK_STATIC_TRAIT_FUNC_3(TP, Ret, Name, Params)                        \
  FREE_FUNC4(TP, Ret, Name, Params)

//--------------------------------------------------------------------
//  Static trait helpers (strict)
//--------------------------------------------------------------------
#define STRICT_STATIC_TRAIT_ITEM(TP, tuple)                                    \
  STRICT_STATIC_TRAIT_ITEM_I(TP, UNWRAP(tuple))
#define STRICT_STATIC_TRAIT_ITEM_I(TP, ...)                                    \
  STRICT_STATIC_TRAIT_ITEM_HELPER(TP, VA_COUNT(__VA_ARGS__), __VA_ARGS__)
#define STRICT_STATIC_TRAIT_ITEM_HELPER(TP, N, ...)                            \
  STRICT_STATIC_TRAIT_ITEM_II(TP, N, __VA_ARGS__)
#define STRICT_STATIC_TRAIT_ITEM_II(TP, N, ...)                                \
  STRICT_STATIC_TRAIT_ITEM_##N(TP, __VA_ARGS__)

#define STRICT_STATIC_TRAIT_ITEM_2(TP, kword, Name)                            \
  typename Impl<ALL_ARGS(TP)>::Name;
#define STRICT_STATIC_TRAIT_ITEM_3(TP, Ret, Name, Params)                      \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}->std::same_as<Ret>;    \
  {&Impl<ALL_ARGS(TP)>::Name}->std::same_as<Ret (*)(UNWRAP(Params))>;

#define STRICT_STATIC_TRAIT_FUNC(TP, tuple)                                    \
  STRICT_STATIC_TRAIT_FUNC_I(TP, UNWRAP(tuple))
#define STRICT_STATIC_TRAIT_FUNC_I(TP, ...)                                    \
  STRICT_STATIC_TRAIT_FUNC_HELPER(TP, VA_COUNT(__VA_ARGS__), __VA_ARGS__)
#define STRICT_STATIC_TRAIT_FUNC_HELPER(TP, N, ...)                            \
  STRICT_STATIC_TRAIT_FUNC_II(TP, N, __VA_ARGS__)
#define STRICT_STATIC_TRAIT_FUNC_II(TP, N, ...)                                \
  STRICT_STATIC_TRAIT_FUNC_##N(TP, __VA_ARGS__)

#define STRICT_STATIC_TRAIT_FUNC_2(TP, kword, Name) /* nothing */
#define STRICT_STATIC_TRAIT_FUNC_3(TP, Ret, Name, Params)                      \
  FREE_FUNC4(TP, Ret, Name, Params)

//--------------------------------------------------------------------
//  Main macros
//--------------------------------------------------------------------

// Duck‑typed trait (loose checking, multiple Dyn overloads)
#define DuckTrait(NS, TP, ...)                                                 \
  namespace NS {                                                               \
  TEMPLATE_DECL(TP) struct Dyn;                                                \
  template <TYPENAME_LIST(TP)> struct Impl;                                    \
  template <TYPENAME_LIST(TP)>                                                 \
  concept Trait = requires(FIRST(TP) t) {                                      \
    FOR_EACH_WITH(DUCK_TRAIT_REQ4_TUPLE, TP, __VA_ARGS__)                      \
  };                                                                           \
  FOR_EACH_WITH(FREE_FUNC4_TUPLE, TP, __VA_ARGS__)                             \
  TEMPLATE_DECL(TP) struct VTable {                                            \
    FOR_EACH_WITH(VTABLE_MEMBER4_TUPLE, TP, __VA_ARGS__)                       \
  };                                                                           \
  template <TYPENAME_LIST(TP)>                                                 \
    requires Trait<ALL_ARGS(TP)>                                               \
  inline static const VTable ANGLE_EXTRA_ARGS(TP) vt = {                       \
      FOR_EACH_WITH(VT_ENTRY4_TUPLE, TP, __VA_ARGS__)};                        \
  TEMPLATE_DECL(TP) struct Dyn {                                               \
    void *object;                                                              \
    const VTable ANGLE_EXTRA_ARGS(TP) * vtable;                                \
    DYN_CTOR_CONSTRAINT(TP)                                                    \
    Dyn(FIRST(TP) & value) : object(&value), vtable(&vt<ALL_ARGS(TP)>) {}      \
    DYN_CTOR_CONSTRAINT(TP)                                                    \
    Dyn &operator=(FIRST(TP) & value) {                                        \
      object = &value;                                                         \
      vtable = &vt<ALL_ARGS(TP)>;                                              \
      return *this;                                                            \
    }                                                                          \
  };                                                                           \
  TEMPLATE_DECL(TP) IMPL_SPEC_HEAD(TP) struct Impl<DYN_IMPL_SPEC_ARGS(TP)> {   \
    FOR_EACH_WITH(IMPL_DYN_METHOD4_TUPLE, TP, __VA_ARGS__)                     \
  };                                                                           \
  }

// Static duck‑typed trait
#define StaticDuckTrait(NS, TP, ...)                                           \
  namespace NS {                                                               \
  template <TYPENAME_LIST(TP)> struct Impl;                                    \
  template <TYPENAME_LIST(TP)>                                                 \
  concept Trait = requires(FIRST(TP) t) {                                      \
    FOR_EACH_WITH(DUCK_STATIC_TRAIT_ITEM, TP, __VA_ARGS__)                     \
  };                                                                           \
  FOR_EACH_WITH(DUCK_STATIC_TRAIT_FUNC, TP, __VA_ARGS__)                       \
  }

// Strict trait (exact signatures for concrete types, duck for Dyn)
#define StrictTrait(NS, TP, ...)                                               \
  namespace NS {                                                               \
  TEMPLATE_DECL(TP) struct Dyn;                                                \
  template <TYPENAME_LIST(TP)> struct Impl;                                    \
  template <typename> struct TraitIsDyn : std::false_type {};                  \
  TEMPLATE_DECL(TP)                                                            \
  IMPL_SPEC_HEAD(TP)                                                           \
  struct TraitIsDyn<Dyn ANGLE_EXTRA_ARGS(TP)> : std::true_type {};             \
  template <TYPENAME_LIST(TP)>                                                 \
  concept TraitStrict = requires(FIRST(TP) t) {                                \
    FOR_EACH_WITH(STRICT_TRAIT_REQ4_TUPLE, TP, __VA_ARGS__)                    \
  };                                                                           \
  template <TYPENAME_LIST(TP)>                                                 \
  concept TraitDuck = requires(FIRST(TP) t) {                                  \
    FOR_EACH_WITH(DUCK_TRAIT_REQ4_TUPLE, TP, __VA_ARGS__)                      \
  };                                                                           \
  template <TYPENAME_LIST(TP)>                                                 \
  concept Trait = (TraitIsDyn<std::remove_cvref_t<FIRST(TP)>>::value &&        \
                   TraitDuck<ALL_ARGS(TP)>) ||                                 \
                  (!TraitIsDyn<std::remove_cvref_t<FIRST(TP)>>::value &&       \
                   TraitStrict<ALL_ARGS(TP)>);                                 \
  FOR_EACH_WITH(FREE_FUNC4_TUPLE, TP, __VA_ARGS__)                             \
  TEMPLATE_DECL(TP) struct VTable {                                            \
    FOR_EACH_WITH(VTABLE_MEMBER4_TUPLE, TP, __VA_ARGS__)                       \
  };                                                                           \
  template <TYPENAME_LIST(TP)>                                                 \
    requires Trait<ALL_ARGS(TP)>                                               \
  inline static const VTable ANGLE_EXTRA_ARGS(TP) vt = {                       \
      FOR_EACH_WITH(VT_ENTRY4_TUPLE, TP, __VA_ARGS__)};                        \
  TEMPLATE_DECL(TP) struct Dyn {                                               \
    void *object;                                                              \
    const VTable ANGLE_EXTRA_ARGS(TP) * vtable;                                \
    DYN_CTOR_CONSTRAINT(TP)                                                    \
    Dyn(FIRST(TP) & value) : object(&value), vtable(&vt<ALL_ARGS(TP)>) {}      \
    DYN_CTOR_CONSTRAINT(TP)                                                    \
    Dyn &operator=(FIRST(TP) & value) {                                        \
      object = &value;                                                         \
      vtable = &vt<ALL_ARGS(TP)>;                                              \
      return *this;                                                            \
    }                                                                          \
  };                                                                           \
  /* Dyn uses duck overloads – strict check skipped for Dyn */                 \
  TEMPLATE_DECL(TP) IMPL_SPEC_HEAD(TP) struct Impl<DYN_IMPL_SPEC_ARGS(TP)> {   \
    FOR_EACH_WITH(IMPL_DYN_METHOD4_TUPLE, TP, __VA_ARGS__)                     \
  };                                                                           \
  }

// Strict static trait
#define StrictStaticTrait(NS, TP, ...)                                         \
  namespace NS {                                                               \
  template <TYPENAME_LIST(TP)> struct Impl;                                    \
  template <TYPENAME_LIST(TP)>                                                 \
  concept Trait = requires(FIRST(TP) t) {                                      \
    FOR_EACH_WITH(STRICT_STATIC_TRAIT_ITEM, TP, __VA_ARGS__)                   \
  };                                                                           \
  FOR_EACH_WITH(STRICT_STATIC_TRAIT_FUNC, TP, __VA_ARGS__)                     \
  }

//--------------------------------------------------------------------
//  Frontend wrappers
//--------------------------------------------------------------------
#define trait(...) TRAIT_EXPAND_1(__VA_ARGS__)
#define TRAIT_EXPAND_1(...) TRAIT_EXPAND_2(__VA_ARGS__)
#define TRAIT_EXPAND_2(Name, TP, MethodsTuple)                                 \
  TRAIT_EXPAND_3(Name, TP, UNWRAP_I MethodsTuple)
#define TRAIT_EXPAND_3(Name, TP, ...) StrictTrait(Name, TP, __VA_ARGS__)

#define static_trait(...) STATIC_TRAIT_EXPAND_1(__VA_ARGS__)
#define STATIC_TRAIT_EXPAND_1(...) STATIC_TRAIT_EXPAND_2(__VA_ARGS__)
#define STATIC_TRAIT_EXPAND_2(Name, TP, MethodsTuple)                          \
  STATIC_TRAIT_EXPAND_3(Name, TP, UNWRAP_I MethodsTuple)
#define STATIC_TRAIT_EXPAND_3(Name, TP, ...)                                   \
  StrictStaticTrait(Name, TP, __VA_ARGS__)

#define ducktyped_trait(...) DUCKTYPED_TRAIT_EXPAND_1(__VA_ARGS__)
#define DUCKTYPED_TRAIT_EXPAND_1(...) DUCKTYPED_TRAIT_EXPAND_2(__VA_ARGS__)
#define DUCKTYPED_TRAIT_EXPAND_2(Name, TP, MethodsTuple)                       \
  DUCKTYPED_TRAIT_EXPAND_3(Name, TP, UNWRAP_I MethodsTuple)
#define DUCKTYPED_TRAIT_EXPAND_3(Name, TP, ...) DuckTrait(Name, TP, __VA_ARGS__)

#define static_ducktyped_trait(...) STATIC_DUCKTYPED_TRAIT_EXPAND_1(__VA_ARGS__)
#define STATIC_DUCKTYPED_TRAIT_EXPAND_1(...)                                   \
  STATIC_DUCKTYPED_TRAIT_EXPAND_2(__VA_ARGS__)
#define STATIC_DUCKTYPED_TRAIT_EXPAND_2(Name, TP, MethodsTuple)                \
  STATIC_DUCKTYPED_TRAIT_EXPAND_3(Name, TP, UNWRAP_I MethodsTuple)
#define STATIC_DUCKTYPED_TRAIT_EXPAND_3(Name, TP, ...)                         \
  StaticDuckTrait(Name, TP, __VA_ARGS__)

#endif // TRAIT_GENERATION_H
