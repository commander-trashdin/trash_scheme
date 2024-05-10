#include "interfaces.h"

class Cell : public Object {
public:
  Cell();

  Cell(GCTracked *head, GCTracked *tail);

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;

  GCTracked *GetFirst() const;

  void SetFirst(GCTracked *object);

  GCTracked *GetSecond() const;

  void SetSecond(GCTracked *object);

  struct ContentIterator : public Object::ContentIterator {
    ContentIterator() {}
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

  struct ListIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = GCTracked *;
    using pointer = value_type *;
    using reference = value_type &;

    ListIterator() {}
    bool operator!=(const ListIterator &other) const { return false; };
    bool operator==(const ListIterator &other) const { return true; };
    ListIterator &operator++() { return *this; };
    ListIterator operator++(int) { return *this; };
    reference operator*() {
      throw std::runtime_error("Dereferencing empty iterator!");
    };
    pointer operator->() const { return nullptr; };
  };

  ListIterator listbegin() { return ListIterator(); }

  ListIterator listend() { return ListIterator(); }

private:
  GCTracked *head_;
  GCTracked *tail_;
};
