#include "cell.h"
#include "gc.h"
#include "interfaces.h"
#include "storage.h"
#include "util.h"

Cell *Cell::AllocIn(T *storage) { return &(storage->c_); }

Cell::Cell() : head_(Create<>()), tail_(Create<>()) {}

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
    ++it;
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

bool Cell::operator==(const Object &other) const {
  if (other.ID() != ID())
    return false;
  auto other_cell = static_cast<const Cell *>(&other);
  return *head_ == *other_cell->head_ && *tail_ == *other_cell->tail_;
}

void Cell::Walk(const std::function<void(GCTracked *)> &fn) {
  fn(head_);
  fn(tail_);
}

Cell::ConstListIterator::ConstListIterator(const Cell *cell) : cell_(cell) {};

bool Cell::ConstListIterator::operator!=(
    const Cell::ConstListIterator &other) const {
  return cell_ != other.cell_;
}
bool Cell::ConstListIterator::operator==(
    const Cell::ConstListIterator &other) const {
  return cell_ == other.cell_;
}

Cell::ConstListIterator &Cell::ConstListIterator::operator++() {
  if (!cell_)
    throw std::runtime_error("Incrementing end iterator!");
  auto next = cell_->tail_;
  if (next->ID() == Types::cell)
    cell_ = next->As<Cell>();
  else if (next->ID() == Types::null)
    cell_ = nullptr;
  else
    throw std::runtime_error("Improper list!");
  return *this;
}

Cell::ConstListIterator Cell::ConstListIterator::operator++(int) {
  auto current = *this;
  ++(*this);
  return current;
}

Cell::ConstListIterator::value_type Cell::ConstListIterator::operator*() {
  return cell_->head_;
}

Cell::MutListIterator::MutListIterator(Cell *cell) : cell_(cell) {};
bool Cell::MutListIterator::operator!=(
    const Cell::MutListIterator &other) const {
  return cell_ != other.cell_;
}
bool Cell::MutListIterator::operator==(
    const Cell::MutListIterator &other) const {
  return cell_ == other.cell_;
}

Cell::MutListIterator &Cell::MutListIterator::operator++() {
  if (!cell_)
    throw std::runtime_error("Incrementing end iterator!");
  auto next = cell_->tail_;
  if (next->ID() == Types::cell)
    cell_ = next->As<Cell>();
  else if (next->ID() == Types::null)
    cell_ = nullptr;
  else
    throw std::runtime_error("Improper list!");
  return *this;
}

Cell::MutListIterator Cell::MutListIterator::operator++(int) {
  auto current = *this;
  ++(*this);
  return current;
}

Cell::MutListIterator::value_type Cell::MutListIterator::operator*() {
  return cell_->head_;
}

Cell::MutListIterator Cell::listbegin() { return MutListIterator(this); }
Cell::MutListIterator Cell::listend() { return MutListIterator(nullptr); }
Cell::ConstListIterator Cell::clistbegin() const {
  return ConstListIterator(this);
}
Cell::ConstListIterator Cell::clistend() const {
  return ConstListIterator(nullptr);
}