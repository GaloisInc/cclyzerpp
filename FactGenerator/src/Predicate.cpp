#include "Predicate.hpp"

// Add to current namespace
using cclyzer::Predicate;

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
