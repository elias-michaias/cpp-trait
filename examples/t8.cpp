// clang-format off
// t8.cpp -- Fluent APIs: builder pattern, cross-trait chaining, cross-type chaining.
//
// Demonstrates how the trait system supports ergonomic fluent interfaces:
//
//   1. Builder pattern: methods take Self* and return Self*, enabling
//      obj.set_a(x)->set_b(y)->set_c(z) chains via dot notation.
//
//   2. Cross-trait chaining: a single struct implements multiple traits.
//      A builder method returns Self*, then a method from a *different*
//      trait continues the chain on the same object:
//        cfg.set_host("...").validate().describe()
//
//   3. Cross-type chaining: a method on one type returns a *different*
//      type that itself implements traits:
//        cfg.set_host("...").build().start().stop()
//      Here build() returns Server*, and start/stop come from a
//      separate trait on Server.

#include "../trait.hpp"

#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
//  Traits
// ---------------------------------------------------------------------------

// Builder: every setter returns Self* for chaining.
trait(Builder, (Self), (
  (Self *, set_host,    (Self *, const char *)),
  (Self *, set_port,    (Self *, int)),
  (Self *, set_retries, (Self *, int)),
))

// Validation: returns bool, can be used as a chain terminal or in条件.
trait(Validatable, (Self), (
  (bool, validate, (Self *)),
))

// Describable: prints the object, returns Self* so it can sit mid-chain.
trait(Describable, (Self), (
  (Self *, describe, (Self *)),
))

// Lifecycle: start/stop on the built product.
trait(Lifecycle, (Self), (
  (Self *, start, (Self *)),
  (Self *, stop,  (Self *)),
))

// Config "builds" into a Server -- this is the cross-type boundary.
// build() lives on Config but returns Server*.
trait(ConfigBuild, (Self), (
  (void *, build, (Self *)),
))

// ---------------------------------------------------------------------------
//  Config -- the builder type
// ---------------------------------------------------------------------------

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
  static Config *set_port(Config *self, int p) {
    self->port = p;
    return self;
  }
  static Config *set_retries(Config *self, int n) {
    self->retries = n;
    return self;
  }
};

template <> struct Validatable::Impl<Config> {
  static bool validate(Config *self) {
    return self->host[0] != '\0' && self->port > 0;
  }
};

template <> struct Describable::Impl<Config> {
  static Config *describe(Config *self) {
    printf("Config(host=%s, port=%d, retries=%d)\n",
           self->host, self->port, self->retries);
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

// ---------------------------------------------------------------------------
//  Server -- the built product
// ---------------------------------------------------------------------------

template <> struct Lifecycle::Impl<Server> {
  static Server *start(Server *self) {
    self->running = true;
    printf("Server started on %s:%d\n", self->host, self->port);
    return self;
  }
  static Server *stop(Server *self) {
    self->running = false;
    printf("Server stopped\n");
    return self;
  }
};

template <> struct Describable::Impl<Server> {
  static Server *describe(Server *self) {
    printf("Server(%s:%d, running=%s, retries=%d)\n",
           self->host, self->port,
           self->running ? "true" : "false",
           self->retries);
    return self;
  }
};

// ---------------------------------------------------------------------------
//  Driver
// ---------------------------------------------------------------------------

int main() {
  // -- 1. Pure builder pattern: Self* returns for chaining -----------------
  //
  // Each set_* method returns Config*, so the next call chains via ->.

  Config cfg;
  Builder::set_host(&cfg, "localhost");
  Builder::set_port(&cfg, 8080);
  Builder::set_retries(&cfg, 3);

  // Same thing via Mixin dot notation:
  cfg.set_host("localhost")->set_port(8080)->set_retries(3);

  // -- 2. Cross-trait chaining: Builder + Validatable + Describable --------
  //
  // cfg.set_host(...) returns Config* (Builder trait).
  // ->validate() returns bool (Validatable trait) -- chain ends here.
  //
  // cfg.set_host(...) returns Config* (Builder trait).
  // ->describe() returns Config* (Describable trait) -- chain continues.

  Config cfg2;
  cfg2.set_host("example.com")
     ->set_port(443)
     ->set_retries(5);

  // validate() returns bool -- natural chain terminator
  bool ok = cfg2.validate();
  printf("valid: %d\n", ok);   // 1

  // describe() returns Config* -- can continue chaining after it
  cfg2.describe()
      ->set_retries(10);

  // The describe() call printed the old config, then we changed retries.
  cfg2.describe();              // retries is now 10

  // Full cross-trait chain in one expression:
  cfg2.set_host("final.com")->set_port(9999)->describe()->validate();
  //    ^ Builder           ^ Builder          ^ Describable  ^ Validatable

  // -- 3. Cross-type chaining: build() returns Server* --------------------
  //
  // cfg.build() is a ConfigBuild method that returns void* (cast to
  // Server*).  From there, start/stop/describe come from Lifecycle and
  // Describable traits on Server.

  Config cfg3;
  cfg3.set_host("api.service.io")
     ->set_port(3000)
     ->set_retries(3);

  // Build the server, then chain lifecycle methods.
  Server *srv = static_cast<Server *>(cfg3.build());

  srv->start()                   // Lifecycle::start, returns Server*
     ->describe()                // Describable::describe, returns Server*
     ->stop();                   // Lifecycle::stop, returns Server*

  // -- 4. Chaining through conditionals ------------------------------------
  //
  // Since validate() returns bool, it can gate further setup.

  Config cfg4;
  cfg4.set_host("guarded.host")->set_port(42);

  if (cfg4.validate()) {
    cfg4.set_retries(1)->describe();
  } else {
    printf("invalid config, skipping\n");
  }

  // -- 5. Method chaining with literal construction ------------------------
  //
  // A helper that returns a fresh Config on the stack, letting callers
  // chain from the very first call.

  // (We can't return temporaries from trait methods safely, but we CAN
  //  use a local and chain from there.)

  Config cfg5;
  cfg5.set_host("chained.host")
     ->set_port(5555)
     ->set_retries(2)
     ->describe()
     ->validate();

  // -- 6. Free-function dispatch alongside chaining ------------------------
  //
  // The same traits work via qualified free functions.

  Config cfg6;
  Builder::set_host(&cfg6, "free.func.host");
  Builder::set_port(&cfg6, 7777);
  Validatable::validate(&cfg6);
  Describable::describe(&cfg6);
}
