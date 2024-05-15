#pragma once
#include "interfaces.h"

class Cell : public Object {
public:
  static Cell *AllocIn(T *storage);
  Cell();

  Cell(GCTracked *head, GCTracked *tail);

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;

  virtual bool operator==(const Object &other) const override;

  GCTracked *GetFirst() const;

  void SetFirst(GCTracked *object);

  GCTracked *GetSecond() const;

  void SetSecond(GCTracked *object);

  struct ContentIterator : public Object::ContentIterator {
    explicit ContentIterator(Cell *cell);
    bool operator!=(const ContentIterator &other) const;
    bool operator==(const ContentIterator &other) const;
    ContentIterator &operator++();
    ContentIterator operator++(int);
    reference operator*();
    pointer operator->() const;
    enum class State { head, tail, end };

  private:
    Cell *cell_;
    State state_ = State::head;
  };

  ContentIterator contbegin() { return ContentIterator(this); }

  ContentIterator contend() { return ContentIterator(this); }

  template <typename T> struct ListIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T *;
    using pointer = value_type *;
    using reference = value_type &;

    explicit ListIterator(const Cell *cell = nullptr);
    bool operator!=(const ListIterator<T> &other) const;
    bool operator==(const ListIterator<T> &other) const;
    ListIterator &operator++();
    ListIterator operator++(int);
    reference operator*();
    pointer operator->() const;

  private:
    const Cell *cell_;
  };

  using ConstListIterator = ListIterator<const std::remove_const_t<GCTracked>>;
  using MutListIterator = ListIterator<std::remove_const_t<GCTracked>>;

  ConstListIterator clistbegin() const;

  ConstListIterator clistend() const;

  MutListIterator listbegin();

  MutListIterator listend();

private:
  GCTracked *head_;
  GCTracked *tail_;
};