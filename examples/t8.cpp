// clang-format off
// t8.cpp -- Fluent APIs: builders, functional pipelines, and cross-type chaining.
//
// Demonstrates three layers of fluent API design using the trait system:
//
//   1. Builder pattern (Self* returns):
//        cfg.set_host("x")->set_port(8080)->set_retries(3)
//
//   2. Functional pipelines (static_trait Functor/Zip + Chain wrapper):
//        Chain<Box<int>>{42}
//          .map([](int x) { return x * 2.0; })      // int -> double
//          .map([](double x) { return (int)x; })     // double -> int
//          .showln()
//
//   3. Cross-type chaining: Config --build--> Server --start/stop--> lifecycle
//
// All three patterns compose freely: you can mix builder chains, functional
// pipelines, and cross-type transitions in the same program.

#include "../trait.hpp"

#include <cstdio>
#include <cstring>
#include <type_traits>
#include <utility>

// ===========================================================================
//  Traits
// ===========================================================================

// -- Show: chainable printing (trait = vtable + dyn + mixin) ----------------
trait(Show, (Self), (
  (Self *, showln, (Self *)),
))

// -- Functor: type-safe map (static_trait = compile-time only) ---------------
// Associated types: value_type, Mapped<U>.
// Higher-order function: map container + callable -> new container of new type.
static_trait(Functor, (Self),
             ((type, value_type), (template, Mapped, (U)),
              hof((template, Mapped, (U)), map,
                  (Self, fn(U, value_type))), ))

// -- Zip: combine two containers with a binary function ---------------------
static_trait(Zip, (Self),
             ((type, value_type), (template, Mapped, (U)),
              hof((template, Mapped, (U)), zip_with,
                  (Self, Self, fn(U, value_type, value_type))), ))

// -- Builder traits (Self* returns for fluent setters) ----------------------
trait(Builder, (Self), (
  (Self *, set_host,    (Self *, const char *)),
  (Self *, set_port,    (Self *, int)),
  (Self *, set_retries, (Self *, int)),
))

trait(Validatable, (Self), (
  (bool, validate, (Self *)),
))

trait(Describable, (Self), (
  (Self *, describe, (Self *)),
))

trait(Lifecycle, (Self), (
  (Self *, start, (Self *)),
  (Self *, stop,  (Self *)),
))

trait(ConfigBuild, (Self), (
  (void *, build, (Self *)),
))

// ===========================================================================
//  Container Types
// ===========================================================================

// Box<T>: simple value wrapper -- implements Functor, Zip, Show.
template <typename T> struct Box { T inner; };

// Maybe<T>: optional wrapper -- implements Functor with short-circuit.
template <typename T> struct Maybe { bool has; T inner; };

// ===========================================================================
//  Chain<T>: fluent wrapper over any Functor container.
//
//  .map(f)        -- delegates to Functor::map, returns Chain<Box<U>>
//  .zip_with(o,f) -- delegates to Zip::zip_with, returns Chain<Box<U>>
//  .showln()      -- delegates to Show::showln, returns *this for chaining
//
//  Each .map() can CHANGE the element type, producing a different Chain<T>.
//  This is the key: the type flows through the pipeline.
// ===========================================================================

template <typename T>
struct Chain {
  T value;

  template <typename F>
  auto map(F &&f) {
    auto result = Functor::map(std::move(value), std::forward<F>(f));
    return Chain<std::remove_cvref_t<decltype(result)>>{std::move(result)};
  }

  template <typename F>
  auto zip_with(Chain<T> other, F &&f) {
    auto result = Zip::zip_with(std::move(value), std::move(other.value),
                                std::forward<F>(f));
    return Chain<std::remove_cvref_t<decltype(result)>>{std::move(result)};
  }

  Chain &showln() {
    ::Show::showln(&value);
    return *this;
  }
};

// ===========================================================================
//  Functor Implementations
// ===========================================================================

template <typename T>
struct Functor::Impl<Box<T>> {
  using value_type = T;
  template <typename U> using Mapped = Box<U>;

  template <typename F>
  static auto map(Box<T> b, F &&fn)
      -> Mapped<std::remove_cvref_t<std::invoke_result_t<F &, T>>> {
    using Out = std::remove_cvref_t<std::invoke_result_t<F &, T>>;
    return Mapped<Out>{std::invoke(std::forward<F>(fn), b.inner)};
  }
};

template <typename T>
struct Functor::Impl<Maybe<T>> {
  using value_type = T;
  template <typename U> using Mapped = Maybe<U>;

  template <typename F>
  static auto map(Maybe<T> m, F &&fn)
      -> Mapped<std::remove_cvref_t<std::invoke_result_t<F &, T>>> {
    using Out = std::remove_cvref_t<std::invoke_result_t<F &, T>>;
    if (!m.has) return Mapped<Out>{false, Out{}};
    return Mapped<Out>{true, std::invoke(std::forward<F>(fn), m.inner)};
  }
};

