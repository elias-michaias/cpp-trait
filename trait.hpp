// gen_interface.h
#ifndef TRAIT_GENERATION_H
#define TRAIT_GENERATION_H

#include <type_traits>
#include <utility>
#include <variant>

namespace gen_interface_detail {

template <class Receiver, class T>
constexpr decltype(auto) receiver_from(void *p) {
  if constexpr (std::is_pointer_v<Receiver>)
    return static_cast<Receiver>(p);
  else
    return *static_cast<std::remove_reference_t<Receiver> *>(p);
}

template <class Receiver, class Member>
constexpr decltype(auto) forward_member(Receiver &&receiver, Member member) {
  using ReceiverT = std::remove_reference_t<Receiver>;
  if constexpr (std::is_pointer_v<ReceiverT>)
    return &(receiver->*member);
  else
    return (std::forward<Receiver>(receiver).*member);
}

template <class Receiver>
constexpr decltype(auto) iterable_from(Receiver &&receiver) {
  using ReceiverT = std::remove_reference_t<Receiver>;
  if constexpr (std::is_pointer_v<ReceiverT>)
    return *receiver;
  else
    return (std::forward<Receiver>(receiver));
}

template <class Receiver, class Elem>
constexpr decltype(auto) iter_receiver(Elem &elem) {
  if constexpr (std::is_pointer_v<Receiver>)
    return &elem;
  else
    return (elem);
}

template <class Receiver, class Elem>
constexpr decltype(auto) variant_receiver(Elem &elem) {
  if constexpr (std::is_pointer_v<Receiver>)
    return &elem;
  else
    return (elem);
}

template <class Receiver>
constexpr decltype(auto) deref_forward(Receiver &&receiver) {
  using ReceiverT = std::remove_reference_t<Receiver>;
  if constexpr (std::is_pointer_v<ReceiverT>)
    return &(**receiver);
  else
    return (*std::forward<Receiver>(receiver));
}

template <class Receiver, class Accessor>
constexpr decltype(auto) accessor_forward(Receiver &&receiver, Accessor &&accessor) {
  using ReceiverT = std::remove_reference_t<Receiver>;
  if constexpr (std::is_pointer_v<ReceiverT>)
    return &std::forward<Accessor>(accessor)(*receiver);
  else
    return std::forward<Accessor>(accessor)(std::forward<Receiver>(receiver));
}

template <class Receiver>
constexpr bool optional_has_value(Receiver &&receiver) {
  using ReceiverT = std::remove_reference_t<Receiver>;
  if constexpr (std::is_pointer_v<ReceiverT>)
    return static_cast<bool>(*receiver);
  else
    return static_cast<bool>(receiver);
}

template <class Receiver, class PtrMember, class CountMember>
constexpr auto ptr_range_from(Receiver &&receiver, PtrMember ptr_member,
                              CountMember count_member) {
  using ReceiverT = std::remove_reference_t<Receiver>;
  if constexpr (std::is_pointer_v<ReceiverT>)
    return std::pair{receiver->*ptr_member, receiver->*count_member};
  else
    return std::pair{receiver.*ptr_member, receiver.*count_member};
}

template <class Receiver, class Visitor, class Func, class RetTag>
constexpr decltype(auto) union_visit(Receiver &&receiver, Visitor &&visitor,
                                     RetTag ret_tag, Func &&func) {
  using ReceiverT = std::remove_reference_t<Receiver>;
  if constexpr (std::is_pointer_v<ReceiverT>)
    return std::forward<Visitor>(visitor)(*receiver, ret_tag,
                                          std::forward<Func>(func));
  else
    return std::forward<Visitor>(visitor)(std::forward<Receiver>(receiver),
                                          ret_tag, std::forward<Func>(func));
}

template <class Ret>
constexpr void iterate_each_requires_void() {
  static_assert(std::is_void_v<Ret>,
                "iterate without a reducer only supports void-returning trait "
                "methods");
}

template <class Ret>
constexpr void optional_requires_fallback() {
  static_assert(std::is_void_v<Ret>,
                "optional without a fallback only supports void-returning trait "
                "methods");
}

template <class Iterable, class Acc, class Func>
constexpr auto iterate_reduce(Iterable &&iterable, Acc acc, Func &&func) {
  for (auto &elem : iterable)
    acc = func(std::move(acc), elem);
  return acc;
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
#define PP_CHECK_N(x, n, ...) n
#define PP_CHECK(...) PP_CHECK_N(__VA_ARGS__, 0)
#define PP_PROBE(...) ~, 1
#define PP_IS_PAREN(x) PP_CHECK(PP_IS_PAREN_PROBE x)
#define PP_IS_PAREN_PROBE(...) PP_PROBE(__VA_ARGS__)

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

#define FOR_EACH_WITH3(macro, data1, data2, data3, ...)                        \
  __VA_OPT__(EXPAND(FEWH3(macro, data1, data2, data3, __VA_ARGS__)))
#define FEWH3(macro, data1, data2, data3, a1, ...)                             \
  macro(data1, data2, data3, a1)                                               \
      __VA_OPT__(FEWA3 PARENS(macro, data1, data2, data3, __VA_ARGS__))
#define FEWA3() FEWH3

#define FOR_EACH_WITH4(macro, data1, data2, data3, data4, ...)                 \
  __VA_OPT__(EXPAND(FEWH4(macro, data1, data2, data3, data4, __VA_ARGS__)))
#define FEWH4(macro, data1, data2, data3, data4, a1, ...)                      \
  macro(data1, data2, data3, data4, a1)                                        \
      __VA_OPT__(FEWA4 PARENS(macro, data1, data2, data3, data4, __VA_ARGS__))
#define FEWA4() FEWH4

//--------------------------------------------------------------------
//  Arity / unwrap
//--------------------------------------------------------------------
#define VA_COUNT(...) VA_COUNT_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define VA_COUNT_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
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
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//  Mixin helpers (instance-method forwarding for non-static traits)
//  C++23+: deducing this, no CRTP.
//  C++20 : CRTP fallback.
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
#define MIXIN_METHOD_EXTRA_PARAMS_2(S, T1) , T1 p1
#define MIXIN_METHOD_EXTRA_PARAMS_3(S, T1, T2) , T1 p1, T2 p2
#define MIXIN_METHOD_EXTRA_PARAMS_4(S, T1, T2, T3) , T1 p1, T2 p2, T3 p3
#define MIXIN_METHOD_EXTRA_PARAMS_5(S, T1, T2, T3, T4) , T1 p1, T2 p2, T3 p3, T4 p4

#define MIXIN_METHOD4_TUPLE(NS, TP, M) MIXIN_METHOD4_APPLY(NS, TP, UNWRAP(M))
#define MIXIN_METHOD4_APPLY(NS, TP, ...) MIXIN_METHOD4(TP, NS, __VA_ARGS__)
#define MIXIN_METHOD4(TP, NS, Ret, Name, Params)                               \
  Ret Name(this auto &self MIXIN_METHOD_EXTRA_PARAMS(Params)) {                     \
    if constexpr (requires {                                                   \
                    ::NS::Name ANGLE_EXTRA_ARGS(TP)(self CALL_EXTRA_ARGS(Params)); \
                  }) {                                                         \
      if constexpr (std::is_void_v<Ret>) {                                     \
        ::NS::Name ANGLE_EXTRA_ARGS(TP)(self CALL_EXTRA_ARGS(Params));         \
      } else {                                                                 \
        return ::NS::Name ANGLE_EXTRA_ARGS(TP)(self CALL_EXTRA_ARGS(Params));  \
      }                                                                        \
    } else {                                                                   \
      if constexpr (std::is_void_v<Ret>) {                                     \
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

#define MIXIN_METHOD_PARAMS(P)                                          \
  MIXIN_METHOD_PARAMS_I(VA_COUNT(UNWRAP(P)), UNWRAP(P))
#define MIXIN_METHOD_PARAMS_I(N, ...)                                    \
  MIXIN_METHOD_PARAMS_II(N, __VA_ARGS__)
#define MIXIN_METHOD_PARAMS_II(N, ...)                                   \
  MIXIN_METHOD_PARAMS_##N(__VA_ARGS__)
#define MIXIN_METHOD_PARAMS_1(S)
#define MIXIN_METHOD_PARAMS_2(S, T1) T1 p1
#define MIXIN_METHOD_PARAMS_3(S, T1, T2) T1 p1, T2 p2
#define MIXIN_METHOD_PARAMS_4(S, T1, T2, T3) T1 p1, T2 p2, T3 p3
#define MIXIN_METHOD_PARAMS_5(S, T1, T2, T3, T4) T1 p1, T2 p2, T3 p3, T4 p4

#define MIXIN_METHOD4_TUPLE(NS, TP, M) MIXIN_METHOD4_APPLY(NS, TP, UNWRAP(M))
#define MIXIN_METHOD4_APPLY(NS, TP, ...) MIXIN_METHOD4(TP, NS, __VA_ARGS__)
#define MIXIN_METHOD4(TP, NS, Ret, Name, Params)                               \
  Ret Name(MIXIN_METHOD_PARAMS(Params)) {                                      \
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

#define DUCK_STATIC_TRAIT_FUNC_2(TP, kword, Name)
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
//  Derived impl helpers
//--------------------------------------------------------------------
#define DERIVE_WRAPPER_TEMPLATE_DECL(ArgsTuple)                                 \
  DERIVE_WRAPPER_TEMPLATE_DECL_I(VA_COUNT(UNWRAP(ArgsTuple)), UNWRAP(ArgsTuple))
#define DERIVE_WRAPPER_TEMPLATE_DECL_I(N, ...)                                  \
  DERIVE_WRAPPER_TEMPLATE_DECL_II(N, __VA_ARGS__)
#define DERIVE_WRAPPER_TEMPLATE_DECL_II(N, ...)                                 \
  DERIVE_WRAPPER_TEMPLATE_DECL_##N(__VA_ARGS__)
#define DERIVE_WRAPPER_TEMPLATE_DECL_1(_SelfArg) template <typename TraitDerived>
#define DERIVE_WRAPPER_TEMPLATE_DECL_2(_SelfArg, A1)                            \
  template <typename TraitDerived, auto A1>
#define DERIVE_WRAPPER_TEMPLATE_DECL_3(_SelfArg, A1, A2)                        \
  template <typename TraitDerived, auto A1, auto A2>

#define DERIVE_WRAPPER_TYPE_ARGS(ArgsTuple)                                     \
  DERIVE_WRAPPER_TYPE_ARGS_I(VA_COUNT(UNWRAP(ArgsTuple)), UNWRAP(ArgsTuple))
#define DERIVE_WRAPPER_TYPE_ARGS_I(N, ...) DERIVE_WRAPPER_TYPE_ARGS_II(N, __VA_ARGS__)
#define DERIVE_WRAPPER_TYPE_ARGS_II(N, ...) DERIVE_WRAPPER_TYPE_ARGS_##N(__VA_ARGS__)
#define DERIVE_WRAPPER_TYPE_ARGS_1(_SelfArg) TraitDerived
#define DERIVE_WRAPPER_TYPE_ARGS_2(_SelfArg, A1) TraitDerived, A1
#define DERIVE_WRAPPER_TYPE_ARGS_3(_SelfArg, A1, A2) TraitDerived, A1, A2

#define DERIVE_TARGET_TEMPLATE(NS, Target, ArgsTuple)                          \
  DERIVE_TARGET_TEMPLATE_I(PP_IS_PAREN(Target), NS, Target, ArgsTuple)
#define DERIVE_TARGET_TEMPLATE_I(IsParen, NS, Target, ArgsTuple)               \
  DERIVE_TARGET_TEMPLATE_II(IsParen, NS, Target, ArgsTuple)
#define DERIVE_TARGET_TEMPLATE_II(IsParen, NS, Target, ArgsTuple)              \
  DERIVE_TARGET_TEMPLATE_##IsParen(NS, Target, ArgsTuple)
#define DERIVE_TARGET_TEMPLATE_0(NS, Target, ArgsTuple)                        \
  DERIVE_WRAPPER_TEMPLATE_DECL(ArgsTuple)                                      \
    requires ::NS::Trait<TraitDerived>
#define DERIVE_TARGET_TEMPLATE_1(NS, Target, ArgsTuple)                        \
  template <typename TraitDerived>                                             \
    requires DERIVE_TARGET_REQUIRES(Target, TraitDerived)

#define DERIVE_TARGET_SELF(Target, ArgsTuple)                                  \
  DERIVE_TARGET_SELF_I(PP_IS_PAREN(Target), Target, ArgsTuple)
#define DERIVE_TARGET_SELF_I(IsParen, Target, ArgsTuple)                       \
  DERIVE_TARGET_SELF_II(IsParen, Target, ArgsTuple)
#define DERIVE_TARGET_SELF_II(IsParen, Target, ArgsTuple)                      \
  DERIVE_TARGET_SELF_##IsParen(Target, ArgsTuple)
#define DERIVE_TARGET_SELF_0(Target, ArgsTuple)                                \
  Target<DERIVE_WRAPPER_TYPE_ARGS(ArgsTuple)>
#define DERIVE_TARGET_SELF_1(Target, ArgsTuple) TraitDerived

#define DERIVE_TARGET_REQUIRES(Target, SelfType)                                  \
  DERIVE_TARGET_REQUIRES_I(SelfType, UNWRAP(Target))
#define DERIVE_TARGET_REQUIRES_I(SelfType, ...)                                \
  DERIVE_TARGET_REQUIRES_II(SelfType, __VA_ARGS__)
#define DERIVE_TARGET_REQUIRES_II(SelfType, Kind, ...)                         \
  DERIVE_TARGET_REQUIRES_##Kind(SelfType, __VA_ARGS__)
#define DERIVE_TARGET_REQUIRES_requires(SelfType, ...)                         \
  true FOR_EACH_WITH(DERIVE_TARGET_REQUIRES_AND, SelfType, __VA_ARGS__)
#define DERIVE_TARGET_REQUIRES_AND(SelfType, Concept) && Concept<SelfType>

#define DERIVE_MEMBER_METHOD4_TUPLE(NS, Field, M)                              \
  DERIVE_MEMBER_METHOD4_APPLY(NS, Field, UNWRAP(M))
#define DERIVE_MEMBER_METHOD4_APPLY(NS, Field, ...)                            \
  DERIVE_MEMBER_METHOD4(NS, Field, __VA_ARGS__)
#define DERIVE_MEMBER_METHOD4(NS, Field, Ret, Name, Params)                    \
  static Ret Name(FUNC_PARAMS(Params)) {                                       \
    if constexpr (std::is_void_v<Ret>) {                                       \
      ::NS::Name(::gen_interface_detail::forward_member(self, &Self::Field)    \
                     CALL_EXTRA_ARGS(Params));                                  \
    } else {                                                                   \
      return ::NS::Name(::gen_interface_detail::forward_member(self,           \
                                                               &Self::Field)   \
                            CALL_EXTRA_ARGS(Params));                           \
    }                                                                          \
  }

#define DERIVE_DEREF_METHOD4_TUPLE(NS, M) DERIVE_DEREF_METHOD4_APPLY(NS, UNWRAP(M))
#define DERIVE_DEREF_METHOD4_APPLY(NS, ...) DERIVE_DEREF_METHOD4(NS, __VA_ARGS__)
#define DERIVE_DEREF_METHOD4(NS, Ret, Name, Params)                            \
  static Ret Name(FUNC_PARAMS(Params)) {                                       \
    if constexpr (std::is_void_v<Ret>) {                                       \
      ::NS::Name(::gen_interface_detail::deref_forward(self)                   \
                     CALL_EXTRA_ARGS(Params));                                  \
    } else {                                                                   \
      return ::NS::Name(::gen_interface_detail::deref_forward(self)            \
                            CALL_EXTRA_ARGS(Params));                           \
    }                                                                          \
  }

#define DERIVE_ACCESSOR_METHOD4_TUPLE(NS, Accessor, M)                         \
  DERIVE_ACCESSOR_METHOD4_APPLY(NS, Accessor, UNWRAP(M))
#define DERIVE_ACCESSOR_METHOD4_APPLY(NS, Accessor, ...)                       \
  DERIVE_ACCESSOR_METHOD4(NS, Accessor, __VA_ARGS__)
#define DERIVE_ACCESSOR_METHOD4(NS, Accessor, Ret, Name, Params)               \
  static Ret Name(FUNC_PARAMS(Params)) {                                       \
    if constexpr (std::is_void_v<Ret>) {                                       \
      ::NS::Name(::gen_interface_detail::accessor_forward(self, Accessor)      \
                     CALL_EXTRA_ARGS(Params));                                  \
    } else {                                                                   \
      return ::NS::Name(::gen_interface_detail::accessor_forward(self,         \
                                                                  Accessor)    \
                            CALL_EXTRA_ARGS(Params));                           \
    }                                                                          \
  }

#define DERIVE_OPTIONAL_METHOD4_TUPLE(NS, Fallback, M)                         \
  DERIVE_OPTIONAL_METHOD4_APPLY(NS, Fallback, UNWRAP(M))
#define DERIVE_OPTIONAL_METHOD4_APPLY(NS, Fallback, ...)                       \
  DERIVE_OPTIONAL_METHOD4(NS, Fallback, __VA_ARGS__)
#define DERIVE_OPTIONAL_METHOD4(NS, Fallback, Ret, Name, Params)               \
  static Ret Name(FUNC_PARAMS(Params)) {                                       \
    if (::gen_interface_detail::optional_has_value(self)) {                    \
      if constexpr (std::is_void_v<Ret>) {                                     \
        ::NS::Name(::gen_interface_detail::deref_forward(self)                 \
                       CALL_EXTRA_ARGS(Params));                                \
      } else {                                                                 \
        return ::NS::Name(::gen_interface_detail::deref_forward(self)          \
                              CALL_EXTRA_ARGS(Params));                         \
      }                                                                        \
    } else {                                                                   \
      DERIVE_OPTIONAL_FALLBACK(Ret, Fallback, Params)                          \
    }                                                                          \
  }

#define DERIVE_OPTIONAL_FALLBACK(Ret, Fallback, Params)                         \
  DERIVE_OPTIONAL_FALLBACK_I(Ret, UNWRAP(Fallback), Params)
#define DERIVE_OPTIONAL_FALLBACK_I(Ret, ...)                                    \
  DERIVE_OPTIONAL_FALLBACK_II(Ret, __VA_ARGS__)
#define DERIVE_OPTIONAL_FALLBACK_II(Ret, Kind, ...)                             \
  DERIVE_OPTIONAL_FALLBACK_##Kind(Ret, __VA_ARGS__)
#define DERIVE_OPTIONAL_FALLBACK_none(Ret, ...)                                 \
  ::gen_interface_detail::optional_requires_fallback<Ret>();                    \
  return;
#define DERIVE_OPTIONAL_FALLBACK_fallback(Ret, Func, Params)                    \
  return (Func)(std::type_identity<Ret>{}, self CALL_EXTRA_ARGS(Params));

#define DERIVE_ITERATE_PTR_METHOD4_TUPLE(NS, PtrField, CountField, Reducer, M) \
  DERIVE_ITERATE_PTR_METHOD4_APPLY(NS, PtrField, CountField, Reducer, UNWRAP(M))
#define DERIVE_ITERATE_PTR_METHOD4_APPLY(NS, PtrField, CountField, Reducer, ...)\
  DERIVE_ITERATE_PTR_METHOD4(NS, PtrField, CountField, Reducer, __VA_ARGS__)
#define DERIVE_ITERATE_PTR_METHOD4(NS, PtrField, CountField, Reducer, Ret, Name,\
                                   Params)                                       \
  static Ret Name(FUNC_PARAMS(Params)) {                                       \
    auto range =                                                               \
        ::gen_interface_detail::ptr_range_from(self, &Self::PtrField,          \
                                               &Self::CountField);             \
    if constexpr (std::is_void_v<Ret>) {                                       \
      for (decltype(range.second) i = 0; i < range.second; ++i) {              \
        auto &elem = range.first[i];                                           \
        ::NS::Name(::gen_interface_detail::iter_receiver<FIRST(Params)>(elem)  \
                       CALL_EXTRA_ARGS(Params));                                \
      }                                                                        \
    } else {                                                                   \
      DERIVE_ITERATE_PTR_REDUCER(NS, Ret, Name, Params, Reducer, range)        \
    }                                                                          \
  }

#define DERIVE_ITERATE_PTR_REDUCER(NS, Ret, Name, Params, ReducerSpec, Range)  \
  DERIVE_ITERATE_PTR_REDUCER_I(NS, Ret, Name, Params, Range, UNWRAP(ReducerSpec))
#define DERIVE_ITERATE_PTR_REDUCER_I(NS, Ret, Name, Params, Range, ...)        \
  DERIVE_ITERATE_PTR_REDUCER_II(NS, Ret, Name, Params, Range, __VA_ARGS__)
#define DERIVE_ITERATE_PTR_REDUCER_II(NS, Ret, Name, Params, Range, Kind, ...) \
  DERIVE_ITERATE_PTR_REDUCER_##Kind(NS, Ret, Name, Params, Range, __VA_ARGS__)
#define DERIVE_ITERATE_PTR_REDUCER_each(NS, Ret, Name, Params, Range, ...)     \
  ::gen_interface_detail::iterate_each_requires_void<Ret>();
#define DERIVE_ITERATE_PTR_REDUCER_reduce(NS, Ret, Name, Params, Range, Init,  \
                                          Func)                                  \
  {                                                                            \
    auto total = Init;                                                         \
    for (decltype(Range.second) i = 0; i < Range.second; ++i) {                \
      auto &elem = Range.first[i];                                             \
      total = (Func)(                                                          \
          std::move(total),                                                    \
          ::NS::Name(::gen_interface_detail::iter_receiver<FIRST(Params)>(elem)\
                         CALL_EXTRA_ARGS(Params)));                             \
    }                                                                          \
    return static_cast<Ret>(total);                                            \
  }
#define DERIVE_ITERATE_PTR_REDUCER_map(NS, Ret, Name, Params, Range, Init,     \
                                       Func)                                     \
  {                                                                            \
    auto mapped = Init;                                                        \
    for (decltype(Range.second) i = 0; i < Range.second; ++i) {                \
      auto &elem = Range.first[i];                                             \
      auto &&receiver =                                                        \
          ::gen_interface_detail::iter_receiver<FIRST(Params)>(elem);          \
      mapped =                                                                 \
          (Func)(std::move(mapped), receiver,                                  \
                 ::NS::Name(receiver CALL_EXTRA_ARGS(Params)));                \
    }                                                                          \
    return static_cast<Ret>(mapped);                                           \
  }

#define DERIVE_TAG_UNION_METHOD4_TUPLE(NS, Visitor, M)                         \
  DERIVE_TAG_UNION_METHOD4_APPLY(NS, Visitor, UNWRAP(M))
#define DERIVE_TAG_UNION_METHOD4_APPLY(NS, Visitor, ...)                       \
  DERIVE_TAG_UNION_METHOD4(NS, Visitor, __VA_ARGS__)
#define DERIVE_TAG_UNION_METHOD4(NS, Visitor, Ret, Name, Params)               \
  static Ret Name(FUNC_PARAMS(Params)) {                                       \
    return ::gen_interface_detail::union_visit(                                \
        self, Visitor, std::type_identity<Ret>{},                              \
        [&](auto &inner) -> Ret {                                              \
          return ::NS::Name(                                                   \
              ::gen_interface_detail::variant_receiver<FIRST(Params)>(inner)   \
                  CALL_EXTRA_ARGS(Params));                                    \
        });                                                                    \
  }

#define DERIVE_ITERATE_METHOD4_TUPLE(NS, ReducerSpec, M)                       \
  DERIVE_ITERATE_METHOD4_APPLY(NS, ReducerSpec, UNWRAP(M))
#define DERIVE_ITERATE_METHOD4_APPLY(NS, ReducerSpec, ...)                     \
  DERIVE_ITERATE_METHOD4(NS, ReducerSpec, __VA_ARGS__)
#define DERIVE_ITERATE_METHOD4(NS, ReducerSpec, Ret, Name, Params)             \
  static Ret Name(FUNC_PARAMS(Params)) {                                       \
    if constexpr (std::is_void_v<Ret>) {                                       \
      for (auto &elem : ::gen_interface_detail::iterable_from(self))           \
        ::NS::Name(                                                            \
            ::gen_interface_detail::iter_receiver<FIRST(Params)>(elem)         \
                CALL_EXTRA_ARGS(Params));                                      \
    } else {                                                                   \
      DERIVE_ITERATE_REDUCER(NS, Ret, Name, Params, ReducerSpec)               \
    }                                                                          \
  }

#define DERIVE_ITERATE_REDUCER(NS, Ret, Name, Params, ReducerSpec)             \
  DERIVE_ITERATE_REDUCER_I(NS, Ret, Name, Params, UNWRAP(ReducerSpec))
#define DERIVE_ITERATE_REDUCER_I(NS, Ret, Name, Params, ...)                   \
  DERIVE_ITERATE_REDUCER_II(NS, Ret, Name, Params, __VA_ARGS__)
#define DERIVE_ITERATE_REDUCER_II(NS, Ret, Name, Params, Kind, ...)            \
  DERIVE_ITERATE_REDUCER_##Kind(NS, Ret, Name, Params, __VA_ARGS__)

#define DERIVE_ITERATE_REDUCER_each(NS, Ret, Name, Params, ...)                \
  ::gen_interface_detail::iterate_each_requires_void<Ret>();
#define DERIVE_ITERATE_REDUCER_reduce(NS, Ret, Name, Params, Init, Func)       \
  return static_cast<Ret>(::gen_interface_detail::iterate_reduce(              \
      ::gen_interface_detail::iterable_from(self),                             \
      Init,                                                                    \
      [&](auto acc, auto &elem) {                                              \
        return (Func)(                                                         \
            std::move(acc),                                                    \
            ::NS::Name(::gen_interface_detail::iter_receiver<FIRST(Params)>(   \
                           elem)                                               \
                           CALL_EXTRA_ARGS(Params)));                          \
      }));
#define DERIVE_ITERATE_REDUCER_map(NS, Ret, Name, Params, Init, Func)          \
  {                                                                            \
    auto mapped = Init;                                                        \
    for (auto &elem : ::gen_interface_detail::iterable_from(self)) {           \
      auto &&receiver =                                                        \
         ::gen_interface_detail::iter_receiver<FIRST(Params)>(elem);          \
      mapped =                                                                 \
         (Func)(std::move(mapped), receiver,                                  \
                ::NS::Name(receiver CALL_EXTRA_ARGS(Params)));                \
    }                                                                          \
    return static_cast<Ret>(mapped);                                           \
  }

#define DERIVE_RULES(NS, TP, MethodsTuple, RulesTuple)                         \
  DERIVE_RULES_I(VA_COUNT(UNWRAP_I RulesTuple), NS, TP, MethodsTuple,          \
                 UNWRAP_I RulesTuple)
#define DERIVE_RULES_I(N, NS, TP, MethodsTuple, ...)                           \
  DERIVE_RULES_II(N, NS, TP, MethodsTuple, __VA_ARGS__)
#define DERIVE_RULES_II(N, NS, TP, MethodsTuple, ...)                          \
  DERIVE_RULES_##N(NS, TP, MethodsTuple, __VA_ARGS__)
#define DERIVE_RULES_1(NS, TP, MethodsTuple, R1)                               \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R1)
#define DERIVE_RULES_2(NS, TP, MethodsTuple, R1, R2)                           \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R1)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R2)
#define DERIVE_RULES_3(NS, TP, MethodsTuple, R1, R2, R3)                       \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R1)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R2)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R3)
#define DERIVE_RULES_4(NS, TP, MethodsTuple, R1, R2, R3, R4)                   \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R1)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R2)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R3)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R4)
#define DERIVE_RULES_5(NS, TP, MethodsTuple, R1, R2, R3, R4, R5)               \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R1)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R2)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R3)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R4)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R5)
#define DERIVE_RULES_6(NS, TP, MethodsTuple, R1, R2, R3, R4, R5, R6)           \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R1)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R2)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R3)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R4)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R5)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R6)
#define DERIVE_RULES_7(NS, TP, MethodsTuple, R1, R2, R3, R4, R5, R6, R7)       \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R1)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R2)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R3)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R4)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R5)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R6)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R7)
#define DERIVE_RULES_8(NS, TP, MethodsTuple, R1, R2, R3, R4, R5, R6, R7, R8)   \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R1)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R2)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R3)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R4)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R5)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R6)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R7)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R8)
#define DERIVE_RULES_9(NS, TP, MethodsTuple, R1, R2, R3, R4, R5, R6, R7, R8,  \
                       R9)                                                      \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R1)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R2)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R3)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R4)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R5)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R6)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R7)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R8)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R9)
