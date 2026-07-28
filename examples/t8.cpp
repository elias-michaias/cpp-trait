// clang-format off
// t8.cpp -- Fluent APIs: builders, functional pipelines, and cross-type chaining.
//
// Struct definitions contain ONLY data.  All methods come from the Impls<T>
// layer chain:  .showln() from Show, .map() from Functor, .zip_with() from Zip.
//
// static_trait + hof() generates free functions but no layers.
// We bridge the gap with manual layer specializations that forward to the
// static trait free functions, giving us method syntax without struct methods.

#include "../trait.hpp"

#include <cstdio>
#include <cstring>
#include <type_traits>
#include <utility>

// ===========================================================================
//  Traits
// ===========================================================================

trait(Show, (Self), (
  (Self *, showln, (Self *)),
))

static_trait(Functor, (Self), (
      (type, value_type), 
      (template, Mapped, (U)),
      hof((template, Mapped, (U)), map, (Self, fn(U, value_type))), 
))

static_trait(Zip, (Self), (
      (type, value_type), 
      (template, Mapped, (U)),
      hof((template, Mapped, (U)), zip_with, (Self, Self, fn(U, value_type, value_type))), 
))

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

struct Server : Impls<Server> {
  char host[64] = {};
  int port       = 0;
  int retries    = 0;
  bool running   = false;
};

trait(ConfigBuild, (Self), (
  (Server, build, (Self *)),
))

// ===========================================================================
//  Pure data structs -- no methods, only members + constructors.
//  All behavior comes from Impls<T> layer chain.
// ===========================================================================

template <typename T>
struct Box : Impls<Box<T>> {
  T inner;
  Box() = default;
  explicit Box(T v) : inner(std::move(v)) {}
};

template <typename T>
struct Maybe : Impls<Maybe<T>> {
  bool has = false;
  T inner{};
  Maybe() = default;
  Maybe(bool h, T v) : has(h), inner(std::move(v)) {}
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
//  Show Implementations
// ===========================================================================

template <> struct Show::Impl<Box<int>> {
  static Box<int> *showln(Box<int> *self) {
    printf("  Box<int>{%d}\n", self->inner);
    return self;
  }
};

template <> struct Show::Impl<Box<double>> {
  static Box<double> *showln(Box<double> *self) {
    printf("  Box<double>{%.2f}\n", self->inner);
    return self;
  }
};

template <> struct Show::Impl<Maybe<int>> {
  static Maybe<int> *showln(Maybe<int> *self) {
    if (self->has) printf("  Maybe<int>{Just(%d)}\n", self->inner);
    else           printf("  Maybe<int>{Nothing}\n");
    return self;
  }
};

template <> struct Show::Impl<Maybe<double>> {
  static Maybe<double> *showln(Maybe<double> *self) {
    if (self->has) printf("  Maybe<double>{Just(%.2f)}\n", self->inner);
    else           printf("  Maybe<double>{Nothing}\n");
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

template <> struct ConfigBuild::Impl<Config> {
  static Server build(Config *self) {
    static Server srv;
    strncpy(srv.host, self->host, sizeof(srv.host) - 1);
    srv.port    = self->port;
    srv.retries = self->retries;
    srv.running = false;
    return srv;
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
  Server srv = cfg3.build();
  srv.start()->describe()->stop();

  // -- 4. Functional pipeline: cross-type map chain -------------------------
  //
  // Each .map() CHANGES the element type.
  //   Box<int>  -->  Box<double>  -->  Box<int>
  //
  // .map() comes from the Functor layer (layer 200), which forwards to the
  // static trait free function Functor::map.
  // .showln() comes from the Show trait layer (auto-registered).
  // Use named variables since layer methods use this auto & (lvalue only).

  printf("\n--- 4. Cross-Type Functional Pipeline ---\n");

  printf("  int -> double -> int:\n");
  auto s4a = Box<int>{42}.map([](int x) { return x * 2.0; });
  s4a.showln();
  auto s4b = s4a.map([](double x) { return (int)(x + 0.5); });
  s4b.showln();

  printf("  int -> double -> int -> double:\n");
  auto s4c = Box<int>{7}.map([](int x) { return x * 3.14; });
  s4c.showln();
  auto s4d = s4c.map([](double x) { return (int)x; });
  s4d.showln();
  auto s4e = s4d.map([](int x) { return x * 0.5; });
  s4e.showln();

  // -- 5. Free function alongside method syntax -----------------------------

  printf("\n--- 5. Free Function vs. Method Syntax ---\n");

  auto r1 = Functor::map(Box<int>{10}, [](int x) { return x * x; });
  printf("  free:  Box<int>{%d}\n", r1.inner);

  auto r2 = Box<int>{10}.map([](int x) { return x * x; });
  printf("  method: ");
  r2.showln();

  // -- 6. Maybe functor: short-circuit on Nothing ---------------------------

  printf("\n--- 6. Maybe Functor (Short-Circuit) ---\n");

  auto m1 = Maybe<int>{true, 42}.map([](int x) { return x * 2; });
  printf("  Just(42) * 2: ");
  m1.showln();

  auto m2 = Maybe<int>{false, 0}.map([](int x) { return x * 2; });
  printf("  Nothing * 2: ");
  m2.showln();

  auto s6a = Maybe<int>{true, 42}.map([](int x) { return x * 1.5; });
  printf("  Just(42) -> double: ");
  s6a.showln();
  auto s6b = s6a.map([](double x) { return (int)x; });
  printf("  -> int: ");
  s6b.showln();

  // -- 7. Zip: combine two containers --------------------------------------

  printf("\n--- 7. Zip: Combine Two Containers ---\n");

  auto z1 = Box<int>{3}.zip_with(Box<int>{4},
              [](int a, int b) { return a + b; });
  printf("  3 + 4: ");
  z1.showln();

  auto z2 = Box<int>{3}.zip_with(Box<int>{4},
              [](int a, int b) { return a * b; });
  printf("  3 * 4: ");
  z2.showln();

  // -- 8. Full pipeline: zip -> map -> show ---------------------------------

  printf("\n--- 8. Full Pipeline: Zip + Map + Show ---\n");

  auto z3 = Box<int>{5}.zip_with(Box<int>{3},
              [](int a, int b) { return a * b; });
  auto z4 = z3.map([](int x) { return x + 0.5; });
  printf("  5 * 3 + 0.5: ");
  z4.showln();

  // -- 9. Cross-type + cross-trait: the full picture -----------------------

  printf("\n--- 9. Full Cross-Type + Cross-Trait Flow ---\n");

  Config cfg4;
  cfg4.set_host("service.io")->set_port(9090)->set_retries(2);
  printf("  configured: ");
  cfg4.describe();
  printf("  built+started: ");
  Server s2 = cfg4.build();
  s2.start();
  printf("  describe: ");
  s2.describe();
  printf("  stopped: ");
  s2.stop();
}
