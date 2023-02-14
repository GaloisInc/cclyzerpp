#ifndef PREDICATE_H__
#define PREDICATE_H__

#include <memory>
#include <set>
#include <string>
#include <utility>

namespace cclyzer {
/* Forward declarations to be able to define actual registries */
class Predicate;

/* Registry base class */
template <typename T>
class Registry {
 public:
  Registry() = default;
  Registry(std::initializer_list<const T *> l) : all_instances(l) {}
  ~Registry() { all_instances.clear(); }

  // Make objects non-copyable
  Registry(const Registry &) = delete;
  auto operator=(const Registry &) -> Registry & = delete;

  // Define iterator methods over class instances
  typedef typename std::set<const T *>::const_iterator iterator;
  auto begin() const -> iterator { return all_instances.begin(); }
  auto end() const -> iterator { return all_instances.end(); }

 protected:
  // Collection of instances
  std::set<const T *> all_instances;
};
}  // namespace cclyzer

/* Predicate */

class cclyzer::Predicate {
 public:
  Predicate(const char *name) : name(name) {}
  Predicate(std::string name) : name(std::move(name)) {}
  Predicate(const Predicate &other) = delete;  // non construction-copyable
  auto operator=(const Predicate &) -> Predicate & = delete;  // non copyable

  // Conversions

  [[nodiscard]] auto getName() const -> std::string { return name; }

  [[nodiscard]] operator std::string() const { return name; }

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
