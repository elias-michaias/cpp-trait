// clang-format off
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
//  FOR_EACH and FOR_EACH_WITH (curried)
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

// FOR_EACH_WITH(macro, data, items...) -- calls macro(data, item) for each item
#define FOR_EACH_WITH(macro, data, ...)                                        \
  __VA_OPT__(EXPAND(FEWH(macro, data, __VA_ARGS__)))
#define FEWH(macro, data, a1, ...)                                             \
  macro(data, a1) __VA_OPT__(FEWA PARENS(macro, data, __VA_ARGS__))
#define FEWA() FEWH

//--------------------------------------------------------------------
//  Arity / unwrap helpers
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
//  Method-params helpers  (Params = (Self, extras...))
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
//  TypeParam (TP) helpers  -- TP = (T) or (T, R) or (T, R, S)
//  Self = first element; Extra = rest
//--------------------------------------------------------------------

// typename T, typename R, ...
#define TYPENAME_LIST(TP) TYPENAME_LIST_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define TYPENAME_LIST_I(N, ...) TYPENAME_LIST_II(N, __VA_ARGS__)
#define TYPENAME_LIST_II(N, ...) TYPENAME_LIST_##N(__VA_ARGS__)
#define TYPENAME_LIST_1(A) typename A
#define TYPENAME_LIST_2(A, B) typename A, typename B
#define TYPENAME_LIST_3(A, B, C) typename A, typename B, typename C

// template<typename R> or template<typename R, typename S> or empty for 1‑param
#define TEMPLATE_DECL(TP) TEMPLATE_DECL_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define TEMPLATE_DECL_I(N, ...) TEMPLATE_DECL_II(N, __VA_ARGS__)
#define TEMPLATE_DECL_II(N, ...) TEMPLATE_DECL_##N(__VA_ARGS__)
#define TEMPLATE_DECL_1(A)
#define TEMPLATE_DECL_2(A, B) template <typename B>
#define TEMPLATE_DECL_3(A, B, C) template <typename B, typename C>

// Full specialisation head: `template<>` for 1‑param, nothing for >1
#define IMPL_SPEC_HEAD(TP) IMPL_SPEC_HEAD_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define IMPL_SPEC_HEAD_I(N, ...) IMPL_SPEC_HEAD_II(N, __VA_ARGS__)
#define IMPL_SPEC_HEAD_II(N, ...) IMPL_SPEC_HEAD_##N(__VA_ARGS__)
#define IMPL_SPEC_HEAD_1(A) template <>
#define IMPL_SPEC_HEAD_2(A, B)
#define IMPL_SPEC_HEAD_3(A, B, C)

// Angle‑bracket extra args: <R> or <R, S> or empty
#define ANGLE_EXTRA_ARGS(TP)                                                   \
  ANGLE_EXTRA_ARGS_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define ANGLE_EXTRA_ARGS_I(N, ...) ANGLE_EXTRA_ARGS_II(N, __VA_ARGS__)
#define ANGLE_EXTRA_ARGS_II(N, ...) ANGLE_EXTRA_ARGS_##N(__VA_ARGS__)
#define ANGLE_EXTRA_ARGS_1(A)
#define ANGLE_EXTRA_ARGS_2(A, B) <B>
#define ANGLE_EXTRA_ARGS_3(A, B, C) <B, C>

// All args: T, R or T, R, S or just T
#define ALL_ARGS(TP) ALL_ARGS_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define ALL_ARGS_I(N, ...) ALL_ARGS_II(N, __VA_ARGS__)
#define ALL_ARGS_II(N, ...) ALL_ARGS_##N(__VA_ARGS__)
#define ALL_ARGS_1(A) A
#define ALL_ARGS_2(A, B) A, B
#define ALL_ARGS_3(A, B, C) A, B, C

// Tail args (everything after the first type)
#define TAIL_ARGS(TP) TAIL_ARGS_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define TAIL_ARGS_I(N, ...) TAIL_ARGS_II(N, __VA_ARGS__)
#define TAIL_ARGS_II(N, ...) TAIL_ARGS_##N(__VA_ARGS__)
#define TAIL_ARGS_1(A)                /* nothing */
#define TAIL_ARGS_2(A, B)             B
#define TAIL_ARGS_3(A, B, C)          B, C

