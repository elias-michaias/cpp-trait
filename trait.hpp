#ifndef TRAIT_HOF_AFTER_SELF_NEW_HPP
#define TRAIT_HOF_AFTER_SELF_NEW_HPP

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace gen_interface_detail {

struct identity_callable {
  template <class X>
  constexpr std::remove_cvref_t<X> operator()(X&& x) const noexcept {
    return x;
  }
};

template <class Receiver, class T>
constexpr decltype(auto) receiver_from(void *p) {
  if constexpr (std::is_pointer_v<Receiver>)
    return static_cast<Receiver>(p);
  else
    return *static_cast<std::remove_reference_t<Receiver> *>(p);
}

template <class Ret, class... Args>
struct probe_callable {
  constexpr Ret operator()(Args...) const noexcept;
};

} // namespace gen_interface_detail

//--------------------------------------------------------------------
//  Preprocessor helpers
//--------------------------------------------------------------------
#define PARENS ()
#define EXPAND(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) __VA_ARGS__

#define CAT(a, b) CAT_I(a, b)
#define CAT_I(a, b) a##b

#define PROBE(x) x, 1
#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0)
#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)
#define IS_PAREN_PROBE(...) PROBE(~)
#define IS_TEMPLATE(x) CHECK(CAT(IS_TEMPLATE_, x))
#define IS_TEMPLATE_template PROBE(~)

//--------------------------------------------------------------------
//  FOR_EACH / FOR_EACH_WITH
//--------------------------------------------------------------------
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

#define FOR_EACH_WITH2(macro, data1, data2, ...)                               \
  __VA_OPT__(EXPAND(FEWH2(macro, data1, data2, __VA_ARGS__)))
#define FEWH2(macro, data1, data2, a1, ...)                                    \
  macro(data1, data2, a1) __VA_OPT__(FEWA2 PARENS(macro, data1, data2, __VA_ARGS__))
#define FEWA2() FEWH2

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
//  Type normalization
//--------------------------------------------------------------------
#define TYPE_SPEC(X) CAT(TYPE_SPEC_, IS_PAREN(X))(X)
#define TYPE_SPEC_0(X) X
#define TYPE_SPEC_1(X) TYPE_SPEC_1_I(VA_COUNT(UNWRAP(X)), UNWRAP(X))
#define TYPE_SPEC_1_I(N, ...) TYPE_SPEC_1_II(N, __VA_ARGS__)
#define TYPE_SPEC_1_II(N, ...) TYPE_SPEC_1_##N(__VA_ARGS__)
#define TYPE_SPEC_1_1(X) UNWRAP(X)
#define TYPE_SPEC_1_2(kind, Expr) UNWRAP(Expr)
#define TYPE_SPEC_1_3(kind, Ret, A1) Ret (*)(A1)
#define TYPE_SPEC_1_4(kind, Ret, A1, A2) Ret (*)(A1, A2)
#define TYPE_SPEC_1_5(kind, Ret, A1, A2, A3) Ret (*)(A1, A2, A3)
#define TYPE_SPEC_1_6(kind, Ret, A1, A2, A3, A4) Ret (*)(A1, A2, A3, A4)

#define PARAM_TYPES_1(A1) TYPE_SPEC(A1)
#define PARAM_TYPES_2(A1, A2) TYPE_SPEC(A1), TYPE_SPEC(A2)
#define PARAM_TYPES_3(A1, A2, A3) TYPE_SPEC(A1), TYPE_SPEC(A2), TYPE_SPEC(A3)
#define PARAM_TYPES_4(A1, A2, A3, A4)                                        \
  TYPE_SPEC(A1), TYPE_SPEC(A2), TYPE_SPEC(A3), TYPE_SPEC(A4)
#define PARAM_TYPES_5(A1, A2, A3, A4, A5)                                     \
  TYPE_SPEC(A1), TYPE_SPEC(A2), TYPE_SPEC(A3), TYPE_SPEC(A4), TYPE_SPEC(A5)
#define PARAM_TYPES(P) PARAM_TYPES_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define PARAM_TYPES_I(N, ...) PARAM_TYPES_II(N, __VA_ARGS__)
#define PARAM_TYPES_II(N, ...) PARAM_TYPES_##N(__VA_ARGS__)

#define DECLVALS_1(A1) std::declval<TYPE_SPEC(A1)>()
#define DECLVALS_2(A1, A2) std::declval<TYPE_SPEC(A1)>(), std::declval<TYPE_SPEC(A2)>()
#define DECLVALS_3(A1, A2, A3)                                                \
  std::declval<TYPE_SPEC(A1)>(), std::declval<TYPE_SPEC(A2)>(),               \
      std::declval<TYPE_SPEC(A3)>()
#define DECLVALS_4(A1, A2, A3, A4)                                            \
  std::declval<TYPE_SPEC(A1)>(), std::declval<TYPE_SPEC(A2)>(),               \
      std::declval<TYPE_SPEC(A3)>(), std::declval<TYPE_SPEC(A4)>()
#define DECLVALS_5(A1, A2, A3, A4, A5)                                        \
  std::declval<TYPE_SPEC(A1)>(), std::declval<TYPE_SPEC(A2)>(),               \
      std::declval<TYPE_SPEC(A3)>(), std::declval<TYPE_SPEC(A4)>(),           \
      std::declval<TYPE_SPEC(A5)>()
#define TUPLE_TO_DECLVALS(P) DECLVALS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define DECLVALS_I(N, ...) DECLVALS_II(N, __VA_ARGS__)
#define DECLVALS_II(N, ...) DECLVALS_##N(__VA_ARGS__)

#define FUNC_PARAMS_1(S) PARAM_DECL(S, self)
#define FUNC_PARAMS_2(S, T1) PARAM_DECL(S, self), PARAM_DECL(T1, p1)
#define FUNC_PARAMS_3(S, T1, T2)                                              \
  PARAM_DECL(S, self), PARAM_DECL(T1, p1), PARAM_DECL(T2, p2)
#define FUNC_PARAMS_4(S, T1, T2, T3)                                          \
  PARAM_DECL(S, self), PARAM_DECL(T1, p1), PARAM_DECL(T2, p2),                 \
      PARAM_DECL(T3, p3)
#define FUNC_PARAMS_5(S, T1, T2, T3, T4)                                      \
  PARAM_DECL(S, self), PARAM_DECL(T1, p1), PARAM_DECL(T2, p2),                 \
      PARAM_DECL(T3, p3), PARAM_DECL(T4, p4)
#define FUNC_PARAMS(P) FUNC_PARAMS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define FUNC_PARAMS_I(N, ...) FUNC_PARAMS_II(N, __VA_ARGS__)
#define FUNC_PARAMS_II(N, ...) FUNC_PARAMS_##N(__VA_ARGS__)

#define CALL_ARGS_1(S) self
#define CALL_ARGS_2(S, T1) self, std::forward<decltype(p1)>(p1)
#define CALL_ARGS_3(S, T1, T2) self, std::forward<decltype(p1)>(p1), std::forward<decltype(p2)>(p2)
#define CALL_ARGS_4(S, T1, T2, T3) self, std::forward<decltype(p1)>(p1), std::forward<decltype(p2)>(p2), std::forward<decltype(p3)>(p3)
#define CALL_ARGS_5(S, T1, T2, T3, T4) self, std::forward<decltype(p1)>(p1), std::forward<decltype(p2)>(p2), std::forward<decltype(p3)>(p3), std::forward<decltype(p4)>(p4)
#define CALL_ARGS(P) CALL_ARGS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define CALL_ARGS_I(N, ...) CALL_ARGS_II(N, __VA_ARGS__)
#define CALL_ARGS_II(N, ...) CALL_ARGS_##N(__VA_ARGS__)

#define CALL_EXTRA_ARGS_1(S)
#define CALL_EXTRA_ARGS_2(S, T1) , std::forward<decltype(p1)>(p1)
#define CALL_EXTRA_ARGS_3(S, T1, T2) , std::forward<decltype(p1)>(p1), std::forward<decltype(p2)>(p2)
#define CALL_EXTRA_ARGS_4(S, T1, T2, T3) , std::forward<decltype(p1)>(p1), std::forward<decltype(p2)>(p2), std::forward<decltype(p3)>(p3)
#define CALL_EXTRA_ARGS_5(S, T1, T2, T3, T4) , std::forward<decltype(p1)>(p1), std::forward<decltype(p2)>(p2), std::forward<decltype(p3)>(p3), std::forward<decltype(p4)>(p4)
#define CALL_EXTRA_ARGS(P) CALL_EXTRA_ARGS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define CALL_EXTRA_ARGS_I(N, ...) CALL_EXTRA_ARGS_II(N, __VA_ARGS__)
#define CALL_EXTRA_ARGS_II(N, ...) CALL_EXTRA_ARGS_##N(__VA_ARGS__)