#define DERIVE_RULES_10(NS, TP, MethodsTuple, R1, R2, R3, R4, R5, R6, R7, R8, \
                        R9, R10)                                                \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R1)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R2)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R3)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R4)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R5)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R6)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R7)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R8)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R9)                                  \
  DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, R10)

#define DERIVE_RULE_TUPLE(NS, TP, MethodsTuple, Rule)                          \
  DERIVE_RULE_TUPLE_I(NS, TP, MethodsTuple, UNWRAP(Rule))
#define DERIVE_RULE_TUPLE_I(NS, TP, MethodsTuple, ...)                         \
  DERIVE_RULE_TUPLE_II(VA_COUNT(__VA_ARGS__), NS, TP, MethodsTuple, __VA_ARGS__)
#define DERIVE_RULE_TUPLE_II(N, NS, TP, MethodsTuple, ...)                     \
  DERIVE_RULE_TUPLE_III(N, NS, TP, MethodsTuple, __VA_ARGS__)
#define DERIVE_RULE_TUPLE_III(N, NS, TP, MethodsTuple, ...)                    \
  DERIVE_RULE_TUPLE_##N(NS, TP, MethodsTuple, __VA_ARGS__)
#define DERIVE_RULE_TUPLE_1(NS, TP, MethodsTuple, Kind)                        \
  DERIVE_RULE_##Kind##_1(NS, TP, MethodsTuple)
