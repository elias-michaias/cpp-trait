// clang-format off

#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <variant>
#include <vector>
#include "../trait.hpp"

template <typename T> struct Box {
  T inner;
};

template <typename T> struct AccessBox {
  T inner;
  T &get() { return inner; }
  const T &get() const { return inner; }
};

template <typename T> concept HasGet = requires(T t) { t.get(); };
template <typename T> concept HasInner = requires(T t) { t.inner; };

template <typename T> struct MaybePtr {
  T *ptr;
  explicit operator bool() const { return ptr != nullptr; }
  T &operator*() { return *ptr; }
  const T &operator*() const { return *ptr; }
};

template <typename T> struct Slice {
  T *ptr;
  std::size_t count;
};

struct Circle;
struct Rect;

template <typename T> struct TaggedPtrUnion {
  enum class Kind { value, other } kind;
  union {
    T *value;
    int other;
  };
};

// 1-param: Shape
// due to limitations of VTable generation,
// the first type arg of a trait must be the first function arg ONLY
// it can't be return type or any argument after the first
// this is because it is actually "Self"
trait(Shape, (Self), (
  (int, area, (Self)),
  (void, scale, (Self *, int))
), (
  (member, Box, (Self), inner),
  (deref, std::shared_ptr, (Self)),
  (optional, std::optional, (Self),
   ([](auto ret, auto &, auto...) {
     if constexpr (std::is_void_v<typename decltype(ret)::type>)
       return;
     else
       return 0;
   })),
  (nullable, MaybePtr, (Self),
   ([](auto ret, auto &, auto...) {
     if constexpr (std::is_void_v<typename decltype(ret)::type>)
       return;
     else
       return 0;
   })),
  (accessor, (requires, HasGet, HasInner), (Self),
   ([](auto &box) -> decltype(auto) { return box.get(); })),
  (iterate, std::array, (Self, N),
   (reduce, 0, ([](auto acc, auto value) { return acc + value; }))),
  (iterate_ptr, Slice, (Self), ptr, count,
   (reduce, 0, ([](auto acc, auto value) { return acc + value; }))),
  (variant,
   ([](auto ret, auto &, auto...) {
     if constexpr (std::is_void_v<typename decltype(ret)::type>)
       return;
     else
       return 0;
   })),
  (tag_union, TaggedPtrUnion, (Self),
   ([](auto &self, auto ret, auto &&visit) -> typename decltype(ret)::type {
     using Union = std::remove_cvref_t<decltype(self)>;
     switch (self.kind) {
     case Union::Kind::value:
       return visit(*self.value);
     case Union::Kind::other:
       if constexpr (std::is_void_v<typename decltype(ret)::type>)
         return;
       else
         return 0;
     }
     if constexpr (std::is_void_v<typename decltype(ret)::type>)
       return;
     else
       return 0;
   }))
))

// static_trait(...)
// no vtable/::Dyn generation --
// allows first type arg in return type 
// or after first function arg
// no "Self" to speak of
static_trait(Test, (T), (
  (type, Factor),
  (T, test, (T, typename Impl<T>::Factor))
))


// C++ 20
// struct Circle : Shape::Mixin<Circle> { 
//   int r;
// };

// C++ 23
struct Circle : Shape::Mixin { 
  int r;
};

template <> struct Shape::Impl<Circle> {
  static int area(Circle c) { return c.r * c.r; }
  static void scale(Circle *c, int f) { c->r *= f; }
};

template <> struct Test::Impl<int> {
  using Factor = bool;
  static int test(int l, bool r) {
    if (l && r) return true;
    return false;
  }
};

struct Rect {
  int x, y;
};

template <> struct Shape::Impl<Rect> {
  static int area(Rect r) { return r.x * r.y; }
  static void scale(Rect *r, int f) {
    r->x *= f;
    r->y *= f;
  }
};

static_assert(Shape::Trait<Circle>);
static_assert(Shape::Trait<Rect>);
static_assert(Shape::Trait<std::array<Rect, 2>>);
static_assert(Shape::Trait<Box<Circle>>);
static_assert(Shape::Trait<std::shared_ptr<Circle>>);
static_assert(Shape::Trait<std::optional<Circle>>);
static_assert(Shape::Trait<MaybePtr<Circle>>);
static_assert(Shape::Trait<AccessBox<Circle>>);
static_assert(Shape::Trait<Slice<Circle>>);
static_assert(Shape::Trait<std::variant<Circle, Rect>>);
static_assert(Shape::Trait<std::variant<Circle, int>>);
static_assert(Shape::Trait<TaggedPtrUnion<Circle>>);
static_assert(Shape::Trait<Shape::Dyn>);
static_assert(Shape::Trait<std::array<Shape::Dyn, 3>>);
static_assert(Shape::Trait<Box<Shape::Dyn>>);

trait(AreaMap, (Self), (
  (std::vector<int>, collect, (Self))
), (
  (iterate, std::array, (Self, N),
   (map, std::vector<int>{},
    ([](auto acc, auto &elem, auto values) {
      acc.insert(acc.end(), values.begin(), values.end());
      acc.push_back(elem.r);
      return acc;
    })))
))

trait(RadiusMap, (Self), (
  (std::vector<int>, collect, (Self *))
), (
  (iterate_ptr, Slice, (Self), ptr, count,
   (map, std::vector<int>{},
    ([](auto acc, auto *elem, auto values) {
      acc.insert(acc.end(), values.begin(), values.end());
      elem->r += 10;
      acc.push_back(elem->r);
      return acc;
    })))
))

template <> struct AreaMap::Impl<Circle> {
  static std::vector<int> collect(Circle c) { return {c.r * c.r}; }
};