//----------------------------------------------------------------------
//  Named parameter declarations (Strict)
//----------------------------------------------------------------------
#define PARAM_DECL(X, Name) PARAM_DECL_I(X, Name)
#define PARAM_DECL_I(X, Name) CAT(PARAM_DECL_, IS_PAREN(X))(X, Name)
#define PARAM_DECL_0(X, Name) TYPE_SPEC(X) Name
#define PARAM_DECL_1(X, Name) PARAM_DECL_1_I(VA_COUNT(UNWRAP(X)), UNWRAP(X), Name)
#define PARAM_DECL_1_I(N, ...) PARAM_DECL_1_II(N, __VA_ARGS__)
#define PARAM_DECL_1_II(N, ...) PARAM_DECL_1_##N(__VA_ARGS__)
#define PARAM_DECL_1_1(X, Name) TYPE_SPEC(X) Name
#define PARAM_DECL_1_2(kind, Expr, Name) TYPE_SPEC(Expr) Name
#define PARAM_DECL_1_3(kind, Ret, A1, Name) Ret (*Name)(A1)
#define PARAM_DECL_1_4(kind, Ret, A1, A2, Name) Ret (*Name)(A1, A2)
#define PARAM_DECL_1_5(kind, Ret, A1, A2, A3, Name) Ret (*Name)(A1, A2, A3)
#define PARAM_DECL_1_6(kind, Ret, A1, A2, A3, A4, Name) Ret (*Name)(A1, A2, A3, A4)

//----------------------------------------------------------------------
//  Associated-template helpers
//----------------------------------------------------------------------
#define TEMPLATE_PLACEHOLDER_ARGS_1(A1) int
#define TEMPLATE_PLACEHOLDER_ARGS_2(A1, A2) int, int
#define TEMPLATE_PLACEHOLDER_ARGS_3(A1, A2, A3) int, int, int
#define TEMPLATE_PLACEHOLDER_ARGS(P)                                           \
  TEMPLATE_PLACEHOLDER_ARGS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define TEMPLATE_PLACEHOLDER_ARGS_I(N, ...) TEMPLATE_PLACEHOLDER_ARGS_II(N, __VA_ARGS__)
#define TEMPLATE_PLACEHOLDER_ARGS_II(N, ...) TEMPLATE_PLACEHOLDER_ARGS_##N(__VA_ARGS__)

#define VTABLE_EXTRA_PARAMS_1(S)
#define VTABLE_EXTRA_PARAMS_2(S, T1) , TYPE_SPEC(T1)
#define VTABLE_EXTRA_PARAMS_3(S, T1, T2) , TYPE_SPEC(T1), TYPE_SPEC(T2)
#define VTABLE_EXTRA_PARAMS_4(S, T1, T2, T3) , TYPE_SPEC(T1), TYPE_SPEC(T2), TYPE_SPEC(T3)
#define VTABLE_EXTRA_PARAMS_5(S, T1, T2, T3, T4) , TYPE_SPEC(T1), TYPE_SPEC(T2), TYPE_SPEC(T3), TYPE_SPEC(T4)
#define VTABLE_EXTRA_PARAMS(P)                                                 \
  VTABLE_EXTRA_PARAMS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define VTABLE_EXTRA_PARAMS_I(N, ...) VTABLE_EXTRA_PARAMS_II(N, __VA_ARGS__)
#define VTABLE_EXTRA_PARAMS_II(N, ...) VTABLE_EXTRA_PARAMS_##N(__VA_ARGS__)

#define VT_LAMBDA_EXTRA_PARAMS_1(S)
#define VT_LAMBDA_EXTRA_PARAMS_2(S, T1) , PARAM_DECL(T1, p1)
#define VT_LAMBDA_EXTRA_PARAMS_3(S, T1, T2) , PARAM_DECL(T1, p1), PARAM_DECL(T2, p2)
#define VT_LAMBDA_EXTRA_PARAMS_4(S, T1, T2, T3)                               \
  , PARAM_DECL(T1, p1), PARAM_DECL(T2, p2), PARAM_DECL(T3, p3)
#define VT_LAMBDA_EXTRA_PARAMS_5(S, T1, T2, T3, T4)                           \
  , PARAM_DECL(T1, p1), PARAM_DECL(T2, p2), PARAM_DECL(T3, p3), PARAM_DECL(T4, p4)
#define VT_LAMBDA_EXTRA_PARAMS(P)                                              \
  VT_LAMBDA_EXTRA_PARAMS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define VT_LAMBDA_EXTRA_PARAMS_I(N, ...)                                      \
  VT_LAMBDA_EXTRA_PARAMS_II(N, __VA_ARGS__)
#define VT_LAMBDA_EXTRA_PARAMS_II(N, ...)                                     \
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
#define TAIL_ARGS_2(A, B) TYPE_SPEC(B)
#define TAIL_ARGS_3(A, B, C) TYPE_SPEC(B), TYPE_SPEC(C)

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
#define FUNC_TEMPLATE_HEAD_3(A, B, C) template <typename B, typename C, Trait<B, C> A>

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
//  Duck‑typed operation macros
//--------------------------------------------------------------------
#define DUCK_TRAIT_REQ4_TUPLE(TP, M) DUCK_TRAIT_REQ4_APPLY(TP, UNWRAP(M))
#define DUCK_TRAIT_REQ4_APPLY(TP, ...) DUCK_TRAIT_REQ4(TP, __VA_ARGS__)
#define DUCK_TRAIT_REQ4(TP, Ret, Name, Params)                                 \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}->std::same_as<TYPE_SPEC(Ret)>;

// Generates both strict and generic overloads for free functions
#define FREE_FUNC4_TUPLE(TP, M) FREE_FUNC4_APPLY(TP, UNWRAP(M))
#define FREE_FUNC4_APPLY(TP, ...) FREE_FUNC4(TP, __VA_ARGS__)
#define FREE_FUNC4(TP, Ret, Name, Params)                                      \
  FUNC_TEMPLATE_HEAD(TP) TYPE_SPEC(Ret) Name(FUNC_PARAMS(Params)) {            \
    return Impl<ALL_ARGS(TP)>::Name(CALL_ARGS(Params));                        \
  }                                                                            

#define VTABLE_MEMBER4_TUPLE(TP, M) VTABLE_MEMBER4_APPLY(TP, UNWRAP(M))
#define VTABLE_MEMBER4_APPLY(TP, ...) VTABLE_MEMBER4(TP, __VA_ARGS__)
#define VTABLE_MEMBER4(TP, Ret, Name, Params)                                  \
  TYPE_SPEC(Ret) (*Name)(void *VTABLE_EXTRA_PARAMS(Params));

#define VT_ENTRY4_TUPLE(TP, M) VT_ENTRY4_APPLY(TP, UNWRAP(M))
#define VT_ENTRY4_APPLY(TP, ...) VT_ENTRY4(TP, __VA_ARGS__)
#define VT_ENTRY4(TP, Ret, Name, Params)                                       \
  .Name = [](void *p VT_LAMBDA_EXTRA_PARAMS(Params)) -> TYPE_SPEC(Ret) {       \
    using Receiver = FIRST(Params);                                            \
    return Impl<ALL_ARGS(TP)>::Name(                                           \
        ::gen_interface_detail::receiver_from<Receiver, FIRST(TP)>(p)          \
            CALL_EXTRA_ARGS(Params));                                          \
  },

#define IMPL_DYN_METHOD4_TUPLE(TP, M) IMPL_DYN_METHOD4_APPLY(TP, UNWRAP(M))
#define IMPL_DYN_METHOD4_APPLY(TP, ...) IMPL_DYN_METHOD4(TP, __VA_ARGS__)
#define IMPL_DYN_METHOD4(TP, Ret, Name, Params)                                \
  static TYPE_SPEC(Ret) Name(Dyn ANGLE_EXTRA_ARGS(TP) &&                       \
                  d VT_LAMBDA_EXTRA_PARAMS(Params)) {                          \
    return d.vtable->Name(d.object CALL_EXTRA_ARGS(Params));                   \
  }                                                                            \
  static TYPE_SPEC(Ret) Name(Dyn ANGLE_EXTRA_ARGS(TP) &                        \
                  d VT_LAMBDA_EXTRA_PARAMS(Params)) {                          \
    return d.vtable->Name(d.object CALL_EXTRA_ARGS(Params));                   \
  }                                                                            \
  static TYPE_SPEC(Ret) Name(Dyn ANGLE_EXTRA_ARGS(TP) * \
                  d VT_LAMBDA_EXTRA_PARAMS(Params)) {                          \
    return d->vtable->Name(d->object CALL_EXTRA_ARGS(Params));                 \
  }                                                                            \
  static TYPE_SPEC(Ret) Name(const Dyn ANGLE_EXTRA_ARGS(TP) &                  \
                  d VT_LAMBDA_EXTRA_PARAMS(Params)) {                          \
    return d.vtable->Name(d.object CALL_EXTRA_ARGS(Params));                   \
  }                                                                            \
  static TYPE_SPEC(Ret) Name(const Dyn ANGLE_EXTRA_ARGS(TP) * \
                  d VT_LAMBDA_EXTRA_PARAMS(Params)) {                          \
    return d->vtable->Name(d->object CALL_EXTRA_ARGS(Params));                 \
  }

//--------------------------------------------------------------------
//  Mixin helpers
//--------------------------------------------------------------------
#if (defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L) || defined(__clang__)

#define MIXIN_TEMPLATE_HEAD(TP)                                                \
  MIXIN_TEMPLATE_HEAD_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define MIXIN_TEMPLATE_HEAD_I(N, ...) MIXIN_TEMPLATE_HEAD_II(N, __VA_ARGS__)
#define MIXIN_TEMPLATE_HEAD_II(N, ...) MIXIN_TEMPLATE_HEAD_##N(__VA_ARGS__)
#define MIXIN_TEMPLATE_HEAD_1(A)
#define MIXIN_TEMPLATE_HEAD_2(A, B) template <typename B>
#define MIXIN_TEMPLATE_HEAD_3(A, B, C) template <typename B, typename C>