#define DERIVE_RULE_TUPLE_2(NS, TP, MethodsTuple, Kind, A1)                    \
  DERIVE_RULE_##Kind##_2(NS, TP, MethodsTuple, A1)
#define DERIVE_RULE_TUPLE_3(NS, TP, MethodsTuple, Kind, A1, A2)                \
  DERIVE_RULE_##Kind##_3(NS, TP, MethodsTuple, A1, A2)
#define DERIVE_RULE_TUPLE_4(NS, TP, MethodsTuple, Kind, A1, A2, A3)            \
  DERIVE_RULE_##Kind##_4(NS, TP, MethodsTuple, A1, A2, A3)
#define DERIVE_RULE_TUPLE_5(NS, TP, MethodsTuple, Kind, A1, A2, A3, A4)        \
  DERIVE_RULE_##Kind##_5(NS, TP, MethodsTuple, A1, A2, A3, A4)
#define DERIVE_RULE_TUPLE_6(NS, TP, MethodsTuple, Kind, A1, A2, A3, A4, A5)    \
  DERIVE_RULE_##Kind##_6(NS, TP, MethodsTuple, A1, A2, A3, A4, A5)
#define DERIVE_RULE_TUPLE_7(NS, TP, MethodsTuple, Kind, A1, A2, A3, A4, A5, A6)\
  DERIVE_RULE_##Kind##_7(NS, TP, MethodsTuple, A1, A2, A3, A4, A5, A6)
