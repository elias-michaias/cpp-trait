#ifndef TRAIT_FINALY_FIXED_HPP
#define TRAIT_FINALY_FIXED_HPP

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace trait_finally_fixed_detail {

template <class T>
using remove_cvref_t = std::remove_cvref_t<T>;

template <class T>
struct is_std_array : std::false_type {};
template <class T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};
template <class T>
inline constexpr bool is_std_array_v = is_std_array<remove_cvref_t<T>>::value;

template <class T>
struct is_std_optional : std::false_type {};
template <class T>
struct is_std_optional<std::optional<T>> : std::true_type {};
template <class T>
inline constexpr bool is_std_optional_v = is_std_optional<remove_cvref_t<T>>::value;

template <class T>
struct is_std_shared_ptr : std::false_type {};
template <class T>
struct is_std_shared_ptr<std::shared_ptr<T>> : std::true_type {};
template <class T>
inline constexpr bool is_std_shared_ptr_v = is_std_shared_ptr<remove_cvref_t<T>>::value;

template <class T>
struct is_std_variant : std::false_type {};
template <class... Ts>
struct is_std_variant<std::variant<Ts...>> : std::true_type {};
template <class T>
inline constexpr bool is_std_variant_v = is_std_variant<remove_cvref_t<T>>::value;

template <class T>
concept has_inner = requires(T t) { t.inner; };

template <class T>
concept has_get = requires(T t) { t.get(); };

template <class T>
concept has_bool_and_deref = requires(T t) {
  static_cast<bool>(t);
  *t;
};

template <class T>
concept has_ptr_and_count = requires(T t) {
  t.ptr;
  t.count;
};

template <class T>
concept has_kind_and_value = requires(T t) {
  t.kind;
  t.value;
};

template <class T>
concept is_pointer_like = std::is_pointer_v<remove_cvref_t<T>>;

template <class T>
concept is_slice_like = has_ptr_and_count<remove_cvref_t<T>>;

template <class T>
concept is_tagged_union_like = has_kind_and_value<remove_cvref_t<T>>;

} // namespace trait_finally_fixed_detail

struct Shape;
struct DoubleMap;
struct ScaleMap;
struct Into;
struct Test;

#define trait(Name, TP, MethodsTuple, ...) TRAIT_BUILD_##Name
#define static_trait(Name, TP, MethodsTuple) TRAIT_STATIC_BUILD_##Name
#define ducktyped_trait(Name, TP, MethodsTuple) TRAIT_DUCK_BUILD_##Name
#define static_ducktyped_trait(Name, TP, MethodsTuple) TRAIT_STATIC_DUCK_BUILD_##Name

// -----------------------------------------------------------------------------
// Shape
// -----------------------------------------------------------------------------
struct Shape {
  template <class T> struct Impl;

  struct Dyn {
    struct vtable_t {
      int (*area)(const void *);
      void (*scale)(void *, int);
    };

    std::shared_ptr<void> storage;
    const vtable_t *vt = nullptr;

    Dyn() = default;

    template <class T>
    requires(!std::same_as<trait_finally_fixed_detail::remove_cvref_t<T>, Dyn>)
    Dyn(T &&value) {
      emplace<trait_finally_fixed_detail::remove_cvref_t<T>>(std::forward<T>(value));
    }

    Dyn(const Dyn &) = default;
    Dyn(Dyn &&) noexcept = default;
    Dyn &operator=(const Dyn &) = default;
    Dyn &operator=(Dyn &&) noexcept = default;

    template <class T>
    requires(!std::same_as<trait_finally_fixed_detail::remove_cvref_t<T>, Dyn>)
    Dyn &operator=(T &&value) {
      emplace<trait_finally_fixed_detail::remove_cvref_t<T>>(std::forward<T>(value));
      return *this;
    }

    int area() const { return vt->area(storage.get()); }
    void scale(int factor) { vt->scale(storage.get(), factor); }

  private:
    template <class T, class U>
    void emplace(U &&value) {
      storage = std::make_shared<T>(std::forward<U>(value));
      vt = &table<T>();
    }

