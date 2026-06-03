// clang-format off

#include <iostream>
#include "../trait.hpp"

template <typename T> struct AccessBox {
  T inner;
  T &get() { return inner; }
  const T &get() const { return inner; }
};

template <typename T> concept HasGet = requires(T t) { t.get(); };

trait(Display, (Self), (
  (int, width, (Self))
), (
  (accessor, (requires, HasGet), (Self),
   ([](auto &box) -> decltype(auto) { return box.get(); }))
))

trait(Outline, (Self), (
  (int, stroke, (Self))
), (
  (accessor, (requires, Display::Trait), (Self),
   ([](auto &box) -> decltype(auto) { return box.get(); }))
))

struct Glyph {
  int units;
};

template <> struct Display::Impl<Glyph> {
  static int width(Glyph g) { return g.units; }
};

template <> struct Outline::Impl<Glyph> {
  static int stroke(Glyph g) { return g.units + 10; }
};

static_assert(Display::Trait<Glyph>);
static_assert(Display::Trait<AccessBox<Glyph>>);
static_assert(Outline::Trait<Glyph>);
static_assert(Outline::Trait<AccessBox<Glyph>>);

int main() {
  Glyph g{.units = 7};
  AccessBox<Glyph> boxed{.inner = g};

  std::cout << Display::width(boxed) << "\n";   // 7
  std::cout << Outline::stroke(boxed) << "\n";  // 17
}