#define DERIVE_RULE_TUPLE_8(NS, TP, MethodsTuple, Kind, A1, A2, A3, A4, A5, A6,\
                            A7)                                                 \
  DERIVE_RULE_##Kind##_8(NS, TP, MethodsTuple, A1, A2, A3, A4, A5, A6, A7)
#define DERIVE_RULE_TUPLE_9(NS, TP, MethodsTuple, Kind, A1, A2, A3, A4, A5, A6,\
                            A7, A8)                                             \
  DERIVE_RULE_##Kind##_9(NS, TP, MethodsTuple, A1, A2, A3, A4, A5, A6, A7, A8)
#define DERIVE_RULE_TUPLE_10(NS, TP, MethodsTuple, Kind, A1, A2, A3, A4, A5,   \
                             A6, A7, A8, A9)                                   \
  DERIVE_RULE_##Kind##_10(NS, TP, MethodsTuple, A1, A2, A3, A4, A5, A6, A7,   \
                          A8, A9)

#define DERIVE_RULE_member_3(NS, TP, MethodsTuple, Wrapper, Field)             \
  DERIVE_RULE_MEMBER_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,        \
                       (Self), Field)
#define DERIVE_RULE_member_4(NS, TP, MethodsTuple, Wrapper, ArgsTuple, Field)  \
  DERIVE_RULE_MEMBER_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,        \
                       ArgsTuple, Field)

