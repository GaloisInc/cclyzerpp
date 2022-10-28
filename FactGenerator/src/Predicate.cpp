#include "Predicate.hpp"

// Add to current namespace
using cclyzer::Predicate;
using cclyzer::Registry;

// Initialize registries
template <typename T>
auto Registry<T>::all() -> std::set<const T *> & {
  static auto *allInstances = new std::set<const T *>();
  return *allInstances;
}

// Add explicit instantiations
namespace cclyzer {
template std::set<const Predicate *> &Registry<Predicate>::all();
}

// Comparing predicates

namespace cclyzer {

auto operator==(Predicate &p1, Predicate &p2) -> bool {
  return p1.name == p2.name;
}

auto operator!=(Predicate &p1, Predicate &p2) -> bool {
  return p1.name != p2.name;
}

auto operator<(Predicate &p1, Predicate &p2) -> bool {
  return p1.name < p2.name;
}

auto operator<=(Predicate &p1, Predicate &p2) -> bool {
  return p1.name <= p2.name;
}

auto operator>(Predicate &p1, Predicate &p2) -> bool {
  return p1.name > p2.name;
}

auto operator>=(Predicate &p1, Predicate &p2) -> bool {
  return p1.name >= p2.name;
}

auto operator<<(std::ostream &stream, const Predicate &pred) -> std::ostream & {
  return stream << std::string(pred);
}
}  // namespace cclyzer