#define MIXIN_METHOD_EXTRA_PARAMS(P)                                          \
  MIXIN_METHOD_EXTRA_PARAMS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define MIXIN_METHOD_EXTRA_PARAMS_I(N, ...)                                    \
  MIXIN_METHOD_EXTRA_PARAMS_II(N, __VA_ARGS__)
#define MIXIN_METHOD_EXTRA_PARAMS_II(N, ...)                                   \
  MIXIN_METHOD_EXTRA_PARAMS_##N(__VA_ARGS__)
#define MIXIN_METHOD_EXTRA_PARAMS_1(S)
#define MIXIN_METHOD_EXTRA_PARAMS_2(S, T1) , PARAM_DECL(T1, p1)
#define MIXIN_METHOD_EXTRA_PARAMS_3(S, T1, T2) , PARAM_DECL(T1, p1), PARAM_DECL(T2, p2)
#define MIXIN_METHOD_EXTRA_PARAMS_4(S, T1, T2, T3)                             \
  , PARAM_DECL(T1, p1), PARAM_DECL(T2, p2), PARAM_DECL(T3, p3)
#define MIXIN_METHOD_EXTRA_PARAMS_5(S, T1, T2, T3, T4)                         \
  , PARAM_DECL(T1, p1), PARAM_DECL(T2, p2), PARAM_DECL(T3, p3), PARAM_DECL(T4, p4)

#define MIXIN_METHOD4_TUPLE(NS, TP, M) MIXIN_METHOD4_APPLY(NS, TP, UNWRAP(M))
#define MIXIN_METHOD4_APPLY(NS, TP, ...) MIXIN_METHOD4(TP, NS, __VA_ARGS__)
#define MIXIN_METHOD4(TP, NS, Ret, Name, Params)                               \
  TYPE_SPEC(Ret) Name(this auto &self MIXIN_METHOD_EXTRA_PARAMS(Params)) {     \
    if constexpr (requires {                                                   \
                    ::NS::Name ANGLE_EXTRA_ARGS(TP)(self CALL_EXTRA_ARGS(Params)); \
                  }) {                                                         \
      if constexpr (std::is_void_v<TYPE_SPEC(Ret)>) {                          \
        ::NS::Name ANGLE_EXTRA_ARGS(TP)(self CALL_EXTRA_ARGS(Params));         \
      } else {                                                                 \
        return ::NS::Name ANGLE_EXTRA_ARGS(TP)(self CALL_EXTRA_ARGS(Params));  \
      }                                                                        \
    } else {                                                                   \
      if constexpr (std::is_void_v<TYPE_SPEC(Ret)>) {                          \
        ::NS::Name ANGLE_EXTRA_ARGS(TP)(&self CALL_EXTRA_ARGS(Params));        \
      } else {                                                                 \
        return ::NS::Name ANGLE_EXTRA_ARGS(TP)(&self CALL_EXTRA_ARGS(Params)); \
      }                                                                        \
    }                                                                          \
  }

#else

#define MIXIN_TEMPLATE_HEAD(TP)                                                \
  MIXIN_TEMPLATE_HEAD_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define MIXIN_TEMPLATE_HEAD_I(N, ...) MIXIN_TEMPLATE_HEAD_II(N, __VA_ARGS__)
#define MIXIN_TEMPLATE_HEAD_II(N, ...) MIXIN_TEMPLATE_HEAD_##N(__VA_ARGS__)
#define MIXIN_TEMPLATE_HEAD_1(A) template <class Derived>
#define MIXIN_TEMPLATE_HEAD_2(A, B) template <class Derived, typename B>
#define MIXIN_TEMPLATE_HEAD_3(A, B, C)                                         \
  template <class Derived, typename B, typename C>

#define MIXIN_METHOD_PARAMS(P)                                                \
  MIXIN_METHOD_PARAMS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define MIXIN_METHOD_PARAMS_I(N, ...)                                          \
  MIXIN_METHOD_PARAMS_II(N, __VA_ARGS__)
#define MIXIN_METHOD_PARAMS_II(N, ...)                                         \
  MIXIN_METHOD_PARAMS_##N(__VA_ARGS__)
#define MIXIN_METHOD_PARAMS_1(S)
#define MIXIN_METHOD_PARAMS_2(S, T1) PARAM_DECL(T1, p1)
#define MIXIN_METHOD_PARAMS_3(S, T1, T2) PARAM_DECL(T1, p1), PARAM_DECL(T2, p2)
#define MIXIN_METHOD_PARAMS_4(S, T1, T2, T3)                                   \
  PARAM_DECL(T1, p1), PARAM_DECL(T2, p2), PARAM_DECL(T3, p3)
#define MIXIN_METHOD_PARAMS_5(S, T1, T2, T3, T4)                               \
  PARAM_DECL(T1, p1), PARAM_DECL(T2, p2), PARAM_DECL(T3, p3), PARAM_DECL(T4, p4)

#define MIXIN_METHOD4_TUPLE(NS, TP, M) MIXIN_METHOD4_APPLY(NS, TP, UNWRAP(M))
#define MIXIN_METHOD4_APPLY(NS, TP, ...) MIXIN_METHOD4(TP, NS, __VA_ARGS__)
#define MIXIN_METHOD4(TP, NS, Ret, Name, Params)                               \
  TYPE_SPEC(Ret) Name(MIXIN_METHOD_PARAMS(Params)) {                           \
    if constexpr (requires {                                                   \
                    ::NS::Name ANGLE_EXTRA_ARGS(TP)(static_cast<Derived &>(    \
                        *this) CALL_EXTRA_ARGS(Params));                       \
                  }) {                                                         \
      if constexpr (std::is_void_v<decltype(::NS::Name ANGLE_EXTRA_ARGS(TP)(   \
                        static_cast<Derived &>(*this)                          \
                            CALL_EXTRA_ARGS(Params)))>) {                      \
        ::NS::Name ANGLE_EXTRA_ARGS(TP)(static_cast<Derived &>(*this)          \
                                            CALL_EXTRA_ARGS(Params));          \
      } else {                                                                 \
        return ::NS::Name ANGLE_EXTRA_ARGS(TP)(static_cast<Derived &>(*this)   \
                                                   CALL_EXTRA_ARGS(Params));   \
      }                                                                        \
    } else {                                                                   \
      if constexpr (std::is_void_v<decltype(::NS::Name ANGLE_EXTRA_ARGS(TP)(   \
                        static_cast<Derived *>(this)                           \
                            CALL_EXTRA_ARGS(Params)))>) {                      \
        ::NS::Name ANGLE_EXTRA_ARGS(TP)(static_cast<Derived *>(this)           \
                                            CALL_EXTRA_ARGS(Params));          \
      } else {                                                                 \
        return ::NS::Name ANGLE_EXTRA_ARGS(TP)(static_cast<Derived *>(this)    \
                                                   CALL_EXTRA_ARGS(Params));   \
      }                                                                        \
    }                                                                          \
  }

#endif

#if (defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L) || defined(__clang__)
#define MIXIN_BASE(TP) : Mixin ANGLE_EXTRA_ARGS(TP)
#else
#define MIXIN_BASE(TP) : Mixin<DYN_IMPL_SPEC_ARGS(TP)>
#endif