#define DERIVE_RULE_MEMBER_I(N, NS, MethodsTuple, Wrapper, ArgsTuple, Field)   \
  DERIVE_RULE_MEMBER_II(N, NS, MethodsTuple, Wrapper, ArgsTuple, Field)
#define DERIVE_RULE_MEMBER_II(N, NS, MethodsTuple, Wrapper, ArgsTuple, Field)  \
  DERIVE_RULE_MEMBER_##N(NS, MethodsTuple, Wrapper, ArgsTuple, Field)

#define DERIVE_RULE_MEMBER_1(NS, MethodsTuple, Target, ArgsTuple, Field)       \
  DERIVE_TARGET_TEMPLATE(NS, Target, ArgsTuple)                                \
  struct NS::Impl<DERIVE_TARGET_SELF(Target, ArgsTuple)> {                     \
    using Self = DERIVE_TARGET_SELF(Target, ArgsTuple);                        \
    FOR_EACH_WITH2(DERIVE_MEMBER_METHOD4_TUPLE, NS, Field,                     \
                   UNWRAP_I MethodsTuple)                                      \
  };
#define DERIVE_RULE_MEMBER_2(NS, MethodsTuple, Wrapper, ArgsTuple, Field)      \
  static_assert(false,                                                         \
                "derive rules currently support only single-parameter traits");