// Comma + tail args if any: , R or , R, S or nothing
#define COMMA_TAIL(TP) COMMA_TAIL_I(VA_COUNT(UNWRAP(TP)), TP)
#define COMMA_TAIL_I(N, TP) COMMA_TAIL_II(N, TP)
#define COMMA_TAIL_II(N, TP) COMMA_TAIL_##N(TP)
#define COMMA_TAIL_1(TP)
#define COMMA_TAIL_2(TP) , TAIL_ARGS(TP)
#define COMMA_TAIL_3(TP) , TAIL_ARGS(TP)

// Impl<Dyn<R>, R> or Impl<Dyn<R,S>, R,S> etc.
#define DYN_IMPL_SPEC_ARGS(TP) Dyn ANGLE_EXTRA_ARGS(TP) COMMA_TAIL(TP)

// Free function template head (constrains on Trait with full type list)
#define FUNC_TEMPLATE_HEAD(TP)                                                 \
  FUNC_TEMPLATE_HEAD_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define FUNC_TEMPLATE_HEAD_I(N, ...) FUNC_TEMPLATE_HEAD_II(N, __VA_ARGS__)
#define FUNC_TEMPLATE_HEAD_II(N, ...) FUNC_TEMPLATE_HEAD_##N(__VA_ARGS__)
#define FUNC_TEMPLATE_HEAD_1(A) template <Trait A>
#define FUNC_TEMPLATE_HEAD_2(A, B) template <typename B, Trait<B> A>
#define FUNC_TEMPLATE_HEAD_3(A, B, C)                                          \
  template <typename B, typename C, Trait<B, C> A>

// Dyn constructor constraint
#define DYN_CTOR_CONSTRAINT(TP)                                                \
  DYN_CTOR_CONSTRAINT_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define DYN_CTOR_CONSTRAINT_I(N, ...) DYN_CTOR_CONSTRAINT_II(N, __VA_ARGS__)
#define DYN_CTOR_CONSTRAINT_II(N, ...) DYN_CTOR_CONSTRAINT_##N(__VA_ARGS__)
#define DYN_CTOR_CONSTRAINT_1(A) template <Trait A>
#define DYN_CTOR_CONSTRAINT_2(A, B)                                            \
  template <typename A>                                                        \
    requires Trait<A, B>
#define DYN_CTOR_CONSTRAINT_3(A, B, C)                                         \
  template <typename A>                                                        \
    requires Trait<A, B, C>

//--------------------------------------------------------------------
//  Per‑method generators (TP, Ret, Name, Params)
//--------------------------------------------------------------------

// Bridge macros to unwrap inner tuple
#define TRAIT_REQ4_TUPLE(TP, M) TRAIT_REQ4_APPLY(TP, UNWRAP(M))
#define FREE_FUNC4_TUPLE(TP, M) FREE_FUNC4_APPLY(TP, UNWRAP(M))
#define VTABLE_MEMBER4_TUPLE(TP, M) VTABLE_MEMBER4_APPLY(TP, UNWRAP(M))
#define VT_ENTRY4_TUPLE(TP, M) VT_ENTRY4_APPLY(TP, UNWRAP(M))
#define IMPL_DYN_METHOD4_TUPLE(TP, M) IMPL_DYN_METHOD4_APPLY(TP, UNWRAP(M))

#define TRAIT_REQ4_APPLY(TP, ...) TRAIT_REQ4(TP, __VA_ARGS__)
#define FREE_FUNC4_APPLY(TP, ...) FREE_FUNC4(TP, __VA_ARGS__)
#define VTABLE_MEMBER4_APPLY(TP, ...) VTABLE_MEMBER4(TP, __VA_ARGS__)
#define VT_ENTRY4_APPLY(TP, ...) VT_ENTRY4(TP, __VA_ARGS__)
#define IMPL_DYN_METHOD4_APPLY(TP, ...) IMPL_DYN_METHOD4(TP, __VA_ARGS__)

// Actual macros – now use ALL_ARGS(TP) for the full template argument list
#define TRAIT_REQ4(TP, Ret, Name, Params)                                      \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}->std::same_as<Ret>;