template <> struct RadiusMap::Impl<Circle> {
  static std::vector<int> collect(Circle *c) { return {c->r}; }
};

static_assert(AreaMap::Trait<Circle>);
static_assert(AreaMap::Trait<std::array<Circle, 2>>);
static_assert(RadiusMap::Trait<Circle>);
static_assert(RadiusMap::Trait<Slice<Circle>>);
 
// 2-param: Into<T>
// ducktyped_trait = allow C/C++ implicit conversions
// examples: 
// - int -> bool
// - const T -> T
// - T -> T &
// try turning this into a regular trait and seeing what happens!
ducktyped_trait(Into, (Self, T), (
  (T, into, (Self))
))

struct MyFloat {
  float v;
};

template <> struct Into::Impl<MyFloat, int> {
  static int into(const MyFloat &f) { return (int)f.v; }
};

template <> struct Into::Impl<float, int> {
  static int into(const float &f) { return (int)f; }
};

static_assert(Into::Trait<MyFloat, int>);
static_assert(Into::Trait<Into::Dyn<int>, int>);

int main() {
  // 1-param: Shape
  Circle c{.r = 5};
  std::cout << Shape::area(c) << "\n"; // 25
  Shape::scale(&c, 2);
  std::cout << Shape::area(c) << "\n"; // 100

  Shape::Dyn sd = c;
  std::cout << Shape::area(sd) << "\n"; // 100
  Shape::scale(&sd, 3);
  std::cout << Shape::area(sd) << "\n"; // 900

  Rect r{3, 4};
  std::array<Shape::Dyn, 2> arr = {c, r};
  std::cout << Shape::area(arr[0]) << " " << Shape::area(arr[1]) << "\n"; // 900 12

  Box<Circle> boxed{.inner = c};
  std::cout << Shape::area(boxed) << "\n"; // 900
  Shape::scale(&boxed, 2);
  std::cout << Shape::area(boxed) << "\n"; // 3600

  auto owned = std::make_shared<Circle>(Circle{.r = 6});
  std::cout << Shape::area(owned) << "\n"; // 36
  Shape::scale(&owned, 2);
  std::cout << Shape::area(owned) << "\n"; // 144

  std::optional<Circle> maybe = Circle{.r = 4};
  std::cout << Shape::area(maybe) << "\n"; // 16
  Shape::scale(&maybe, 2);
  std::cout << Shape::area(maybe) << "\n"; // 64
  maybe.reset();
  std::cout << Shape::area(maybe) << "\n"; // 0
  Shape::scale(&maybe, 2);

  MaybePtr<Circle> rawish{.ptr = &c};
  std::cout << Shape::area(rawish) << "\n"; // 3600
  Shape::scale(&rawish, 2);
  std::cout << Shape::area(rawish) << "\n"; // 14400

  AccessBox<Circle> accessed{.inner = Circle{.r = 3}};
  std::cout << Shape::area(accessed) << "\n"; // 9
  Shape::scale(&accessed, 2);
  std::cout << Shape::area(accessed) << "\n"; // 36

  Circle pair[2];
  pair[0].r = 1;
  pair[1].r = 2;
  Slice<Circle> slice{.ptr = pair, .count = 2};
  std::cout << Shape::area(slice) << "\n"; // 5
  Shape::scale(&slice, 3);
  std::cout << Shape::area(slice) << "\n"; // 45

  std::array<Circle, 2> mapped = {Circle{.r = 2}, Circle{.r = 3}};
  auto mapped_areas = AreaMap::collect(mapped);
  std::cout << mapped_areas[0] << " " << mapped_areas[1] << " "
            << mapped_areas[2] << " " << mapped_areas[3] << "\n"; // 4 2 9 3

  Circle mapped_pair[2];
  mapped_pair[0].r = 1;
  mapped_pair[1].r = 2;
  Slice<Circle> mapped_slice{.ptr = mapped_pair, .count = 2};
  auto mapped_radii = RadiusMap::collect(&mapped_slice);
  std::cout << mapped_radii[0] << " " << mapped_radii[1] << " "
            << mapped_radii[2] << " " << mapped_radii[3] << "\n"; // 1 11 2 12
  std::cout << mapped_pair[0].r << " " << mapped_pair[1].r << "\n"; // 11 12
 
  std::variant<Circle, Rect> vr = r;
  std::cout << Shape::area(vr) << "\n"; // 12
  Shape::scale(&vr, 2);
  std::cout << Shape::area(vr) << "\n"; // 48

  std::variant<Circle, int> mixed = 7;
  std::cout << Shape::area(mixed) << "\n"; // 0
  Shape::scale(&mixed, 2);
  std::cout << std::get<int>(mixed) << "\n"; // 7

  TaggedPtrUnion<Circle> tagged{.kind = TaggedPtrUnion<Circle>::Kind::value,
                                .value = &c};
  std::cout << Shape::area(tagged) << "\n"; // 14400
  Shape::scale(&tagged, 2);
  std::cout << Shape::area(tagged) << "\n"; // 57600

  // 2-param: Into
  MyFloat f{3.7f};
  float f2 = 4.9;
  Into::Dyn<int> id(f);
  Into::Dyn<int> f3 = f2;
  std::cout << Into::into<int>(id) << "\n"; // 3
  std::cout << Into::into<int>(f2) << "\n"; // 3
  // also via Dyn directly:
  std::cout << Into::into<int>(f) << "\n"; // 3
  std::cout << Into::into<int>(f3) << "\n"; // 3
                                      
  std::array<Into::Dyn<int>, 2> list = {id, f3};

  Test::test(2, 4);

  f3 = f;

  c.scale(2);
  std::cout << c.area() << std::endl;
  std::cout << f3.into() << std::endl;

}