#define DERIVE_RULE_MEMBER_3(NS, MethodsTuple, Wrapper, ArgsTuple, Field)      \
  static_assert(false,                                                         \
                "derive rules currently support only single-parameter traits");

#define DERIVE_RULE_iterate_3(NS, TP, MethodsTuple, Wrapper, ArgsTuple)        \
  DERIVE_RULE_ITERATE_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,       \
                        ArgsTuple, (each))
#define DERIVE_RULE_iterate_4(NS, TP, MethodsTuple, Wrapper, ArgsTuple,        \
                              ReducerSpec)                                      \
  DERIVE_RULE_ITERATE_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,       \
                        ArgsTuple, ReducerSpec)

#define DERIVE_RULE_ITERATE_I(N, NS, MethodsTuple, Wrapper, ArgsTuple,         \
                              Reducer)                                          \
  DERIVE_RULE_ITERATE_II(N, NS, MethodsTuple, Wrapper, ArgsTuple, Reducer)
#define DERIVE_RULE_ITERATE_II(N, NS, MethodsTuple, Wrapper, ArgsTuple,        \
                               Reducer)                                         \
  DERIVE_RULE_ITERATE_##N(NS, MethodsTuple, Wrapper, ArgsTuple, Reducer)

#define DERIVE_RULE_ITERATE_1(NS, MethodsTuple, Target, ArgsTuple, Reducer)    \
  DERIVE_TARGET_TEMPLATE(NS, Target, ArgsTuple)                                \
  struct NS::Impl<DERIVE_TARGET_SELF(Target, ArgsTuple)> {                     \
    using Self = DERIVE_TARGET_SELF(Target, ArgsTuple);                        \
    FOR_EACH_WITH2(DERIVE_ITERATE_METHOD4_TUPLE, NS, Reducer,                  \
                   UNWRAP_I MethodsTuple)                                      \
  };
#define DERIVE_RULE_ITERATE_2(NS, MethodsTuple, Wrapper, ArgsTuple, Reducer)   \
  static_assert(false,                                                         \
                "derive rules currently support only single-parameter traits");
#define DERIVE_RULE_ITERATE_3(NS, MethodsTuple, Wrapper, ArgsTuple, Reducer)   \
  static_assert(false,                                                         \
                "derive rules currently support only single-parameter traits");

#define DERIVE_RULE_deref_2(NS, TP, MethodsTuple, Wrapper)                     \
  DERIVE_RULE_DEREF_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper, (Self))
#define DERIVE_RULE_deref_3(NS, TP, MethodsTuple, Wrapper, ArgsTuple)          \
  DERIVE_RULE_DEREF_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,         \
                      ArgsTuple)
#define DERIVE_RULE_DEREF_I(N, NS, MethodsTuple, Wrapper, ArgsTuple)           \
  DERIVE_RULE_DEREF_II(N, NS, MethodsTuple, Wrapper, ArgsTuple)
#define DERIVE_RULE_DEREF_II(N, NS, MethodsTuple, Wrapper, ArgsTuple)          \
  DERIVE_RULE_DEREF_##N(NS, MethodsTuple, Wrapper, ArgsTuple)
#define DERIVE_RULE_DEREF_1(NS, MethodsTuple, Target, ArgsTuple)               \
  DERIVE_TARGET_TEMPLATE(NS, Target, ArgsTuple)                                \
  struct NS::Impl<DERIVE_TARGET_SELF(Target, ArgsTuple)> {                     \
    using Self = DERIVE_TARGET_SELF(Target, ArgsTuple);                        \
    FOR_EACH_WITH(DERIVE_DEREF_METHOD4_TUPLE, NS, UNWRAP_I MethodsTuple)       \
  };
#define DERIVE_RULE_DEREF_2(NS, MethodsTuple, Wrapper, ArgsTuple)              \
  static_assert(false,                                                         \
                "deref rules currently support only single-parameter traits");
#define DERIVE_RULE_DEREF_3(NS, MethodsTuple, Wrapper, ArgsTuple)              \
  static_assert(false,                                                         \
                "deref rules currently support only single-parameter traits");

#define DERIVE_RULE_accessor_4(NS, TP, MethodsTuple, Wrapper, ArgsTuple,       \
                               Accessor)                                        \
  DERIVE_RULE_ACCESSOR_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,      \
                         ArgsTuple, Accessor)
#define DERIVE_RULE_ACCESSOR_I(N, NS, MethodsTuple, Wrapper, ArgsTuple,        \
                               Accessor)                                        \
  DERIVE_RULE_ACCESSOR_II(N, NS, MethodsTuple, Wrapper, ArgsTuple, Accessor)
#define DERIVE_RULE_ACCESSOR_II(N, NS, MethodsTuple, Wrapper, ArgsTuple,       \
                                Accessor)                                       \
  DERIVE_RULE_ACCESSOR_##N(NS, MethodsTuple, Wrapper, ArgsTuple, Accessor)
#define DERIVE_RULE_ACCESSOR_1(NS, MethodsTuple, Target, ArgsTuple, Accessor)  \
  DERIVE_TARGET_TEMPLATE(NS, Target, ArgsTuple)                                \
  struct NS::Impl<DERIVE_TARGET_SELF(Target, ArgsTuple)> {                     \
    using Self = DERIVE_TARGET_SELF(Target, ArgsTuple);                        \
    FOR_EACH_WITH2(DERIVE_ACCESSOR_METHOD4_TUPLE, NS, Accessor,                \
                   UNWRAP_I MethodsTuple)                                      \
  };
#define DERIVE_RULE_ACCESSOR_2(NS, MethodsTuple, Wrapper, ArgsTuple, Accessor) \
  static_assert(false,                                                         \
                "accessor rules currently support only single-parameter traits");
#define DERIVE_RULE_ACCESSOR_3(NS, MethodsTuple, Wrapper, ArgsTuple, Accessor) \
  static_assert(false,                                                         \
                "accessor rules currently support only single-parameter traits");

#define DERIVE_RULE_optional_2(NS, TP, MethodsTuple, Wrapper)                  \
  DERIVE_RULE_OPTIONAL_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,      \
                         (Self), (none))
#define DERIVE_RULE_optional_3(NS, TP, MethodsTuple, Wrapper, ArgsTuple)       \
  DERIVE_RULE_OPTIONAL_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,      \
                         ArgsTuple, (none))
#define DERIVE_RULE_optional_4(NS, TP, MethodsTuple, Wrapper, ArgsTuple,       \
                               Fallback)                                        \
  DERIVE_RULE_OPTIONAL_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,      \
                         ArgsTuple, (fallback, Fallback))
