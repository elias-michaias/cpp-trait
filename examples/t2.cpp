#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include "../trait.hpp"

static_trait(Functor, (Self), (
  (type, value_type),
  (template, Mapped, (U)),
  hof((template, Mapped, (U)), map, (Self, fn(value_type))),
))

template <typename T>
struct Box {
  T inner;
};

template <typename T>
struct Maybe {
  std::optional<T> inner;
};

template <typename T>
struct Functor::Impl<Box<T>> {
  using value_type = T;

  template <typename U>
  using Mapped = Box<U>;

  template <class F>
  static auto map(Box<T> b, F&& fn) -> Mapped<std::remove_cvref_t<std::invoke_result_t<F&, T>>> {
    return Mapped{std::invoke(std::forward<F>(fn), b.inner)};
  }
};

template <typename T>
struct Functor::Impl<Maybe<T>> {
  using value_type = T;

  template <typename U>
  using Mapped = Maybe<U>;

  template <class F>
  static auto map(Maybe<T> m, F&& fn) -> Mapped<std::remove_cvref_t<std::invoke_result_t<F&, T>>> {
    using Out = std::remove_cvref_t<std::invoke_result_t<F&, T>>;
    if (!m.inner)
      return Mapped<Out>{std::nullopt};
    return Mapped<Out>{std::invoke(std::forward<F>(fn), *m.inner)};
  }
};

static int add1(int x) { return x + 1; }
static std::string to_string2(int x) { return std::to_string(x); }
static bool is_even(int x) { return (x % 2) == 0; }

static_assert(Functor::Trait<Box<int>>);
static_assert(Functor::Trait<Maybe<int>>);
static_assert(std::same_as<decltype(Functor::map(Box<int>{41}, add1)), Box<int>>);
static_assert(std::same_as<decltype(Functor::map(Box<int>{41}, [](auto x) {
  return std::to_string(x);
})), Box<std::string>>);
static_assert(std::same_as<decltype(Functor::map(Box<int>{41}, is_even)), Box<bool>>);

int main() {
  Box<int> b{41};
  auto b2 = Functor::map(b, add1);
  std::cout << b2.inner << "\n";

  auto b3 = Functor::map(b, [](auto x) {
    return std::to_string(x);
  });
  std::cout << b3.inner << "\n";

  auto b4 = Functor::map(b, is_even);
  std::cout << std::boolalpha << b4.inner << "\n";

  Maybe<int> m{std::optional<int>{10}};
  auto m2 = Functor::map(m, add1);
  std::cout << *m2.inner << "\n";

  auto m3 = Functor::map(m, to_string2);
  std::cout << *m3.inner << "\n";

  Maybe<int> empty{};
  auto e = Functor::map(empty, add1);
  std::cout << std::boolalpha << !e.inner.has_value() << "\n";
}
