// clang-format off

#include <iostream>
#include "../trait.hpp"

template <typename T> struct AccessBox {
  T inner;
  T &get() { return inner; }
  const T &get() const { return inner; }
};

template <typename T> concept HasGet = requires(T t) { t.get(); };

constexpr auto access_via_get = [](auto &box) -> decltype(auto) {
  return box.get();
};

#define DISPLAY_METHODS ((int, width, (Self)))
#define DISPLAY_RULES ((accessor, (requires, HasGet), (Self), access_via_get))

trait(Display, (Self), DISPLAY_METHODS, DISPLAY_RULES)

#define OUTLINE_METHODS ((int, stroke, (Self)))
#define OUTLINE_RULES                                                         \
  ((accessor, (requires, Display::Trait), (Self), access_via_get))

trait(Outline, (Self), OUTLINE_METHODS, OUTLINE_RULES)

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