#define DERIVE_RULE_OPTIONAL_I(N, NS, MethodsTuple, Wrapper, ArgsTuple,        \
                               Fallback)                                        \
  DERIVE_RULE_OPTIONAL_II(N, NS, MethodsTuple, Wrapper, ArgsTuple, Fallback)
#define DERIVE_RULE_OPTIONAL_II(N, NS, MethodsTuple, Wrapper, ArgsTuple,       \
                                Fallback)                                       \
  DERIVE_RULE_OPTIONAL_##N(NS, MethodsTuple, Wrapper, ArgsTuple, Fallback)
#define DERIVE_RULE_OPTIONAL_1(NS, MethodsTuple, Target, ArgsTuple, Fallback)  \
  DERIVE_TARGET_TEMPLATE(NS, Target, ArgsTuple)                                \
  struct NS::Impl<DERIVE_TARGET_SELF(Target, ArgsTuple)> {                     \
    using Self = DERIVE_TARGET_SELF(Target, ArgsTuple);                        \
    FOR_EACH_WITH2(DERIVE_OPTIONAL_METHOD4_TUPLE, NS, Fallback,                \
                   UNWRAP_I MethodsTuple)                                      \
  };
#define DERIVE_RULE_OPTIONAL_2(NS, MethodsTuple, Wrapper, ArgsTuple, Fallback) \
  static_assert(false,                                                         \
                "optional rules currently support only single-parameter traits");
#define DERIVE_RULE_OPTIONAL_3(NS, MethodsTuple, Wrapper, ArgsTuple, Fallback) \
  static_assert(false,                                                         \
                "optional rules currently support only single-parameter traits");

#define DERIVE_RULE_nullable_2(NS, TP, MethodsTuple, Wrapper)                  \
  DERIVE_RULE_OPTIONAL_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,      \
                         (Self), (none))
#define DERIVE_RULE_nullable_3(NS, TP, MethodsTuple, Wrapper, ArgsTuple)       \
  DERIVE_RULE_OPTIONAL_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,      \
                         ArgsTuple, (none))
#define DERIVE_RULE_nullable_4(NS, TP, MethodsTuple, Wrapper, ArgsTuple,       \
                               Fallback)                                        \
  DERIVE_RULE_OPTIONAL_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,      \
                         ArgsTuple, (fallback, Fallback))

#define DERIVE_RULE_iterate_ptr_5(NS, TP, MethodsTuple, Wrapper, ArgsTuple,    \
                                  PtrField, CountField)                         \
  DERIVE_RULE_ITERATE_PTR_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,   \
                            ArgsTuple, PtrField, CountField, (each))
#define DERIVE_RULE_iterate_ptr_6(NS, TP, MethodsTuple, Wrapper, ArgsTuple,    \
                                  PtrField, CountField, Reducer)                \
  DERIVE_RULE_ITERATE_PTR_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,   \
                            ArgsTuple, PtrField, CountField, Reducer)
#define DERIVE_RULE_ITERATE_PTR_I(N, NS, MethodsTuple, Wrapper, ArgsTuple,     \
                                  PtrField, CountField, Reducer)                \
  DERIVE_RULE_ITERATE_PTR_II(N, NS, MethodsTuple, Wrapper, ArgsTuple, PtrField,\
                             CountField, Reducer)
#define DERIVE_RULE_ITERATE_PTR_II(N, NS, MethodsTuple, Wrapper, ArgsTuple,    \
                                   PtrField, CountField, Reducer)               \
  DERIVE_RULE_ITERATE_PTR_##N(NS, MethodsTuple, Wrapper, ArgsTuple, PtrField,  \
                              CountField, Reducer)
#define DERIVE_RULE_ITERATE_PTR_1(NS, MethodsTuple, Target, ArgsTuple,         \
                                  PtrField, CountField, Reducer)                \
  DERIVE_TARGET_TEMPLATE(NS, Target, ArgsTuple)                                \
  struct NS::Impl<DERIVE_TARGET_SELF(Target, ArgsTuple)> {                     \
    using Self = DERIVE_TARGET_SELF(Target, ArgsTuple);                        \
    FOR_EACH_WITH4(DERIVE_ITERATE_PTR_METHOD4_TUPLE, NS, PtrField, CountField, \
                   Reducer, UNWRAP_I MethodsTuple)                             \
  };
#define DERIVE_RULE_ITERATE_PTR_2(NS, MethodsTuple, Wrapper, ArgsTuple,        \
                                  PtrField, CountField, Reducer)                \
  static_assert(false,                                                         \
                "iterate_ptr rules currently support only single-parameter "   \
                "traits");
#define DERIVE_RULE_ITERATE_PTR_3(NS, MethodsTuple, Wrapper, ArgsTuple,        \
                                  PtrField, CountField, Reducer)                \
  static_assert(false,                                                         \
                "iterate_ptr rules currently support only single-parameter "   \
                "traits");

#define DERIVE_RULE_tag_union_3(NS, TP, MethodsTuple, Wrapper, Visitor)        \
  DERIVE_RULE_TAG_UNION_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,     \
                          (Self), Visitor)
#define DERIVE_RULE_tag_union_4(NS, TP, MethodsTuple, Wrapper, ArgsTuple,      \
                                Visitor)                                        \
  DERIVE_RULE_TAG_UNION_I(VA_COUNT(UNWRAP(TP)), NS, MethodsTuple, Wrapper,     \
                          ArgsTuple, Visitor)
#define DERIVE_RULE_TAG_UNION_I(N, NS, MethodsTuple, Wrapper, ArgsTuple,       \
                                Visitor)                                        \
  DERIVE_RULE_TAG_UNION_II(N, NS, MethodsTuple, Wrapper, ArgsTuple, Visitor)
#define DERIVE_RULE_TAG_UNION_II(N, NS, MethodsTuple, Wrapper, ArgsTuple,      \
                                 Visitor)                                       \
  DERIVE_RULE_TAG_UNION_##N(NS, MethodsTuple, Wrapper, ArgsTuple, Visitor)
#define DERIVE_RULE_TAG_UNION_1(NS, MethodsTuple, Target, ArgsTuple, Visitor)  \
  DERIVE_TARGET_TEMPLATE(NS, Target, ArgsTuple)                                \
  struct NS::Impl<DERIVE_TARGET_SELF(Target, ArgsTuple)> {                     \
    using Self = DERIVE_TARGET_SELF(Target, ArgsTuple);                        \
    FOR_EACH_WITH2(DERIVE_TAG_UNION_METHOD4_TUPLE, NS, Visitor,                \
                   UNWRAP_I MethodsTuple)                                      \
  };
#define DERIVE_RULE_TAG_UNION_2(NS, MethodsTuple, Wrapper, ArgsTuple, Visitor) \
  static_assert(false,                                                         \
                "tag_union rules currently support only single-parameter traits");
#define DERIVE_RULE_TAG_UNION_3(NS, MethodsTuple, Wrapper, ArgsTuple, Visitor) \
  static_assert(false,                                                         \
                "tag_union rules currently support only single-parameter traits");