// ===========================================================================
//  Zip Implementations
// ===========================================================================

template <typename T>
struct Zip::Impl<Box<T>> {
  using value_type = T;
  template <typename U> using Mapped = Box<U>;

  template <typename F>
  static auto zip_with(Box<T> a, Box<T> b, F &&fn)
      -> Mapped<std::remove_cvref_t<std::invoke_result_t<F &, T, T>>> {
    using Out = std::remove_cvref_t<std::invoke_result_t<F &, T, T>>;
    return Mapped<Out>{std::invoke(std::forward<F>(fn), a.inner, b.inner)};
  }
};

// ===========================================================================
//  Show Implementations (Self* returns for chainable .showln())
// ===========================================================================

template <> struct Show::Impl<Box<int>> {
  static Box<int> *showln(Box<int> *self) {
    printf("Box<int>{%d}\n", self->inner);
    return self;
  }
};

template <> struct Show::Impl<Box<double>> {
  static Box<double> *showln(Box<double> *self) {
    printf("Box<double>{%.2f}\n", self->inner);
    return self;
  }
};

template <> struct Show::Impl<Maybe<int>> {
  static Maybe<int> *showln(Maybe<int> *self) {
    if (self->has) printf("Maybe<int>{Just(%d)}\n", self->inner);
    else           printf("Maybe<int>{Nothing}\n");
    return self;
  }
};

template <> struct Show::Impl<Maybe<double>> {
  static Maybe<double> *showln(Maybe<double> *self) {
    if (self->has) printf("Maybe<double>{Just(%.2f)}\n", self->inner);
    else           printf("Maybe<double>{Nothing}\n");
    return self;
  }
};

// ===========================================================================
//  Config / Server (builder + cross-type pattern)
// ===========================================================================

struct Config : Impls<Config> {
  char host[64] = {};
  int port       = 0;
  int retries    = 0;
};

template <> struct Builder::Impl<Config> {
  static Config *set_host(Config *self, const char *h) {
    strncpy(self->host, h, sizeof(self->host) - 1);
    return self;
  }
  static Config *set_port(Config *self, int p) { self->port = p; return self; }
  static Config *set_retries(Config *self, int n) { self->retries = n; return self; }
};

template <> struct Validatable::Impl<Config> {
  static bool validate(Config *self) {
    return self->host[0] != '\0' && self->port > 0;
  }
};

template <> struct Describable::Impl<Config> {
  static Config *describe(Config *self) {
    printf("Config{host=%s, port=%d, retries=%d}\n", self->host, self->port, self->retries);
    return self;
  }
};

struct Server : Impls<Server> {
  char host[64] = {};
  int port       = 0;
  int retries    = 0;
  bool running   = false;
};

template <> struct ConfigBuild::Impl<Config> {
  static void *build(Config *self) {
    static Server srv;
    strncpy(srv.host, self->host, sizeof(srv.host) - 1);
    srv.port    = self->port;
    srv.retries = self->retries;
    srv.running = false;
    return &srv;
  }
};

template <> struct Lifecycle::Impl<Server> {
  static Server *start(Server *self) {
    self->running = true;
    printf("Server{host=%s, port=%d, running=true}\n", self->host, self->port);
    return self;
  }
  static Server *stop(Server *self) {
    self->running = false;
    printf("Server{stopped}\n");
    return self;
  }
};

template <> struct Describable::Impl<Server> {
  static Server *describe(Server *self) {
    printf("Server{host=%s, port=%d, running=%s}\n",
           self->host, self->port, self->running ? "true" : "false");
    return self;
  }
};

// ===========================================================================
//  Compile-time trait checks
// ===========================================================================

static_assert(Functor::Trait<Box<int>>);
static_assert(Functor::Trait<Box<double>>);
static_assert(Functor::Trait<Maybe<int>>);
static_assert(Functor::Trait<Maybe<double>>);
static_assert(Zip::Trait<Box<int>>);
static_assert(Zip::Trait<Box<double>>);

// ===========================================================================
//  Driver
// ===========================================================================

