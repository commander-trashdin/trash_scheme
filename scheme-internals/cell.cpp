#include "cell.h"
#include "gc.h"
#include "interfaces.h"
#include "storage.h"
#include "util.h"

Cell::Cell() : head_(nullptr), tail_(nullptr) {}

Cell::Cell(GCTracked *head, GCTracked *tail) : head_(head), tail_(tail) {}

Types Cell::ID() const { return Types::cell; }

void Cell::PrintTo(std::ostream *out) const {
  *out << '(';
  head_->PrintTo(out);
  if (tail_->ID() != Types::cell && tail_) {
    *out << " . ";
    tail_->PrintTo(out);
  } else {
    auto it = clistbegin();
    while (it != clistend()) {
      *out << " ";
      (*it)->PrintTo(out);
      if (tail_->ID() == Types::cell) {
        ++it;
      } else {
        *out << " . ";
        tail_->PrintTo(out);
        break;
      }
    }
  }
  *out << ')';
  return;
}

GCTracked *Cell::GetFirst() const { return head_; }

void Cell::SetFirst(GCTracked *object) { head_ = object; }

GCTracked *Cell::GetSecond() const { return tail_; }

void Cell::SetSecond(GCTracked *object) { tail_ = object; }

Cell::ContentIterator::ContentIterator(Cell *cell) : cell_(cell) {}

bool Cell::ContentIterator::operator!=(
    const Cell::ContentIterator &other) const {
  return false;
}

bool Cell::ContentIterator::operator==(
    const Cell::ContentIterator &other) const {
  return true;
};
Cell::ContentIterator &Cell::ContentIterator::operator++() { return *this; };
Cell::ContentIterator Cell::ContentIterator::operator++(int) { return *this; };
Cell::ContentIterator::reference Cell::ContentIterator::operator*() {
  throw std::runtime_error("Dereferencing empty iterator!");
};
Cell::ContentIterator::pointer Cell::ContentIterator::operator->() const {
  return nullptr;
};
template <typename T>
Cell::ListIterator<T>::ListIterator(const Cell *cell) : cell_(cell){};
template <typename T>
bool Cell::ListIterator<T>::operator!=(const ListIterator &other) const {
  return cell_ != other.cell_;
}
template <typename T>
bool Cell::ListIterator<T>::operator==(const ListIterator &other) const {
  return cell_ == other.cell_;
}
template <typename T>
Cell::ListIterator<T> &Cell::ListIterator<T>::operator++() {
  if (!cell_)
    throw std::runtime_error("Incrementing end iterator!");
  auto next = cell_->tail_;
  if (next->ID() == Types::cell)
    cell_ = next->As<Cell>();
  else
    throw std::runtime_error("Improper list!");
  return *this;
}
template <typename T>
Cell::ListIterator<T> Cell::ListIterator<T>::operator++(int) {
  if (!cell_)
    throw std::runtime_error("Incrementing end iterator!");
  auto next = cell_->tail_;
  auto current = *this;
  if (next->ID() == Types::cell)
    cell_ = next->As<Cell>();
  else
    throw std::runtime_error("Improper list!");
  return current;
}

template <typename T>
Cell::ListIterator<T>::reference Cell::ListIterator<T>::operator*() {
  return const_cast<const T *&>(cell_->head_);
}

template <typename T>
Cell::ListIterator<T>::pointer Cell::ListIterator<T>::operator->() const {
  return &(cell_->head_);
}

Cell::MutListIterator Cell::listbegin() { return MutListIterator(this); }
Cell::MutListIterator Cell::listend() { return MutListIterator(nullptr); }
Cell::ConstListIterator Cell::clistbegin() const {
  return ConstListIterator(this);
}
Cell::ConstListIterator Cell::clistend() const {
  return ConstListIterator(nullptr);
}