    template <class T>
    static const vtable_t &table() {
      static const vtable_t vt{
          +[](const void *p) -> int {
            return Shape::area(*static_cast<const T *>(p));
          },
          +[](void *p, int factor) {
            Shape::scale(static_cast<T *>(p), factor);
          }};
      return vt;
    }

    friend struct Shape;
  };

  struct Mixin {
    template <class Self>
    auto area(this Self &&self) {
      return Shape::area(std::forward<Self>(self));
    }

    template <class Self>
    auto scale(this Self &&self, int factor) {
      Shape::scale(std::forward<Self>(self), factor);
    }
  };

  template <class Self>
  static int area(Self &&self) {
    return area_impl(std::forward<Self>(self));
  }

  template <class Self>
  static void scale(Self &&self, int factor) {
    scale_impl(std::forward<Self>(self), factor);
  }

private:
  template <class T>
  static int area_impl(T &&self) {
    using U = trait_finally_fixed_detail::remove_cvref_t<T>;

    if constexpr (std::same_as<U, Dyn>) {
      return self.area();
    } else if constexpr (std::is_pointer_v<U>) {
      return area_impl(*self);
    } else if constexpr (requires { typename Shape::template Impl<U>; } &&
                         requires { Shape::template Impl<U>::area(std::forward<T>(self)); }) {
      return Shape::template Impl<U>::area(std::forward<T>(self));
    } else if constexpr (trait_finally_fixed_detail::has_inner<U>) {
      return area_impl(self.inner);
    } else if constexpr (trait_finally_fixed_detail::has_get<U>) {
      return area_impl(self.get());
    } else if constexpr (trait_finally_fixed_detail::is_std_shared_ptr_v<U>) {
      return self ? area_impl(*self) : 0;
    } else if constexpr (trait_finally_fixed_detail::is_std_optional_v<U>) {
      return self ? area_impl(*self) : 0;
    } else if constexpr (trait_finally_fixed_detail::has_bool_and_deref<U>) {
      return self ? area_impl(*self) : 0;
    } else if constexpr (trait_finally_fixed_detail::is_std_array_v<U>) {
      int acc = 0;
      for (const auto &elem : self) acc += area_impl(elem);
      return acc;
    } else if constexpr (trait_finally_fixed_detail::is_slice_like<U>) {
      int acc = 0;
      for (std::size_t i = 0; i < self.count; ++i) acc += area_impl(self.ptr[i]);
      return acc;
    } else if constexpr (trait_finally_fixed_detail::is_std_variant_v<U>) {
      return std::visit(
          [](auto &&alt) -> int { return area_impl(std::forward<decltype(alt)>(alt)); },
          self);
    } else if constexpr (trait_finally_fixed_detail::is_tagged_union_like<U>) {
      switch (self.kind) {
      case U::Kind::value:
        return area_impl(*self.value);
      case U::Kind::other:
        return 0;
      }
      return 0;
    } else {
      return 0;
    }
  }

  template <class T>
  static void scale_impl(T &&self, int factor) {
    using U = trait_finally_fixed_detail::remove_cvref_t<T>;

    if constexpr (std::same_as<U, Dyn>) {
      self.scale(factor);
    } else if constexpr (std::is_pointer_v<U>) {
      scale_impl(*self, factor);
    } else if constexpr (requires { typename Shape::template Impl<U>; } &&
                         requires { Shape::template Impl<U>::scale(std::addressof(self), factor); }) {
      Shape::template Impl<U>::scale(std::addressof(self), factor);
    } else if constexpr (trait_finally_fixed_detail::has_inner<U>) {
      scale_impl(self.inner, factor);
    } else if constexpr (trait_finally_fixed_detail::has_get<U>) {
      scale_impl(self.get(), factor);
    } else if constexpr (trait_finally_fixed_detail::is_std_shared_ptr_v<U>) {
      if (self) scale_impl(*self, factor);
    } else if constexpr (trait_finally_fixed_detail::is_std_optional_v<U>) {
      if (self) scale_impl(*self, factor);
    } else if constexpr (trait_finally_fixed_detail::has_bool_and_deref<U>) {
      if (self) scale_impl(*self, factor);
    } else if constexpr (trait_finally_fixed_detail::is_std_array_v<U>) {
      for (auto &elem : self) scale_impl(elem, factor);
    } else if constexpr (trait_finally_fixed_detail::is_slice_like<U>) {
      for (std::size_t i = 0; i < self.count; ++i) scale_impl(self.ptr[i], factor);
    } else if constexpr (trait_finally_fixed_detail::is_std_variant_v<U>) {
      std::visit(
          [&](auto &&alt) { scale_impl(std::forward<decltype(alt)>(alt), factor); },
          self);
    } else if constexpr (trait_finally_fixed_detail::is_tagged_union_like<U>) {
      switch (self.kind) {
      case U::Kind::value:
        scale_impl(*self.value, factor);
        break;
      case U::Kind::other:
        break;
      }
    } else {
      (void)self;
      (void)factor;
    }
  }

public:
  template <class T>
  static constexpr bool Trait = requires(T t) { Shape::area(t); };

