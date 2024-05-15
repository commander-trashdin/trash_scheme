#pragma once
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <istream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

enum class Types {
  t,
  cell,
  null,
  number,
  symbol,
  boolean,
  function,
  specialform,
  builtin
};

inline bool Subtype(const Types fst, const Types snd) {
  static std::unordered_map<Types, std::vector<Types>> map{
      {Types::t,
       {Types::t, Types::cell, Types::null, Types::number, Types::symbol,
        Types::boolean, Types::function, Types::specialform, Types::builtin}},
      {Types::cell, {Types::cell}},
      {Types::null, {Types::null}},
      {Types::number, {Types::number}},
      {Types::symbol, {Types::symbol, Types::boolean}},
      {Types::boolean, {Types::boolean}},
      {Types::function, {Types::function}},
      {Types::specialform, {Types::specialform}},
      {Types::builtin, {Types::builtin}}};
  return std::find(map[fst].begin(), map[fst].end(), snd) != map[fst].end();
}

enum class Kind { Allow, Disallow };

class Object;
class Scope;
class Number;
class Symbol;
class Boolean;
class Function;
class LambdaFunction;
class Cell;
class SpecialForm;
class GCTracked;

union T;

struct constant {};

template <typename Derived>
concept DerivedFromObject =
    std::derived_from<Derived, Object> || std::is_same_v<Derived, void>;

template <typename Tag>
concept ConstTag =
    std::is_same_v<Tag, void> || std::is_same_v<Tag, struct constant>;

template <typename Derived>
concept NumberOrSymbol =
    std::is_same_v<Derived, Number> || std::is_same_v<Derived, Symbol>;

template <typename Derived, typename Tag>
concept Creatable =
    (std::is_same_v<Tag, constant> && NumberOrSymbol<Derived>) ||
    !std::is_same_v<Tag, constant>;

template <typename T> struct is_static : std::false_type {};

template <> struct is_static<Boolean> : std::true_type {};
template <> struct is_static<void> : std::true_type {};

template <typename Derived>
inline constexpr bool is_static_v = is_static<Derived>::value;

class Object {
public:
  virtual ~Object();

  virtual Types ID() const { return Types::t; }

  virtual bool IsFalse() const { return false; }

  virtual void PrintTo(std::ostream *out) const = 0;

  virtual bool operator==(const Object &other) const { return false; };

  struct ContentIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = GCTracked *;
    using pointer = value_type *;
    using reference = value_type &;

    ContentIterator() {}
    virtual ~ContentIterator() {}
    bool operator!=(const ContentIterator &other) const { return false; };
    bool operator==(const ContentIterator &other) const { return true; };
    ContentIterator &operator++() { return *this; };
    ContentIterator operator++(int) { return *this; };
    reference operator*() {
      throw std::runtime_error("Dereferencing empty iterator!");
    };
    pointer operator->() const { return nullptr; };
  };

  ContentIterator contbegin() { return ContentIterator(); }

  ContentIterator contend() { return ContentIterator(); }
};

class Applicable {
  virtual GCTracked *Apply(std::shared_ptr<Scope> &scope, GCTracked *args) = 0;
};