int main() {

  // -- 1. Builder pattern: Self* returns for chaining -----------------------

  printf("--- 1. Builder Pattern ---\n");

  Config cfg;
  Builder::set_host(&cfg, "localhost");
  Builder::set_port(&cfg, 8080);
  Builder::set_retries(&cfg, 3);

  cfg.set_host("localhost")->set_port(8080)->set_retries(3);

  // -- 2. Cross-trait chaining: Builder + Validatable + Describable ----------

  printf("\n--- 2. Cross-Trait Chaining ---\n");

  Config cfg2;
  cfg2.set_host("example.com")->set_port(443)->set_retries(5);
  printf("valid: %d\n", cfg2.validate());

  cfg2.describe()->set_retries(10);
  cfg2.describe();

  cfg2.set_host("final.com")->set_port(9999)->describe()->validate();

  // -- 3. Cross-type: Config -> Server via build() --------------------------

  printf("\n--- 3. Cross-Type: Config -> Server ---\n");

  Config cfg3;
  cfg3.set_host("api.service.io")->set_port(3000)->set_retries(3);
  Server *srv = static_cast<Server *>(cfg3.build());
  srv->start()->describe()->stop();

  // -- 4. Functional pipeline: cross-type map chain -------------------------
  //
  // The key demonstration: each .map() CHANGES the element type.
  //   Box<int>  -->  Box<double>  -->  Box<int>
  // The type flows through the pipeline, and the compiler tracks it.

  printf("\n--- 4. Cross-Type Functional Pipeline ---\n");

  printf("  int -> double -> int:\n");
  Chain<Box<int>>{42}
    .map([](int x) { return x * 2.0; })               // Box<int>    -> Box<double>
    .showln()                                           // Box<double>{84.00}
    .map([](double x) { return (int)(x + 0.5); })     // Box<double> -> Box<int>
    .showln();                                          // Box<int>{84}

  printf("  int -> double -> int -> double:\n");
  Chain<Box<int>>{7}
    .map([](int x) { return x * 3.14; })               // Box<int>    -> Box<double>
    .showln()                                           // Box<double>{21.98}
    .map([](double x) { return (int)x; })              // Box<double> -> Box<int>
    .showln()                                           // Box<int>{21}
    .map([](int x) { return x * 0.5; })               // Box<int>    -> Box<double>
    .showln();                                          // Box<double>{10.50}

  // -- 5. Free function alongside Chain (mixed API) -------------------------
  //
  // Functor::map works as a free function.
  // Chain::map wraps it for fluent syntax.
  // Both call the same underlying Impl.

  printf("\n--- 5. Free Function vs. Chain ---\n");

  auto r1 = Functor::map(Box<int>{10}, [](int x) { return x * x; });
  printf("  free:  Box<int>{%d}\n", r1.inner);

  printf("  chain: ");
  Chain<Box<int>>{10}
    .map([](int x) { return x * x; })
    .showln();

  // -- 6. Maybe functor: short-circuit on Nothing ---------------------------
  //
  // Maybe<T> implements Functor.  map() applies the function only when
  // has == true; Nothing passes through unchanged (fn is never called).

  printf("\n--- 6. Maybe Functor (Short-Circuit) ---\n");

  printf("  Just(42) -> *2:\n  ");
  Chain<Maybe<int>>{true, 42}
    .map([](int x) { return x * 2; })
    .showln();                                          // Maybe<int>{Just(84)}

  printf("  Nothing -> *2:\n  ");
  Chain<Maybe<int>>{false, 0}
    .map([](int x) { return x * 2; })
    .showln();                                          // Maybe<int>{Nothing}

  printf("  Just(42) -> double -> int:\n  ");
  Chain<Maybe<int>>{true, 42}
    .map([](int x) { return x * 1.5; })               // Maybe<int>    -> Maybe<double>
    .showln()                                           // Maybe<double>{Just(63.00)}
    .map([](double x) { return (int)x; })              // Maybe<double> -> Maybe<int>
    .showln();                                          // Maybe<int>{Just(63)}

  // -- 7. Zip: combine two containers --------------------------------------

  printf("\n--- 7. Zip: Combine Two Containers ---\n");

  printf("  ");
  Chain<Box<int>>{3}
    .zip_with(Chain<Box<int>>{4},
              [](int a, int b) { return a + b; })
    .showln();                                          // Box<int>{7}

  printf("  ");
  Chain<Box<int>>{3}
    .zip_with(Chain<Box<int>>{4},
              [](int a, int b) { return a * b; })
    .showln();                                          // Box<int>{12}

  // -- 8. Full pipeline: zip -> map -> show ---------------------------------

  printf("\n--- 8. Full Pipeline: Zip + Map + Show ---\n");

  printf("  ");
  Chain<Box<int>>{5}
    .zip_with(Chain<Box<int>>{3},
              [](int a, int b) { return a * b; })     // Box<int>{15}
    .map([](int x) { return x + 0.5; })               // Box<double>{15.5}
    .showln();                                          // Box<double>{15.50}

  // -- 9. Cross-type + cross-trait: the full picture -----------------------

  printf("\n--- 9. Full Cross-Type + Cross-Trait Flow ---\n");

  Config cfg4;
  cfg4.set_host("service.io")->set_port(9090)->set_retries(2);
  printf("  configured: ");
  cfg4.describe();
  printf("  built+started: ");
  static_cast<Server *>(cfg4.build())->start();
  printf("  describe: ");
  static_cast<Server *>(cfg4.build())->describe();
  printf("  stopped: ");
  static_cast<Server *>(cfg4.build())->stop();
}