#define DERIVE_VARIANT_METHOD4_TUPLE(NS, M)                                    \
  DERIVE_VARIANT_METHOD4_APPLY(NS, UNWRAP(M))
#define DERIVE_VARIANT_METHOD4_APPLY(NS, ...) DERIVE_VARIANT_METHOD4(NS, __VA_ARGS__)
#define DERIVE_VARIANT_METHOD4(NS, Ret, Name, Params)                          \
  static Ret Name(FUNC_PARAMS(Params)) {                                       \
    return std::visit(                                                         \
        [&](auto &inner) -> Ret {                                              \
          return ::NS::Name(                                                   \
              ::gen_interface_detail::variant_receiver<FIRST(Params)>(inner)   \
                  CALL_EXTRA_ARGS(Params));                                    \
        },                                                                     \
        ::gen_interface_detail::iterable_from(self));                          \
  }

#define DERIVE_FILTERED_VARIANT_METHOD4_TUPLE(NS, Fallback, M)                 \
  DERIVE_FILTERED_VARIANT_METHOD4_APPLY(NS, Fallback, UNWRAP(M))
#define DERIVE_FILTERED_VARIANT_METHOD4_APPLY(NS, Fallback, ...)               \
  DERIVE_FILTERED_VARIANT_METHOD4(NS, Fallback, __VA_ARGS__)
#define DERIVE_FILTERED_VARIANT_METHOD4(NS, Fallback, Ret, Name, Params)       \
  static Ret Name(FUNC_PARAMS(Params)) {                                       \
    return std::visit(                                                         \
        [&](auto &inner) -> Ret {                                              \
          using Inner = std::remove_cvref_t<decltype(inner)>;                  \
          if constexpr (::NS::Trait<Inner>) {                                  \
            return ::NS::Name(                                                 \
                ::gen_interface_detail::variant_receiver<FIRST(Params)>(inner) \
                    CALL_EXTRA_ARGS(Params));                                  \
          } else {                                                             \
            return (Fallback)(std::type_identity<Ret>{}, inner                 \
                              CALL_EXTRA_ARGS(Params));                        \
          }                                                                    \
        },                                                                     \
        ::gen_interface_detail::iterable_from(self));                          \
  }

#define DERIVE_RULE_variant_1(NS, TP, MethodsTuple)                            \
  static_assert(VA_COUNT(UNWRAP(TP)) == 1,                                     \
                "variant rules currently support only single-parameter traits");\
  template <typename... TraitDerived>                                          \
    requires(sizeof...(TraitDerived) > 0 &&                                     \
             (... && ::NS::Trait<TraitDerived>))                               \
  struct NS::Impl<std::variant<TraitDerived...>> {                             \
    using Self = std::variant<TraitDerived...>;                                \
    FOR_EACH_WITH(DERIVE_VARIANT_METHOD4_TUPLE, NS, UNWRAP_I MethodsTuple)     \
  };
#define DERIVE_RULE_variant_2(NS, TP, MethodsTuple, Fallback)                  \
  static_assert(VA_COUNT(UNWRAP(TP)) == 1,                                     \
                "variant rules currently support only single-parameter traits");\
  template <typename... TraitDerived>                                          \
    requires(sizeof...(TraitDerived) > 0)                                      \
  struct NS::Impl<std::variant<TraitDerived...>> {                             \
    using Self = std::variant<TraitDerived...>;                                \
    FOR_EACH_WITH2(DERIVE_FILTERED_VARIANT_METHOD4_TUPLE, NS, Fallback,        \
                   UNWRAP_I MethodsTuple)                                      \
  };

//--------------------------------------------------------------------
//  Main macros
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
    FOR_EACH_WITH2(MIXIN_METHOD4_TUPLE, NS, TP, __VA_ARGS__)                        \
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
    FOR_EACH_WITH2(MIXIN_METHOD4_TUPLE, NS, TP, __VA_ARGS__)                        \
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
#define TRAIT_SELECT(_1, _2, _3, _4, NAME, ...) NAME
#define trait(...)                                                             \
  TRAIT_SELECT(__VA_ARGS__, TRAIT_EXPAND_4, TRAIT_EXPAND_3)(__VA_ARGS__)
#define TRAIT_EXPAND_3(Name, TP, MethodsTuple)                                 \
  StrictTrait(Name, TP, UNWRAP_I MethodsTuple)
#define TRAIT_EXPAND_4(Name, TP, MethodsTuple, RulesTuple)                     \
  StrictTrait(Name, TP, UNWRAP_I MethodsTuple)                                 \
  DERIVE_RULES(Name, TP, MethodsTuple, RulesTuple)

#define static_trait(...) STATIC_TRAIT_EXPAND_1(__VA_ARGS__)
#define STATIC_TRAIT_EXPAND_1(...) STATIC_TRAIT_EXPAND_2(__VA_ARGS__)
#define STATIC_TRAIT_EXPAND_2(Name, TP, MethodsTuple)                          \
  STATIC_TRAIT_EXPAND_3(Name, TP, UNWRAP_I MethodsTuple)
#define STATIC_TRAIT_EXPAND_3(Name, TP, ...)                                   \
  StrictStaticTrait(Name, TP, __VA_ARGS__)

#define DUCKTYPED_TRAIT_SELECT(_1, _2, _3, _4, NAME, ...) NAME
#define ducktyped_trait(...)                                                   \
  DUCKTYPED_TRAIT_SELECT(__VA_ARGS__, DUCKTYPED_TRAIT_EXPAND_4,                \
                         DUCKTYPED_TRAIT_EXPAND_3)(__VA_ARGS__)
#define DUCKTYPED_TRAIT_EXPAND_3(Name, TP, MethodsTuple)                       \
  DuckTrait(Name, TP, UNWRAP_I MethodsTuple)
#define DUCKTYPED_TRAIT_EXPAND_4(Name, TP, MethodsTuple, RulesTuple)           \
  DuckTrait(Name, TP, UNWRAP_I MethodsTuple)                                   \
  DERIVE_RULES(Name, TP, MethodsTuple, RulesTuple)

#define static_ducktyped_trait(...) STATIC_DUCKTYPED_TRAIT_EXPAND_1(__VA_ARGS__)
#define STATIC_DUCKTYPED_TRAIT_EXPAND_1(...)                                   \
  STATIC_DUCKTYPED_TRAIT_EXPAND_2(__VA_ARGS__)
#define STATIC_DUCKTYPED_TRAIT_EXPAND_2(Name, TP, MethodsTuple)                \
  STATIC_DUCKTYPED_TRAIT_EXPAND_3(Name, TP, UNWRAP_I MethodsTuple)
#define STATIC_DUCKTYPED_TRAIT_EXPAND_3(Name, TP, ...)                         \
  StaticDuckTrait(Name, TP, __VA_ARGS__)

#endif // TRAIT_GENERATION_H
