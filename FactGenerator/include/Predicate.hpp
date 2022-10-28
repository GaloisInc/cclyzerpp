#ifndef PREDICATE_H__
#define PREDICATE_H__

#include <memory>
#include <set>
#include <string>
#include <utility>

#include "Singleton.hpp"

namespace cclyzer {
/* Forward declarations to be able to define actual registries */
class Predicate;

/* Registry base class */
template <typename T>
class Registry {
 public:
  Registry() { all().insert((const T *)(this)); }

  // Define iterator methods over class instances

  typedef typename std::set<const T *>::const_iterator iterator;

  static auto begin() -> iterator { return all().begin(); }

  static auto end() -> iterator { return all().end(); }

 protected:
  ~Registry() { all().erase((const T *)(this)); }

  // Collection of instances
  static auto all() -> std::set<const T *> &;

 private:
  // Make objects non-copyable
  Registry(const Registry &);
  auto operator=(const Registry &) -> Registry &;
};

template class Registry<Predicate>;
}  // namespace cclyzer

/* Predicate */

class cclyzer::Predicate : public Registry<Predicate> {
 public:
  Predicate(const char *name) : name(name) {}
  Predicate(std::string name) : name(std::move(name)) {}
  Predicate(const Predicate &other) = delete;  // non construction-copyable
  auto operator=(const Predicate &) -> Predicate & = delete;  // non copyable

  // Conversions

  [[nodiscard]] auto getName() const -> std::string { return name; }

  operator std::string() const { return name; }

  [[nodiscard]] auto c_str() const -> const char * { return name.c_str(); }

  // Comparison operators

  friend auto operator==(Predicate &p1, Predicate &p2) -> bool;
  friend auto operator!=(Predicate &p1, Predicate &p2) -> bool;

  friend auto operator>(Predicate &p1, Predicate &p2) -> bool;
  friend auto operator<=(Predicate &p1, Predicate &p2) -> bool;

  friend auto operator<(Predicate &p1, Predicate &p2) -> bool;
  friend auto operator>=(Predicate &p1, Predicate &p2) -> bool;

  friend auto operator<<(std::ostream &stream, const Predicate &p)
      -> std::ostream &;

  virtual ~Predicate() = default;

 private:
  const std::string name;
};

#endif