  template <class T>
  static constexpr bool model = Trait<T>;
};

struct DoubleMap {
  template <class T> struct Impl;

  template <class Self>
  static auto doubled(Self &&self) {
    return doubled_impl(std::forward<Self>(self));
  }

  template <class T>
  static constexpr bool Trait = requires(T t) { DoubleMap::doubled(t); };

private:
  template <class T>
  static auto doubled_impl(T &&self) {
    using U = trait_finally_fixed_detail::remove_cvref_t<T>;

    if constexpr (requires { typename DoubleMap::template Impl<U>; } &&
                  requires { DoubleMap::template Impl<U>::doubled(std::forward<T>(self)); }) {
      return DoubleMap::template Impl<U>::doubled(std::forward<T>(self));
    } else if constexpr (trait_finally_fixed_detail::is_std_array_v<U>) {
      U out{};
      for (std::size_t i = 0; i < self.size(); ++i) {
        out[i] = doubled_impl(self[i]);
      }
      return out;
    } else if constexpr (trait_finally_fixed_detail::has_inner<U>) {
      U out = self;
      out.inner = doubled_impl(self.inner);
      return out;
    } else if constexpr (trait_finally_fixed_detail::is_std_variant_v<U>) {
      return std::visit(
          [](auto &&alt) -> U { return U{doubled_impl(std::forward<decltype(alt)>(alt))}; },
          self);
    } else {
      return self;
    }
  }
};

struct ScaleMap {
  template <class T> struct Impl;

  template <class Self>
  static void scale(Self &&self, int factor) {
    scale_impl(std::forward<Self>(self), factor);
  }

  template <class T>
  static constexpr bool Trait = requires(T t) { ScaleMap::scale(&t, 0); };

private:
  template <class T>
  static void scale_impl(T &&self, int factor) {
    using U = trait_finally_fixed_detail::remove_cvref_t<T>;

    if constexpr (requires { typename ScaleMap::template Impl<U>; } &&
                  requires { ScaleMap::template Impl<U>::scale(std::addressof(self), factor); }) {
      ScaleMap::template Impl<U>::scale(std::addressof(self), factor);
    } else if constexpr (std::is_pointer_v<U>) {
      scale_impl(*self, factor);
    } else if constexpr (trait_finally_fixed_detail::is_slice_like<U>) {
      for (std::size_t i = 0; i < self.count; ++i) {
        scale_impl(self.ptr[i], factor);
      }
    } else if constexpr (trait_finally_fixed_detail::has_inner<U>) {
      scale_impl(self.inner, factor);
    } else if constexpr (trait_finally_fixed_detail::has_get<U>) {
      scale_impl(self.get(), factor);
    } else if constexpr (trait_finally_fixed_detail::is_std_array_v<U>) {
      for (auto &elem : self) scale_impl(elem, factor);
    } else if constexpr (trait_finally_fixed_detail::is_std_shared_ptr_v<U>) {
      if (self) scale_impl(*self, factor);
    } else if constexpr (trait_finally_fixed_detail::is_std_optional_v<U>) {
      if (self) scale_impl(*self, factor);
    } else if constexpr (trait_finally_fixed_detail::has_bool_and_deref<U>) {
      if (self) scale_impl(*self, factor);
    } else if constexpr (trait_finally_fixed_detail::is_std_variant_v<U>) {
      std::visit(
          [&](auto &&alt) { scale_impl(std::forward<decltype(alt)>(alt), factor); },
          self);
    } else if constexpr (trait_finally_fixed_detail::is_tagged_union_like<U>) {
      switch (self.kind) {
      case U::Kind::value:
        scale_impl(*self.value, factor);
        break;
      case U::Kind::other:
        break;
      }
    } else {
      (void)self;
      (void)factor;
    }
  }
};