#define FREE_FUNC4(TP, Ret, Name, Params)                                      \
  FUNC_TEMPLATE_HEAD(TP) Ret Name(FUNC_PARAMS(Params)) {                       \
    return Impl<ALL_ARGS(TP)>::Name(CALL_ARGS(Params));                        \
  }

#define VTABLE_MEMBER4(TP, Ret, Name, Params)                                  \
  Ret (*Name)(void *VTABLE_EXTRA_PARAMS(Params));

#define VT_ENTRY4(TP, Ret, Name, Params)                                       \
  .Name = [](void *p VT_LAMBDA_EXTRA_PARAMS(Params)) -> Ret {                  \
    using Receiver = FIRST(Params);                                            \
    return Impl<ALL_ARGS(TP)>::Name(                                           \
        ::gen_interface_detail::receiver_from<Receiver, FIRST(TP)>(p)          \
            CALL_EXTRA_ARGS(Params));                                          \
  },

#define IMPL_DYN_METHOD4(TP, Ret, Name, Params)                                \
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
//  Main macro: Trait(NS, (TypeParams), methods...)
//--------------------------------------------------------------------
#define Trait(NS, TP, ...)                                                     \
  namespace NS {                                                               \
  /* Primary Impl template with all type parameters */                         \
  template <TYPENAME_LIST(TP)> struct Impl;                                    \
                                                                               \
  template <TYPENAME_LIST(TP)>                                                 \
  concept Trait = requires(FIRST(TP) t) {                                      \
    FOR_EACH_WITH(TRAIT_REQ4_TUPLE, TP, __VA_ARGS__)                           \
  };                                                                           \
                                                                               \
  FOR_EACH_WITH(FREE_FUNC4_TUPLE, TP, __VA_ARGS__)                             \
                                                                               \
  TEMPLATE_DECL(TP) struct VTable {                                            \
    FOR_EACH_WITH(VTABLE_MEMBER4_TUPLE, TP, __VA_ARGS__)                       \
  };                                                                           \
                                                                               \
  template <TYPENAME_LIST(TP)>                                                 \
    requires Trait<ALL_ARGS(TP)>                                               \
  inline static const VTable ANGLE_EXTRA_ARGS(TP) vt = {                       \
      FOR_EACH_WITH(VT_ENTRY4_TUPLE, TP, __VA_ARGS__)};                        \
                                                                               \
  TEMPLATE_DECL(TP) struct Dyn {                                               \
    void *object;                                                              \
    const VTable ANGLE_EXTRA_ARGS(TP) * vtable;                                \
                                                                               \
    /* Construct from a concrete type */                                       \
    DYN_CTOR_CONSTRAINT(TP)                                                    \
    Dyn(FIRST(TP) & value) : object(&value), vtable(&vt<ALL_ARGS(TP)>) {}      \
                                                                               \
    /* Rebind to a new concrete type */                                        \
    DYN_CTOR_CONSTRAINT(TP)                                                    \
    Dyn& operator=(FIRST(TP)& value) {                                         \
      object = &value;                                                         \
      vtable = &vt<ALL_ARGS(TP)>;                                              \
      return *this;                                                            \
    }                                                                          \
  };                                                                           \
                                                                               \
  /* Impl<Dyn<Extra...>, Extra...> specialization */                           \
  TEMPLATE_DECL(TP) IMPL_SPEC_HEAD(TP) struct Impl< DYN_IMPL_SPEC_ARGS(TP) > { \
    FOR_EACH_WITH(IMPL_DYN_METHOD4_TUPLE, TP, __VA_ARGS__)                     \
  };                                                                           \
  }

//--------------------------------------------------------------------
//  Frontend API wrapper: trait(NS, (TP), (method, method, ...))
//--------------------------------------------------------------------
#define trait(...) TRAIT_EXPAND_1(__VA_ARGS__)
#define TRAIT_EXPAND_1(...) TRAIT_EXPAND_2(__VA_ARGS__)
#define TRAIT_EXPAND_2(Name, TP, MethodsTuple) TRAIT_EXPAND_3(Name, TP, UNWRAP_I MethodsTuple)
#define TRAIT_EXPAND_3(Name, TP, ...) Trait(Name, TP, __VA_ARGS__)

#endif // TRAIT_GENERATION_H
