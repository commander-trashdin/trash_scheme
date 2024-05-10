#pragma once
#include <concepts>
#include <cstdint>
#include <istream>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <unordered_map>
#include <utility>
#include <vector>

enum class Types { t, cell, number, symbol, function };

enum class Kind { Allow, Disallow };

class Object;
class Scope;
class Number;
class Symbol;
class GCTracked;

struct constant {};

template <typename Derived>
concept DerivedFromObject = std::derived_from<Derived, Object>;

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

class Object {
public:
  virtual ~Object();

  virtual Types ID() const;

  virtual bool IsFalse() const;

  virtual void PrintTo(std::ostream *out) const = 0;

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
  virtual Object *Apply(std::shared_ptr<Scope> &scope, GCTracked *args) = 0;
};
