// clang-format off
// t10.cpp -- cpp2-style DSL: `fn` for (higher-order) functions, `assoctype`
//            without `as`, `assoctemplate` with single-paren `as (U)`,
//            `callable(...)` using `->` for return type.

#include <cstdio>
#include <type_traits>
#include <functional>
#include "../cpp2_trait.hpp"

// ===========================================================================
//  Trait declarations (cpp2 ergonomic syntax)
// ===========================================================================

traitdef(
    // -- 1. Static trait: Functor -------------------------------------------
    static_trait Functor with (
        assoctype value_type as (),
        assoctemplate Mapped as ((U)),
        fn map as ((Self, callable((T) -> U)) -> Mapped<U>)
    )

    // -- 2. Static trait: Zip -----------------------------------------------
    static_trait Zip with (
        assoctype value_type as (),
        assoctemplate Mapped as ((U)),
        fn zip_with as ((Self, Self, callable((T, T) -> U)) -> Mapped<U>)
    )

    // -- 3. Static trait: Foldable (fixed return type) ----------------------
    static_trait Foldable with (
        assoctype value_type as (),
        fn fold as ((Self, int, callable((int, T) -> int)) -> int)
    )

    // -- 5. Static duck trait with assoctype --------------------------------
    static_duck_trait Iter with (
        assoctype Item as ()
    )

    // -- 6. Regular trait for layer-chain dot syntax -----------------------
    dynamic Show with (
        fn show  as ((Self const &) -> void),
        fn showln as ((Self *) -> Self *)
    )
)
CPP2_END;

// ===========================================================================
//  Helper containers
// ===========================================================================

template <typename T> struct Box   { T value; };
template <typename T> struct Maybe { bool has; T value; };

// ===========================================================================
//  Functor Implementations
// ===========================================================================

template <typename T>
struct Functor::Impl<Box<T>> {
    using value_type = T;
    template <typename U> using Mapped = Box<U>;

    template <class F>
    static auto map(Box<T> b, F&& fn)
        -> Mapped<std::remove_cvref_t<std::invoke_result_t<F&, T>>> {
        return { std::invoke(std::forward<F>(fn), b.value) };
    }
};

template <typename T>
struct Functor::Impl<Maybe<T>> {
    using value_type = T;
    template <typename U> using Mapped = Maybe<U>;

    template <class F>
    static auto map(Maybe<T> m, F&& fn)
        -> Mapped<std::remove_cvref_t<std::invoke_result_t<F&, T>>> {
        if (!m.has) return { false, {} };
        return { true, std::invoke(std::forward<F>(fn), m.value) };
    }
};

static_assert(Functor::Trait<Box<int>>);
static_assert(Functor::Trait<Box<double>>);
static_assert(Functor::Trait<Maybe<int>>);

// ===========================================================================
//  Zip Implementations
// ===========================================================================

template <typename T>
struct Zip::Impl<Box<T>> {
    using value_type = T;
    template <typename U> using Mapped = Box<U>;

    template <class F>
    static auto zip_with(Box<T> a, Box<T> b, F&& fn)
        -> Mapped<std::remove_cvref_t<std::invoke_result_t<F&, T, T>>> {
        return { std::invoke(std::forward<F>(fn), a.value, b.value) };
    }
};

static_assert(Zip::Trait<Box<int>>);
static_assert(Zip::Trait<Box<double>>);

// ===========================================================================
//  Foldable Implementations
// ===========================================================================

template <typename T>
struct Foldable::Impl<Box<T>> {
    using value_type = T;

    template <class F>
        requires std::invocable<F&, int, T>
    static auto fold(Box<T> b, int init, F&& fn)
        -> std::invoke_result_t<F&, int, T> {
        return std::invoke(std::forward<F>(fn), init, b.value);
    }
};

static_assert(Foldable::Trait<Box<int>>);

// ===========================================================================
//  Iter Implementations (static duck trait with assoctype)
// ===========================================================================

struct Counter {
    int cur;
};
template <> struct Iter::Impl<Counter> {
    using Item = int;
};

struct BadCounter {
    int cur;
};
template <> struct Iter::Impl<BadCounter> {
    // no Item typedef -- concept should fail
};

static_assert( Iter::Trait<Counter>);
static_assert(!Iter::Trait<BadCounter>);

// ===========================================================================
//  Impls-based types with dot-syntax (via layer chain)
// ===========================================================================

struct MyBox : Impls<MyBox> {
    int value;
};

template <> struct Show::Impl<MyBox> {
    static void show(const MyBox &self) {
        printf("MyBox(%d)", self.value);
    }
    static MyBox *showln(MyBox *self) {
        printf("MyBox(%d)\n", self->value);
        return self;
    }
};

template <> struct Functor::Impl<MyBox> {
    using value_type = int;
    template <typename U> using Mapped = MyBox;

    template <class F>
    static auto map(MyBox b, F&& fn)
        -> Mapped<std::remove_cvref_t<std::invoke_result_t<F&, int>>> {
        using Out = std::remove_cvref_t<std::invoke_result_t<F&, int>>;
        return MyBox{{}, static_cast<int>(std::invoke(std::forward<F>(fn), b.value))};
    }
};

// ===========================================================================
//  Return-type verification
// ===========================================================================

static_assert(std::same_as<
    decltype(Functor::map(Box<int>{1}, [](int x) { return x + 1; })),
    Box<int>>);
static_assert(std::same_as<
    decltype(Functor::map(Box<int>{1}, [](int x) { return (double)x; })),
    Box<double>>);

static_assert(std::same_as<
    decltype(Zip::zip_with(Box<int>{2}, Box<int>{3},
                           [](int a, int b) { return a + b; })),
    Box<int>>);

static_assert(std::same_as<
    decltype(Foldable::fold(Box<int>{5}, 0, [](int acc, int v) { return acc + v; })),
    int>);

// ===========================================================================
//  main -- runtime verification
// ===========================================================================

int main() {
    // -- 1. Functor::map over Box -------------------------------------------
    Box<int> b{5};
    auto b2 = Functor::map(b, [](int x) { return x * 2; });
    printf("%d\n", b2.value);  // 10

    // map that changes element type: int -> double
    auto b3 = Functor::map(b, [](int x) { return (double)x; });
    printf("%g\n", b3.value);  // 5

    // -- 2. Functor::map over Maybe (present) -------------------------------
    Maybe<int> m{true, 7};
    auto m2 = Functor::map(m, [](int x) { return x + 1; });
    printf("%d %d\n", (int)m2.has, m2.value);  // 1 8

    // -- 3. Functor::map over Maybe (absent -- maps to empty) ---------------
    Maybe<int> empty{false, 0};
    auto m3 = Functor::map(empty, [](int x) { return x + 1; });
    printf("%d\n", (int)m3.has);  // 0

    // -- 4. Zip::zip_with ---------------------------------------------------
    auto z = Zip::zip_with(Box<int>{3}, Box<int>{4},
                           [](int a, int b) { return a * b; });
    printf("%d\n", z.value);  // 12

    // -- 5. Foldable::fold --------------------------------------------------
    Box<int> bf{10};
    int sum = Foldable::fold(bf, 100, [](int acc, int v) { return acc + v; });
    printf("%d\n", sum);  // 110

    // -- 6. Layer-chain dot syntax (showln from Show, map from Functor) -----
    MyBox mb{{}, 42};
    printf("  MyBox:");
    mb.show();
    printf("\n");

    auto mb2 = mb.map([](int x) { return x + 1; });
    printf("  after map(+1): ");
    mb2.showln();
}
