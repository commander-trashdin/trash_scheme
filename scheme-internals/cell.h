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

  virtual void Walk(const std::function<void(GCTracked *)> &fn) override;

  struct ConstListIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = const GCTracked *;
    using pointer = value_type *;
    using reference = value_type &;

    ConstListIterator(const Cell *cell);
    bool operator!=(const ConstListIterator &other) const;
    bool operator==(const ConstListIterator &other) const;

    ConstListIterator &operator++();

    ConstListIterator operator++(int);

    value_type operator*();

  private:
    const Cell *cell_;
  };

  struct MutListIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = GCTracked *;
    using pointer = value_type *;
    using reference = value_type &;

    MutListIterator(Cell *cell);
    bool operator!=(const MutListIterator &other) const;
    bool operator==(const MutListIterator &other) const;

    MutListIterator &operator++();

    MutListIterator operator++(int);

    value_type operator*();

  private:
    Cell *cell_;
  };

  ConstListIterator clistbegin() const;

  ConstListIterator clistend() const;

  MutListIterator listbegin();

  MutListIterator listend();

private:
  GCTracked *head_;
  GCTracked *tail_;
};