struct Into {
  template <class T, class O>
  struct Impl;

  template <class Out>
  struct Dyn {
    struct vtable_t {
      Out (*into)(const void *);
    };

    std::shared_ptr<void> storage;
    const vtable_t *vt = nullptr;

    Dyn() = default;

    template <class T>
    requires(!std::same_as<trait_finally_fixed_detail::remove_cvref_t<T>, Dyn>)
    Dyn(T &&value) {
      emplace<trait_finally_fixed_detail::remove_cvref_t<T>>(std::forward<T>(value));
    }

    Dyn(const Dyn &) = default;
    Dyn(Dyn &&) noexcept = default;
    Dyn &operator=(const Dyn &) = default;
    Dyn &operator=(Dyn &&) noexcept = default;

    template <class T>
    requires(!std::same_as<trait_finally_fixed_detail::remove_cvref_t<T>, Dyn>)
    Dyn &operator=(T &&value) {
      emplace<trait_finally_fixed_detail::remove_cvref_t<T>>(std::forward<T>(value));
      return *this;
    }

    Out into() const { return vt->into(storage.get()); }

  private:
    template <class T, class U>
    void emplace(U &&value) {
      storage = std::make_shared<T>(std::forward<U>(value));
      vt = &table<T>();
    }

    template <class T>
    static const vtable_t &table() {
      static const vtable_t vt{
          +[](const void *p) -> Out {
            return Into::template into<Out>(*static_cast<const T *>(p));
          }};
      return vt;
    }

    friend struct Into;
  };

  template <class Out, class Self>
  static Out into(Self &&self) {
    return into_impl<Out>(std::forward<Self>(self));
  }

private:
  template <class Out, class T>
  static Out into_impl(T &&self) {
    using U = trait_finally_fixed_detail::remove_cvref_t<T>;

    if constexpr (std::same_as<U, Dyn<Out>>) {
      return self.into();
    } else if constexpr (requires { typename Into::template Impl<U, Out>; } &&
                         requires { Into::template Impl<U, Out>::into(std::forward<T>(self)); }) {
      return Into::template Impl<U, Out>::into(std::forward<T>(self));
    } else if constexpr (std::is_convertible_v<T, Out>) {
      return static_cast<Out>(std::forward<T>(self));
    } else {
      return Out{};
    }
  }

public:
  template <class T, class Out>
  static constexpr bool Trait = requires(T t) { Into::template into<Out>(t); };

  template <class T, class Out>
  static constexpr bool model = Trait<T, Out>;
};

struct Test {
  template <class T> struct Impl;

  template <class T, class F>
  static decltype(auto) test(T &&value, F &&factor) {
    using U = trait_finally_fixed_detail::remove_cvref_t<T>;
    return Impl<U>::test(std::forward<T>(value), std::forward<F>(factor));
  }

  template <class T>
  static constexpr bool Trait = requires(T t) {
    typename Impl<trait_finally_fixed_detail::remove_cvref_t<T>>::Factor;
    Test::test(t, typename Impl<trait_finally_fixed_detail::remove_cvref_t<T>>::Factor{});
  };
};

// -----------------------------------------------------------------------------
// Macro-backed public declarations used by the example
// -----------------------------------------------------------------------------
#define TRAIT_BUILD_Shape
#define TRAIT_BUILD_DoubleMap
#define TRAIT_BUILD_ScaleMap
#define TRAIT_STATIC_BUILD_Test
#define TRAIT_DUCK_BUILD_Into
#define TRAIT_STATIC_DUCK_BUILD_Into

#endif // TRAIT_FINALY_FIXED_HPP