//--------------------------------------------------------------------
//  Strict operation macros
//--------------------------------------------------------------------
#define STRICT_TRAIT_REQ4_TUPLE(TP, M) STRICT_TRAIT_REQ4_APPLY(TP, UNWRAP(M))
#define STRICT_TRAIT_REQ4_APPLY(TP, ...) STRICT_TRAIT_REQ4(TP, __VA_ARGS__)
#define STRICT_TRAIT_REQ4(TP, Ret, Name, Params)                               \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}->std::same_as<TYPE_SPEC(Ret)>;    \
  {&Impl<ALL_ARGS(TP)>::Name}->std::same_as<TYPE_SPEC(Ret) (*)(PARAM_TYPES(Params))>;

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
#define DUCK_STATIC_TRAIT_ITEM_3(TP, A, Name, Params)                          \
  DUCK_STATIC_TRAIT_ITEM_3_DISPATCH(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_ITEM_3_DISPATCH(TP, A, Name, Params)                \
  CAT(DUCK_STATIC_TRAIT_ITEM_3_KIND_, IS_PAREN(A))(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_ITEM_3_KIND_1(TP, A, Name, Params)                    \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}                        \
      ->std::same_as<TYPE_SPEC(A)>;
#define DUCK_STATIC_TRAIT_ITEM_3_KIND_0(TP, A, Name, Params)                    \
  CAT(DUCK_STATIC_TRAIT_ITEM_3_KIND_0_, IS_TEMPLATE(A))(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_ITEM_3_KIND_0_1(TP, A, Name, Params)                  \
  typename Impl<ALL_ARGS(TP)>::template Name<TEMPLATE_PLACEHOLDER_ARGS(Params)>;
#define DUCK_STATIC_TRAIT_ITEM_3_KIND_0_0(TP, A, Name, Params)                  \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}                        \
      ->std::same_as<TYPE_SPEC(A)>;

#define DUCK_STATIC_TRAIT_FUNC(TP, tuple)                                      \
  DUCK_STATIC_TRAIT_FUNC_I(TP, UNWRAP(tuple))
#define DUCK_STATIC_TRAIT_FUNC_I(TP, ...)                                      \
  DUCK_STATIC_TRAIT_FUNC_HELPER(TP, VA_COUNT(__VA_ARGS__), __VA_ARGS__)
#define DUCK_STATIC_TRAIT_FUNC_HELPER(TP, N, ...)                              \
  DUCK_STATIC_TRAIT_FUNC_II(TP, N, __VA_ARGS__)
#define DUCK_STATIC_TRAIT_FUNC_II(TP, N, ...)                                  \
  DUCK_STATIC_TRAIT_FUNC_##N(TP, __VA_ARGS__)

#define DUCK_STATIC_TRAIT_FUNC_2(TP, kword, Name)
#define DUCK_STATIC_TRAIT_FUNC_3(TP, A, Name, Params)                          \
  DUCK_STATIC_TRAIT_FUNC_3_DISPATCH(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_FUNC_3_DISPATCH(TP, A, Name, Params)                \
  CAT(DUCK_STATIC_TRAIT_FUNC_3_KIND_, IS_PAREN(A))(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_FUNC_3_KIND_1(TP, A, Name, Params)                   \
  FREE_FUNC4(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_FUNC_3_KIND_0(TP, A, Name, Params)                   \
  CAT(DUCK_STATIC_TRAIT_FUNC_3_KIND_0_, IS_TEMPLATE(A))(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_FUNC_3_KIND_0_1(TP, A, Name, Params) /* nothing */
#define DUCK_STATIC_TRAIT_FUNC_3_KIND_0_0(TP, A, Name, Params)                 \
  FREE_FUNC4(TP, A, Name, Params)

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
#define STRICT_STATIC_TRAIT_ITEM_3(TP, A, Name, Params)                        \
  STRICT_STATIC_TRAIT_ITEM_3_DISPATCH(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_ITEM_3_DISPATCH(TP, A, Name, Params)                \
  CAT(STRICT_STATIC_TRAIT_ITEM_3_KIND_, IS_PAREN(A))(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_ITEM_3_KIND_1(TP, A, Name, Params)                 \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}                        \
      ->std::same_as<TYPE_SPEC(A)>;
#define STRICT_STATIC_TRAIT_ITEM_3_KIND_0(TP, A, Name, Params)                 \
  CAT(STRICT_STATIC_TRAIT_ITEM_3_KIND_0_, IS_TEMPLATE(A))(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_ITEM_3_KIND_0_1(TP, A, Name, Params)               \
  typename Impl<ALL_ARGS(TP)>::template Name<TEMPLATE_PLACEHOLDER_ARGS(Params)>;
#define STRICT_STATIC_TRAIT_ITEM_3_KIND_0_0(TP, A, Name, Params)               \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}                        \
      ->std::same_as<TYPE_SPEC(A)>;

#define STRICT_STATIC_TRAIT_FUNC(TP, tuple)                                    \
  STRICT_STATIC_TRAIT_FUNC_I(TP, UNWRAP(tuple))
#define STRICT_STATIC_TRAIT_FUNC_I(TP, ...)                                    \
  STRICT_STATIC_TRAIT_FUNC_HELPER(TP, VA_COUNT(__VA_ARGS__), __VA_ARGS__)
#define STRICT_STATIC_TRAIT_FUNC_HELPER(TP, N, ...)                            \
  STRICT_STATIC_TRAIT_FUNC_II(TP, N, __VA_ARGS__)
#define STRICT_STATIC_TRAIT_FUNC_II(TP, N, ...)                                \
  STRICT_STATIC_TRAIT_FUNC_##N(TP, __VA_ARGS__)

#define STRICT_STATIC_TRAIT_FUNC_2(TP, kword, Name) /* nothing */
#define STRICT_STATIC_TRAIT_FUNC_3(TP, A, Name, Params)                        \
  STRICT_STATIC_TRAIT_FUNC_3_DISPATCH(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_FUNC_3_DISPATCH(TP, A, Name, Params)               \
  CAT(STRICT_STATIC_TRAIT_FUNC_3_KIND_, IS_PAREN(A))(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_FUNC_3_KIND_1(TP, A, Name, Params)                 \
  FREE_FUNC4(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_FUNC_3_KIND_0(TP, A, Name, Params)                 \
  CAT(STRICT_STATIC_TRAIT_FUNC_3_KIND_0_, IS_TEMPLATE(A))(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_FUNC_3_KIND_0_1(TP, A, Name, Params) /* nothing */
#define STRICT_STATIC_TRAIT_FUNC_3_KIND_0_0(TP, A, Name, Params)               \
  FREE_FUNC4(TP, A, Name, Params)

//--------------------------------------------------------------------
//  Main macros (Static Mixins Removed)
//--------------------------------------------------------------------

#define DuckTrait(NS, TP, ...)                                                 \
  namespace NS {                                                               \
  TEMPLATE_DECL(TP) struct Dyn;                                                \
  template <TYPENAME_LIST(TP)> struct Impl;                                    \
  template <TYPENAME_LIST(TP)>                                                 \
  concept Trait = requires(FIRST(TP) t) {                                      \
    FOR_EACH_WITH(DUCK_TRAIT_REQ4_TUPLE, TP, __VA_ARGS__)                      \
  };                                                                           \
  FOR_EACH_WITH(FREE_FUNC4_TUPLE, TP, __VA_ARGS__)                             \
  MIXIN_TEMPLATE_HEAD(TP) struct Mixin {                                       \
    FOR_EACH_WITH2(MIXIN_METHOD4_TUPLE, NS, TP, __VA_ARGS__)                   \
  };                                                                           \
  TEMPLATE_DECL(TP) struct VTable {                                            \
    FOR_EACH_WITH(VTABLE_MEMBER4_TUPLE, TP, __VA_ARGS__)                       \
  };                                                                           \
  template <TYPENAME_LIST(TP)>                                                 \
    requires Trait<ALL_ARGS(TP)>                                               \
  inline static const VTable ANGLE_EXTRA_ARGS(TP) vt = {                       \
      FOR_EACH_WITH(VT_ENTRY4_TUPLE, TP, __VA_ARGS__)};                        \
  TEMPLATE_DECL(TP) struct Dyn MIXIN_BASE(TP) {                                \
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

#define StaticDuckTrait(NS, TP, ...)                                           \
  namespace NS {                                                               \
  template <TYPENAME_LIST(TP)> struct Impl;                                    \
  template <TYPENAME_LIST(TP)>                                                 \
  concept Trait = requires(FIRST(TP) t) {                                      \
    FOR_EACH_WITH(DUCK_STATIC_TRAIT_ITEM, TP, __VA_ARGS__)                     \
  };                                                                           \
  FOR_EACH_WITH(DUCK_STATIC_TRAIT_FUNC, TP, __VA_ARGS__)                       \
  }

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
  MIXIN_TEMPLATE_HEAD(TP) struct Mixin {                                       \
    FOR_EACH_WITH2(MIXIN_METHOD4_TUPLE, NS, TP, __VA_ARGS__)                   \
  };                                                                           \
  TEMPLATE_DECL(TP) struct VTable {                                            \
    FOR_EACH_WITH(VTABLE_MEMBER4_TUPLE, TP, __VA_ARGS__)                       \
  };                                                                           \
  template <TYPENAME_LIST(TP)>                                                 \
    requires Trait<ALL_ARGS(TP)>                                               \
  inline static const VTable ANGLE_EXTRA_ARGS(TP) vt = {                       \
      FOR_EACH_WITH(VT_ENTRY4_TUPLE, TP, __VA_ARGS__)};                        \
  TEMPLATE_DECL(TP) struct Dyn MIXIN_BASE(TP) {                                \
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

//--------------------------------------------------------------------
//  Callable-trait helpers
//--------------------------------------------------------------------
#define IS_CALLABLE(x) CHECK(CAT(IS_CALLABLE_, x))
#define IS_CALLABLE_callable PROBE(~)

#define CALLABLE_TEMPLATE_NAME(A) CALLABLE_TEMPLATE_NAME_I(VA_COUNT(UNWRAP(A)), UNWRAP(A))
#define CALLABLE_TEMPLATE_NAME_I(N, ...) CALLABLE_TEMPLATE_NAME_II(N, __VA_ARGS__)
#define CALLABLE_TEMPLATE_NAME_II(N, ...) CALLABLE_TEMPLATE_NAME_##N(__VA_ARGS__)
#define CALLABLE_TEMPLATE_NAME_3(kind, TemplateName, AssocName) TemplateName

#define CALLABLE_ASSOC_NAME(A) CALLABLE_ASSOC_NAME_I(VA_COUNT(UNWRAP(A)), UNWRAP(A))
#define CALLABLE_ASSOC_NAME_I(N, ...) CALLABLE_ASSOC_NAME_II(N, __VA_ARGS__)
#define CALLABLE_ASSOC_NAME_II(N, ...) CALLABLE_ASSOC_NAME_##N(__VA_ARGS__)
#define CALLABLE_ASSOC_NAME_3(kind, TemplateName, AssocName) AssocName

#define FUNC_TEMPLATE_HEAD_CALLABLE(TP)                                        \
  FUNC_TEMPLATE_HEAD_CALLABLE_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define FUNC_TEMPLATE_HEAD_CALLABLE_I(N, ...) FUNC_TEMPLATE_HEAD_CALLABLE_II(N, __VA_ARGS__)
#define FUNC_TEMPLATE_HEAD_CALLABLE_II(N, ...) FUNC_TEMPLATE_HEAD_CALLABLE_##N(__VA_ARGS__)
#define FUNC_TEMPLATE_HEAD_CALLABLE_1(A) template <Trait A, typename F>
#define FUNC_TEMPLATE_HEAD_CALLABLE_2(A, B) template <typename B, Trait<B> A, typename F>
#define FUNC_TEMPLATE_HEAD_CALLABLE_3(A, B, C) template <typename B, typename C, Trait<B, C> A, typename F>

#undef STRICT_STATIC_TRAIT_ITEM_3_KIND_1
#undef STRICT_STATIC_TRAIT_FUNC_3_KIND_1
#undef DUCK_STATIC_TRAIT_ITEM_3_KIND_1
#undef DUCK_STATIC_TRAIT_FUNC_3_KIND_1

#define STRICT_STATIC_TRAIT_ITEM_3_KIND_1(TP, A, Name, Params)                 \
  CAT(STRICT_STATIC_TRAIT_ITEM_3_KIND_1_, IS_CALLABLE(FIRST(A)))(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_ITEM_3_KIND_1_0(TP, A, Name, Params)               \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}                        \
      ->std::same_as<TYPE_SPEC(A)>;
#define STRICT_STATIC_TRAIT_ITEM_3_KIND_1_1(TP, A, Name, Params)               \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params),                        \
                            ::gen_interface_detail::identity_callable{})}     \
      ->std::same_as<typename Impl<ALL_ARGS(TP)>::template                    \
                     CALLABLE_TEMPLATE_NAME(A)<                               \
                         typename Impl<ALL_ARGS(TP)>::CALLABLE_ASSOC_NAME(A)>>;

#define STRICT_STATIC_TRAIT_FUNC_3_KIND_1(TP, A, Name, Params)                 \
  CAT(STRICT_STATIC_TRAIT_FUNC_3_KIND_1_, IS_CALLABLE(FIRST(A)))(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_FUNC_3_KIND_1_0(TP, A, Name, Params)               \
  FREE_FUNC4(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_FUNC_3_KIND_1_1(TP, A, Name, Params)               \
  FUNC_TEMPLATE_HEAD_CALLABLE(TP)                                              \
    requires std::invocable<F&, typename Impl<ALL_ARGS(TP)>::                \
                                   CALLABLE_ASSOC_NAME(A)>                    \
  auto Name(FUNC_PARAMS(Params), F&& fn)                                       \
      -> typename Impl<ALL_ARGS(TP)>::template CALLABLE_TEMPLATE_NAME(A)<     \
          std::invoke_result_t<F&,                                            \
                               typename Impl<ALL_ARGS(TP)>::                  \
                                   CALLABLE_ASSOC_NAME(A)>> {                  \
    return Impl<ALL_ARGS(TP)>::Name(CALL_ARGS(Params), std::forward<F>(fn));   \
  }

#define DUCK_STATIC_TRAIT_ITEM_3_KIND_1(TP, A, Name, Params)                   \
  CAT(DUCK_STATIC_TRAIT_ITEM_3_KIND_1_, IS_CALLABLE(FIRST(A)))(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_ITEM_3_KIND_1_0(TP, A, Name, Params)                 \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}                        \
      ->std::same_as<TYPE_SPEC(A)>;
#define DUCK_STATIC_TRAIT_ITEM_3_KIND_1_1(TP, A, Name, Params)                 \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params),                        \
                            ::gen_interface_detail::identity_callable{})}     \
      ->std::same_as<typename Impl<ALL_ARGS(TP)>::template                    \
                     CALLABLE_TEMPLATE_NAME(A)<                               \
                         typename Impl<ALL_ARGS(TP)>::CALLABLE_ASSOC_NAME(A)>>;

#define DUCK_STATIC_TRAIT_FUNC_3_KIND_1(TP, A, Name, Params)                   \
  CAT(DUCK_STATIC_TRAIT_FUNC_3_KIND_1_, IS_CALLABLE(FIRST(A)))(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_FUNC_3_KIND_1_0(TP, A, Name, Params)                 \
  FREE_FUNC4(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_FUNC_3_KIND_1_1(TP, A, Name, Params)                 \
  FUNC_TEMPLATE_HEAD_CALLABLE(TP)                                              \
    requires std::invocable<F&, typename Impl<ALL_ARGS(TP)>::                \
                                   CALLABLE_ASSOC_NAME(A)>                    \
  auto Name(FUNC_PARAMS(Params), F&& fn)                                       \
      -> typename Impl<ALL_ARGS(TP)>::template CALLABLE_TEMPLATE_NAME(A)<     \
          std::invoke_result_t<F&,                                            \
                               typename Impl<ALL_ARGS(TP)>::                  \
                                   CALLABLE_ASSOC_NAME(A)>> {                  \
    return Impl<ALL_ARGS(TP)>::Name(CALL_ARGS(Params), std::forward<F>(fn));   \
  }

// Public surface:
//   hof((template, Mapped, (U)), map, (Self, fn(value_type)))
// lowers to the callable-aware internal form used by the working trait engine.
// The function-like argument stays after Self, so the declaration reads like a
// normal function signature while still generating a generic-callable overload.

#define fn(...) (fn, __VA_ARGS__)

#define HOF_SECOND_2(A, B) B
#define HOF_SECOND_3(A, B, C) B
#define HOF_SECOND_4(A, B, C, D) B
#define HOF_SECOND(P) HOF_SECOND_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define HOF_SECOND_I(N, ...) HOF_SECOND_II(N, __VA_ARGS__)
#define HOF_SECOND_II(N, ...) HOF_SECOND_##N(__VA_ARGS__)

#define HOF_FIRST_2(A, B) A
#define HOF_FIRST_3(A, B, C) A
#define HOF_FIRST_4(A, B, C, D) A
#define HOF_FIRST(P) HOF_FIRST_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define HOF_FIRST_I(N, ...) HOF_FIRST_II(N, __VA_ARGS__)
#define HOF_FIRST_II(N, ...) HOF_FIRST_##N(__VA_ARGS__)

#define HOF_TEMPLATE_NAME(Ret) HOF_TEMPLATE_NAME_I(VA_COUNT(UNWRAP(Ret)), UNWRAP(Ret))
#define HOF_TEMPLATE_NAME_I(N, ...) HOF_TEMPLATE_NAME_II(N, __VA_ARGS__)
#define HOF_TEMPLATE_NAME_II(N, ...) HOF_TEMPLATE_NAME_##N(__VA_ARGS__)
#define HOF_TEMPLATE_NAME_3(kind, TemplateName, AssocTuple) TemplateName

#define HOF_ARG_TYPE(Params) HOF_ARG_TYPE_I(VA_COUNT(UNWRAP(Params)), UNWRAP(Params))
#define HOF_ARG_TYPE_I(N, ...) HOF_ARG_TYPE_II(N, __VA_ARGS__)
#define HOF_ARG_TYPE_II(N, ...) HOF_ARG_TYPE_##N(__VA_ARGS__)
#define HOF_ARG_TYPE_2(Self, FnTuple) HOF_SECOND(FnTuple)

// Higher-order signature helper:
//   fn((ArgType), (type, RetType))
//   fn((ArgType), (template, TemplateName))
// The surrounding hof(...) item still declares the trait-level return shape.
#define fn(...) (fn, __VA_ARGS__)

#define HOF_IS_FN(x) CHECK(CAT(HOF_IS_FN_, x))
#define HOF_IS_FN_fn PROBE(~)

#define HOF_FIRST_2(A, B) A
#define HOF_FIRST_3(A, B, C) A
#define HOF_FIRST(P) HOF_FIRST_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define HOF_FIRST_I(N, ...) HOF_FIRST_II(N, __VA_ARGS__)
#define HOF_FIRST_II(N, ...) HOF_FIRST_##N(__VA_ARGS__)

#define HOF_SECOND_2(A, B) B
#define HOF_SECOND_3(A, B, C) B
#define HOF_SECOND(P) HOF_SECOND_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define HOF_SECOND_I(N, ...) HOF_SECOND_II(N, __VA_ARGS__)
#define HOF_SECOND_II(N, ...) HOF_SECOND_##N(__VA_ARGS__)

#define HOF_THIRD_3(A, B, C) C
#define HOF_THIRD(P) HOF_THIRD_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define HOF_THIRD_I(N, ...) HOF_THIRD_II(N, __VA_ARGS__)
#define HOF_THIRD_II(N, ...) HOF_THIRD_##N(__VA_ARGS__)

#define HOF_FN_ARG(Fn) HOF_SECOND(Fn)
#define HOF_FN_RET(Fn) HOF_THIRD(Fn)

#define HOF_ARG_NAME(Arg) TYPE_SPEC(Arg)
#define HOF_IS_VALUE_TYPE(x) CHECK(CAT(HOF_IS_VALUE_TYPE_, x))
#define HOF_IS_VALUE_TYPE_value_type PROBE(~)
#define HOF_ARG_EXPR(TP, Arg) CAT(HOF_ARG_EXPR_, HOF_IS_VALUE_TYPE(HOF_ARG_NAME(Arg)))(TP, Arg)
#define HOF_ARG_EXPR_0(TP, Arg) TYPE_SPEC(Arg)
#define HOF_ARG_EXPR_1(TP, Arg) typename Impl<ALL_ARGS(TP)>::value_type

#define HOF_RET_IS_TEMPLATE(Ret) CAT(HOF_RET_IS_TEMPLATE_, IS_PAREN(Ret))(Ret)
#define HOF_RET_IS_TEMPLATE_0(Ret) 0
#define HOF_RET_IS_TEMPLATE_1(Ret) IS_TEMPLATE(FIRST(Ret))

#define HOF_RET_TEMPLATE_NAME(Ret)                                             \
  HOF_RET_TEMPLATE_NAME_I(VA_COUNT(UNWRAP(Ret)), UNWRAP(Ret))
#define HOF_RET_TEMPLATE_NAME_I(N, ...) HOF_RET_TEMPLATE_NAME_II(N, __VA_ARGS__)
#define HOF_RET_TEMPLATE_NAME_II(N, ...) HOF_RET_TEMPLATE_NAME_##N(__VA_ARGS__)
#define HOF_RET_TEMPLATE_NAME_2(kind, TemplateName) TemplateName
#define HOF_RET_TEMPLATE_NAME_3(kind, TemplateName, AssocTuple) TemplateName

#define HOF_EXPECTED_RETURN_VALUE(TP, Ret)                                     \
  CAT(HOF_EXPECTED_RETURN_VALUE_, IS_PAREN(Ret))(TP, Ret)
#define HOF_EXPECTED_RETURN_VALUE_0(TP, Ret) TYPE_SPEC(Ret)
#define HOF_EXPECTED_RETURN_VALUE_1(TP, Ret)                                   \
  CAT(HOF_EXPECTED_RETURN_VALUE_1_, HOF_RET_IS_TEMPLATE(Ret))(TP, Ret)
#define HOF_EXPECTED_RETURN_VALUE_1_1(TP, Ret)                                 \
  typename Impl<ALL_ARGS(TP)>::template HOF_RET_TEMPLATE_NAME(Ret)<           \
      typename Impl<ALL_ARGS(TP)>::value_type>
#define HOF_EXPECTED_RETURN_VALUE_1_0(TP, Ret) TYPE_SPEC(Ret)

#define HOF_INVOKE_RESULT(F, TP)                                                \
  std::remove_cvref_t<                                                         \
      std::invoke_result_t<F &, typename Impl<ALL_ARGS(TP)>::value_type>>

#define HOF_WRAPPER_RETURN(TP, Ret, F)                                         \
  CAT(HOF_WRAPPER_RETURN_, IS_PAREN(Ret))(TP, Ret, F)
#define HOF_WRAPPER_RETURN_0(TP, Ret, F) TYPE_SPEC(Ret)
#define HOF_WRAPPER_RETURN_1(TP, Ret, F)                                       \
  CAT(HOF_WRAPPER_RETURN_1_, HOF_RET_IS_TEMPLATE(Ret))(TP, Ret, F)
#define HOF_WRAPPER_RETURN_1_1(TP, Ret, F)                                     \
  typename Impl<ALL_ARGS(TP)>::template HOF_RET_TEMPLATE_NAME(Ret)<           \
      HOF_INVOKE_RESULT(F, TP)>
#define HOF_WRAPPER_RETURN_1_0(TP, Ret, F) TYPE_SPEC(Ret)

#define HOF_WRAPPER_REQUIRES(TP, Ret, F)                                       \
  CAT(HOF_WRAPPER_REQUIRES_, IS_PAREN(Ret))(TP, Ret, F)
#define HOF_WRAPPER_REQUIRES_0(TP, Ret, F)                                     \
  std::same_as<HOF_INVOKE_RESULT(F, TP), TYPE_SPEC(Ret)>
#define HOF_WRAPPER_REQUIRES_1(TP, Ret, F)                                     \
  CAT(HOF_WRAPPER_REQUIRES_1_, HOF_RET_IS_TEMPLATE(Ret))(TP, Ret, F)
#define HOF_WRAPPER_REQUIRES_1_1(TP, Ret, F) true
#define HOF_WRAPPER_REQUIRES_1_0(TP, Ret, F)                                   \
  std::same_as<HOF_INVOKE_RESULT(F, TP), TYPE_SPEC(Ret)>

#define HOF_PARAM_TYPE(Fn) TYPE_SPEC(HOF_FN_ARG(Fn))

#define HOF_ITEM_KIND(TP, A, Name, Params)                                     \
  CAT(HOF_ITEM_KIND_, HOF_IS_FN(FIRST(A)))(TP, A, Name, Params)
#define HOF_ITEM_KIND_0(TP, A, Name, Params)                                   \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}                        \
      ->std::same_as<TYPE_SPEC(A)>;
#define HOF_ITEM_KIND_1(TP, A, Name, Params)                                   \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params),                        \
                            ::gen_interface_detail::identity_callable{})}     \
      ->std::same_as<HOF_EXPECTED_RETURN_VALUE(TP, HOF_FN_RET(A))>;

#define HOF_FUNC_KIND(TP, A, Name, Params)                                     \
  CAT(HOF_FUNC_KIND_, HOF_IS_FN(FIRST(A)))(TP, A, Name, Params)
#define HOF_FUNC_KIND_0(TP, A, Name, Params)                                   \
  FREE_FUNC4(TP, A, Name, Params)
#define HOF_FUNC_KIND_1(TP, A, Name, Params)                                   \
  template <Trait ALL_ARGS(TP), typename F>                                    \
    requires std::invocable<F &, HOF_ARG_EXPR(TP, HOF_FN_ARG(A))> &&            \
             HOF_WRAPPER_REQUIRES(TP, HOF_FN_RET(A), F)                        \
  auto Name(FUNC_PARAMS(Params), F &&fn) -> HOF_WRAPPER_RETURN(TP,           \
                                                               HOF_FN_RET(A), F) { \
    return Impl<ALL_ARGS(TP)>::Name(CALL_ARGS(Params), std::forward<F>(fn));   \
  }

#undef STRICT_STATIC_TRAIT_ITEM_3_KIND_1
#undef STRICT_STATIC_TRAIT_FUNC_3_KIND_1
#undef DUCK_STATIC_TRAIT_ITEM_3_KIND_1
#undef DUCK_STATIC_TRAIT_FUNC_3_KIND_1

#define STRICT_STATIC_TRAIT_ITEM_3_KIND_1(TP, A, Name, Params)                 \
  HOF_ITEM_KIND(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_FUNC_3_KIND_1(TP, A, Name, Params)                 \
  HOF_FUNC_KIND(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_ITEM_3_KIND_1(TP, A, Name, Params)                   \
  HOF_ITEM_KIND(TP, A, Name, Params)
#define DUCK_STATIC_TRAIT_FUNC_3_KIND_1(TP, A, Name, Params)                   \
  HOF_FUNC_KIND(TP, A, Name, Params)

// User-facing syntax:
//   hof((template, Mapped, (U)), map, (Self, fn((value_type), (template, Mapped))))
//   hof((type, bool), test, (Self, fn((value_type), (type, bool))))
#define hof(Ret, Name, Params)                                                 \
  ((fn, HOF_FN_ARG(HOF_SECOND(Params)), Ret), Name, (HOF_FIRST(Params)))

#define fn(Return, ...) (fn, Return, __VA_ARGS__)

#define IS_HOF_MARKER(x) CHECK(CAT(IS_HOF_MARKER_, x))
#define IS_HOF_MARKER_hof PROBE(~)

#define HOF_SECOND_2(A, B) B
#define HOF_SECOND_3(A, B, C) B
#define HOF_SECOND_4(A, B, C, D) B
#define HOF_SECOND_5(A, B, C, D, E) B
#define HOF_SECOND_6(A, B, C, D, E, F) B
#define HOF_SECOND_7(A, B, C, D, E, F, G) B
#define HOF_ITEM_FN(A) HOF_SECOND(A)
#define HOF_SECOND(P) HOF_SECOND_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define HOF_SECOND_I(N, ...) HOF_SECOND_II(N, __VA_ARGS__)
#define HOF_SECOND_II(N, ...) HOF_SECOND_##N(__VA_ARGS__)

#define HOF_LAST_2(A, B) B
#define HOF_LAST_3(A, B, C) C
#define HOF_LAST_4(A, B, C, D) D
#define HOF_LAST_5(A, B, C, D, E) E
#define HOF_LAST_6(A, B, C, D, E, F) F
#define HOF_LAST_7(A, B, C, D, E, F, G) G
#define HOF_LAST(P) HOF_LAST_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define HOF_LAST_I(N, ...) HOF_LAST_II(N, __VA_ARGS__)
#define HOF_LAST_II(N, ...) HOF_LAST_##N(__VA_ARGS__)

#define HOF_BUTLAST_2(A, B) (A)
#define HOF_BUTLAST_3(A, B, C) (A, B)
#define HOF_BUTLAST_4(A, B, C, D) (A, B, C)
#define HOF_BUTLAST_5(A, B, C, D, E) (A, B, C, D)
#define HOF_BUTLAST_6(A, B, C, D, E, F) (A, B, C, D, E)
#define HOF_BUTLAST_7(A, B, C, D, E, F, G) (A, B, C, D, E, F)
#define HOF_BUTLAST(P) HOF_BUTLAST_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define HOF_BUTLAST_I(N, ...) HOF_BUTLAST_II(N, __VA_ARGS__)
#define HOF_BUTLAST_II(N, ...) HOF_BUTLAST_##N(__VA_ARGS__)

#define HOF_FN_RET(Fn) HOF_SECOND(Fn)

#define HOF_FN_ARGS_2(Marker, Ret) ()
#define HOF_FN_ARGS_3(Marker, Ret, A1) (A1)
#define HOF_FN_ARGS_4(Marker, Ret, A1, A2) (A1, A2)
#define HOF_FN_ARGS_5(Marker, Ret, A1, A2, A3) (A1, A2, A3)
#define HOF_FN_ARGS_6(Marker, Ret, A1, A2, A3, A4) (A1, A2, A3, A4)
#define HOF_FN_ARGS_7(Marker, Ret, A1, A2, A3, A4, A5) (A1, A2, A3, A4, A5)
#define HOF_FN_ARGS(Fn) HOF_FN_ARGS_I(VA_COUNT(UNWRAP(Fn)), UNWRAP(Fn))
#define HOF_FN_ARGS_I(N, ...) HOF_FN_ARGS_II(N, __VA_ARGS__)
#define HOF_FN_ARGS_II(N, ...) HOF_FN_ARGS_##N(__VA_ARGS__)
#define HOF_FN_ARGS_EVAL(Fn) HOF_FN_ARGS(Fn)
#define HOF_FN_ARGS_UNWRAP(Fn) UNWRAP(HOF_FN_ARGS_EVAL(Fn))

#define HOF_ARG_NAME(Arg) TYPE_SPEC(Arg)
#define HOF_IS_VALUE_TYPE(x) CHECK(CAT(HOF_IS_VALUE_TYPE_, x))
#define HOF_IS_VALUE_TYPE_value_type PROBE(~)
#define HOF_ARG_EXPR(TP, Arg)                                                  \
  CAT(HOF_ARG_EXPR_, HOF_IS_VALUE_TYPE(HOF_ARG_NAME(Arg)))(TP, Arg)
#define HOF_ARG_EXPR_0(TP, Arg) TYPE_SPEC(Arg)
#define HOF_ARG_EXPR_1(TP, Arg) typename Impl<ALL_ARGS(TP)>::value_type

#define HOF_FN_ARG_TYPES_0(TP)
#define HOF_FN_ARG_TYPES_1(TP, A1) HOF_ARG_EXPR(TP, A1)
#define HOF_FN_ARG_TYPES_2(TP, A1, A2)                                         \
  HOF_ARG_EXPR(TP, A1), HOF_ARG_EXPR(TP, A2)
#define HOF_FN_ARG_TYPES_3(TP, A1, A2, A3)                                     \
  HOF_ARG_EXPR(TP, A1), HOF_ARG_EXPR(TP, A2), HOF_ARG_EXPR(TP, A3)
#define HOF_FN_ARG_TYPES_4(TP, A1, A2, A3, A4)                                 \
  HOF_ARG_EXPR(TP, A1), HOF_ARG_EXPR(TP, A2), HOF_ARG_EXPR(TP, A3),           \
      HOF_ARG_EXPR(TP, A4)
#define HOF_FN_ARG_TYPES_5(TP, A1, A2, A3, A4, A5)                               HOF_ARG_EXPR(TP, A1), HOF_ARG_EXPR(TP, A2), HOF_ARG_EXPR(TP, A3),                 HOF_ARG_EXPR(TP, A4), HOF_ARG_EXPR(TP, A5)
#define HOF_FN_ARG_TYPES_6(TP, A1, A2, A3, A4, A5, A6)                           HOF_ARG_EXPR(TP, A1), HOF_ARG_EXPR(TP, A2), HOF_ARG_EXPR(TP, A3),                 HOF_ARG_EXPR(TP, A4), HOF_ARG_EXPR(TP, A5), HOF_ARG_EXPR(TP, A6)
#define HOF_FN_ARG_TYPES_7(TP, A1, A2, A3, A4, A5, A6, A7)                       HOF_ARG_EXPR(TP, A1), HOF_ARG_EXPR(TP, A2), HOF_ARG_EXPR(TP, A3),                 HOF_ARG_EXPR(TP, A4), HOF_ARG_EXPR(TP, A5), HOF_ARG_EXPR(TP, A6),             HOF_ARG_EXPR(TP, A7)
#define HOF_FN_ARG_TYPES(Fn, TP)                                                 HOF_FN_ARG_TYPES_I(TP, VA_COUNT(UNWRAP(HOF_FN_ARGS(Fn))),                                         UNWRAP(HOF_FN_ARGS(Fn)))
#define HOF_FN_ARG_TYPES_I(TP, N, ...) HOF_FN_ARG_TYPES_II(TP, N, __VA_ARGS__)
#define HOF_FN_ARG_TYPES_II(TP, N, ...) HOF_FN_ARG_TYPES_##N(TP, __VA_ARGS__)

#define HOF_RET_IS_TEMPLATE(Ret) CAT(HOF_RET_IS_TEMPLATE_, IS_PAREN(Ret))(Ret)
#define HOF_RET_IS_TEMPLATE_0(Ret) 0
#define HOF_RET_IS_TEMPLATE_1(Ret) IS_TEMPLATE(FIRST(Ret))

#define HOF_PROBE_RETURN_TYPE(TP, Ret)                                         \
  CAT(HOF_PROBE_RETURN_TYPE_, HOF_RET_IS_TEMPLATE(Ret))(TP, Ret)
#define HOF_PROBE_RETURN_TYPE_0(TP, Ret) TYPE_SPEC(Ret)
#define HOF_PROBE_RETURN_TYPE_1(TP, Ret)                                       \
  typename Impl<ALL_ARGS(TP)>::value_type

#define HOF_EXPECTED_RETURN_TYPE(TP, Ret, Fn)                                  \
  CAT(HOF_EXPECTED_RETURN_TYPE_, HOF_RET_IS_TEMPLATE(Ret))(TP, Ret, Fn)
#define HOF_EXPECTED_RETURN_TYPE_0(TP, Ret, Fn) TYPE_SPEC(Ret)
#define HOF_EXPECTED_RETURN_TYPE_1(TP, Ret, Fn)                                \
  typename Impl<ALL_ARGS(TP)>::template Mapped<                               \
      HOF_PROBE_RETURN_TYPE(TP, Ret)>

#define HOF_WRAPPER_RETURN(TP, Ret, Fn, F)                                     \
  CAT(HOF_WRAPPER_RETURN_, HOF_RET_IS_TEMPLATE(Ret))(TP, Ret, Fn, F)
#define HOF_WRAPPER_RETURN_0(TP, Ret, Fn, F) TYPE_SPEC(Ret)
#define HOF_WRAPPER_RETURN_1(TP, Ret, Fn, F)                                   \
  typename Impl<ALL_ARGS(TP)>::template Mapped<                                \
      std::remove_cvref_t<std::invoke_result_t<F &,                            \
                                               HOF_FN_ARG_TYPES(Fn, TP)>>>     

#define HOF_WRAPPER_REQUIRES(TP, Ret, Fn, F)                                   \
  CAT(HOF_WRAPPER_REQUIRES_, HOF_RET_IS_TEMPLATE(Ret))(TP, Ret, Fn, F)
#define HOF_WRAPPER_REQUIRES_0(TP, Ret, Fn, F)                                 \
  std::same_as<std::remove_cvref_t<std::invoke_result_t<F &,                  \
                                                        HOF_FN_ARG_TYPES(Fn, TP)>>, \
               TYPE_SPEC(Ret)>
#define HOF_WRAPPER_REQUIRES_1(TP, Ret, Fn, F) true

#define HOF_FUNC_TEMPLATE_HEAD(TP)                                             \
  HOF_FUNC_TEMPLATE_HEAD_I(VA_COUNT(UNWRAP(TP)), UNWRAP(TP))
#define HOF_FUNC_TEMPLATE_HEAD_I(N, ...) HOF_FUNC_TEMPLATE_HEAD_II(N, __VA_ARGS__)
#define HOF_FUNC_TEMPLATE_HEAD_II(N, ...) HOF_FUNC_TEMPLATE_HEAD_##N(__VA_ARGS__)
#define HOF_FUNC_TEMPLATE_HEAD_1(A) template <Trait A, typename F>
#define HOF_FUNC_TEMPLATE_HEAD_2(A, B) template <typename B, Trait<B> A, typename F>
#define HOF_FUNC_TEMPLATE_HEAD_3(A, B, C)                                      \
  template <typename B, typename C, Trait<B, C> A, typename F>

#define HOF_ITEM_CONCEPT(TP, A, Name, Params)                                  \
  {Impl<ALL_ARGS(TP)>::Name(                                                   \
       TUPLE_TO_DECLVALS(Params),                                              \
       ::gen_interface_detail::probe_callable<                                 \
           HOF_PROBE_RETURN_TYPE(TP, HOF_FN_RET(HOF_ITEM_FN(A))),                           \
           HOF_FN_ARG_TYPES(HOF_ITEM_FN(A), TP)>{})};

#define HOF_FUNC_WRAPPER(TP, A, Name, Params)                                  \
  HOF_FUNC_TEMPLATE_HEAD(TP)                                                   \
    requires std::invocable<F &, HOF_FN_ARG_TYPES(HOF_ITEM_FN(A), TP)>                    \
  auto Name(FUNC_PARAMS(Params), F &&fn)                                       \
      -> HOF_WRAPPER_RETURN(TP, HOF_FN_RET(HOF_ITEM_FN(A)), HOF_ITEM_FN(A), F) {                         \
    return Impl<ALL_ARGS(TP)>::Name(CALL_ARGS(Params), std::forward<F>(fn));   \
  }

// -----------------------------------------------------------------------------
//  Extend strict static trait parsing for fn(...) higher-order items
// -----------------------------------------------------------------------------

#undef STRICT_STATIC_TRAIT_ITEM_3_KIND_1
#undef STRICT_STATIC_TRAIT_FUNC_3_KIND_1

#define STRICT_STATIC_TRAIT_ITEM_3_KIND_1(TP, A, Name, Params)                 \
  CAT(STRICT_STATIC_TRAIT_ITEM_3_KIND_1_, IS_HOF_MARKER(FIRST(A)))(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_ITEM_3_KIND_1_0(TP, A, Name, Params)               \
  {Impl<ALL_ARGS(TP)>::Name(TUPLE_TO_DECLVALS(Params))}                        \
      ->std::same_as<TYPE_SPEC(A)>;
#define STRICT_STATIC_TRAIT_ITEM_3_KIND_1_1(TP, A, Name, Params)               \
  HOF_ITEM_CONCEPT(TP, A, Name, Params)

#define STRICT_STATIC_TRAIT_FUNC_3_KIND_1(TP, A, Name, Params)                 \
  CAT(STRICT_STATIC_TRAIT_FUNC_3_KIND_1_, IS_HOF_MARKER(FIRST(A)))(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_FUNC_3_KIND_1_0(TP, A, Name, Params)               \
  FREE_FUNC4(TP, A, Name, Params)
#define STRICT_STATIC_TRAIT_FUNC_3_KIND_1_1(TP, A, Name, Params)               \
  HOF_FUNC_WRAPPER(TP, A, Name, Params)

// Convenience wrapper: user writes hof(Name, (Self, ...ordinary...), fn(ReturnSpec, Args...))
#define hof(Name, Params, Fn)                                                 \
  ((hof, Fn), Name, Params)

// Higher-order signature markers.
// The inner fn helper is purely a DSL marker here; the generated wrapper
// forwards a callable object and leaves the implementation to type-check it.
#define fn(Return, ...) ((fn, Return, __VA_ARGS__))
#define hof(Return, Name, Params) (hof, Return, Name, Params)

#define HOF_IS_HOF(x) CHECK(CAT(HOF_IS_HOF_, x))
#define HOF_IS_HOF_hof PROBE(~)

#define SECOND_2(A, B) B
#define SECOND_3(A, B, ...) B
#define SECOND(P) SECOND_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define SECOND_I(N, ...) SECOND_II(N, __VA_ARGS__)
#define SECOND_II(N, ...) SECOND_##N(__VA_ARGS__)
#define THIRD_3(A, B, C) C
#define THIRD(P) THIRD_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define THIRD_I(N, ...) THIRD_II(N, __VA_ARGS__)
#define THIRD_II(N, ...) THIRD_##N(__VA_ARGS__)

// -----------------------------------------------------------------------------
// Minimal hof-aware static_trait override
// -----------------------------------------------------------------------------

#undef static_trait
#define static_trait(Name, TP, MethodsTuple)                                   \
  namespace Name {                                                             \
  template <TYPENAME_LIST(TP)> struct Impl;                                    \
  template <TYPENAME_LIST(TP)>                                                 \
  concept Trait = requires {                                                   \
    FOR_EACH_WITH(STATIC_TRAIT_REQ_ITEM, TP, UNWRAP_I MethodsTuple)            \
  };                                                                           \
  FOR_EACH_WITH(STATIC_TRAIT_FUNC_ITEM, TP, UNWRAP_I MethodsTuple)             \
  }

#define STATIC_TRAIT_REQ_ITEM(TP, Item) STATIC_TRAIT_REQ_ITEM_I(TP, UNWRAP(Item))
#define STATIC_TRAIT_REQ_ITEM_I(TP, ...)                                       \
  STATIC_TRAIT_REQ_ITEM_DISPATCH(TP, VA_COUNT(__VA_ARGS__), __VA_ARGS__)
#define STATIC_TRAIT_REQ_ITEM_DISPATCH(TP, N, ...)                             \
  CAT(STATIC_TRAIT_REQ_ITEM_, N)(TP, __VA_ARGS__)

#define STATIC_TRAIT_REQ_ITEM_2(TP, Kind, Name)                                \
  STATIC_TRAIT_REQ_ITEM_2_DISPATCH(TP, Kind, Name)
#define STATIC_TRAIT_REQ_ITEM_2_DISPATCH(TP, Kind, Name)                      \
  CAT(STATIC_TRAIT_REQ_ITEM_2_KIND_, IS_TEMPLATE(Kind))(TP, Kind, Name)
#define STATIC_TRAIT_REQ_ITEM_2_KIND_1(TP, Kind, Name)                        \
  typename Impl<ALL_ARGS(TP)>::template Name<void>;
#define STATIC_TRAIT_REQ_ITEM_2_KIND_0(TP, Kind, Name)                        \
  typename Impl<ALL_ARGS(TP)>::Name;

#define STATIC_TRAIT_REQ_ITEM_3(TP, Kind, Name, Params)                        \
  STATIC_TRAIT_REQ_ITEM_3_DISPATCH(TP, Kind, Name, Params)
#define STATIC_TRAIT_REQ_ITEM_3_DISPATCH(TP, Kind, Name, Params)              \
  CAT(STATIC_TRAIT_REQ_ITEM_3_KIND_, IS_TEMPLATE(Kind))(TP, Kind, Name, Params)
#define STATIC_TRAIT_REQ_ITEM_3_KIND_1(TP, Kind, Name, Params)                \
  typename Impl<ALL_ARGS(TP)>::template Name<void>;
#define STATIC_TRAIT_REQ_ITEM_3_KIND_0(TP, Kind, Name, Params)                \
  /* Regular static function items are not used by the hof layer. */

#define STATIC_TRAIT_REQ_ITEM_4(TP, Kind, Return, Name, Params)                \
  STATIC_TRAIT_REQ_ITEM_4_DISPATCH(TP, Kind, Return, Name, Params)
#define STATIC_TRAIT_REQ_ITEM_4_DISPATCH(TP, Kind, Return, Name, Params)      \
  CAT(STATIC_TRAIT_REQ_ITEM_4_KIND_, HOF_IS_HOF(Kind))(TP, Kind, Return, Name, Params)
#define STATIC_TRAIT_REQ_ITEM_4_KIND_1(TP, Kind, Return, Name, Params)        \
  /* The actual callable validation happens through the generated wrapper. */
#define STATIC_TRAIT_REQ_ITEM_4_KIND_0(TP, Kind, Return, Name, Params)        \
  /* not hof */

#define STATIC_TRAIT_FUNC_ITEM(TP, Item) STATIC_TRAIT_FUNC_ITEM_I(TP, UNWRAP(Item))
#define STATIC_TRAIT_FUNC_ITEM_I(TP, ...)                                      \
  STATIC_TRAIT_FUNC_ITEM_DISPATCH(TP, VA_COUNT(__VA_ARGS__), __VA_ARGS__)
#define STATIC_TRAIT_FUNC_ITEM_DISPATCH(TP, N, ...)                           \
  CAT(STATIC_TRAIT_FUNC_ITEM_, N)(TP, __VA_ARGS__)

#define STATIC_TRAIT_FUNC_ITEM_2(TP, Kind, Name)                               \
  /* no wrapper */
#define STATIC_TRAIT_FUNC_ITEM_3(TP, Kind, Name, Params)                       \
  /* no wrapper */

#define STATIC_TRAIT_FUNC_ITEM_4(TP, Kind, Return, Name, Params)               \
  STATIC_TRAIT_FUNC_ITEM_4_DISPATCH(TP, Kind, Return, Name, Params)
#define STATIC_TRAIT_FUNC_ITEM_4_DISPATCH(TP, Kind, Return, Name, Params)     \
  CAT(STATIC_TRAIT_FUNC_ITEM_4_KIND_, HOF_IS_HOF(Kind))(TP, Kind, Return, Name, Params)
#define STATIC_TRAIT_FUNC_ITEM_4_KIND_1(TP, Kind, Return, Name, Params)       \
  STATIC_HOF_FUNC(TP, Return, Name, Params)
#define STATIC_TRAIT_FUNC_ITEM_4_KIND_0(TP, Kind, Return, Name, Params)       \
  /* not hof */

#define STATIC_HOF_PARAMS_COUNT(...) VA_COUNT(__VA_ARGS__)

#define STATIC_HOF_FN(P) STATIC_HOF_FN_I(UNWRAP(P))
#define STATIC_HOF_FN_I(...) STATIC_HOF_FN_##VA_COUNT(__VA_ARGS__)(__VA_ARGS__)

// Direct helpers for parameter tuples of the form:
//   (Self, fn(...))
//   (Self, Self, fn(...))
#define STATIC_HOF_FUNC(TP, Return, Name, Params)                              \
  STATIC_HOF_FUNC_DISPATCH(TP, Return, Name, Params)
#define STATIC_HOF_FUNC_DISPATCH(TP, Return, Name, Params)                     \
  CAT(STATIC_HOF_FUNC_, VA_COUNT(UNWRAP(Params)))(TP, Return, Name, Params)

#define STATIC_HOF_FUNC_2(TP, Return, Name, Params)                            \
  template <TYPENAME_LIST(TP), typename F>                                     \
  decltype(auto) Name(TYPE_SPEC(FIRST(Params)) a1, F &&fn) {                   \
    return Impl<ALL_ARGS(TP)>::Name(a1, std::forward<F>(fn));                  \
  }

#define STATIC_HOF_FUNC_3(TP, Return, Name, Params)                            \
  template <TYPENAME_LIST(TP), typename F>                                     \
  decltype(auto) Name(TYPE_SPEC(FIRST(Params)) a1,                             \
                      TYPE_SPEC(SECOND(Params)) a2, F &&fn) {                  \
    return Impl<ALL_ARGS(TP)>::Name(a1, a2, std::forward<F>(fn));              \
  }

#define STATIC_HOF_FUNC_4(TP, Return, Name, Params)                            \
  /* not needed for current examples */


#endif // TRAIT_HOF_AFTER_SELF_NEW_HPP
