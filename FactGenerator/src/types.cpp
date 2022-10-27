// IWYU pragma: no_include <boost/unordered/detail/implementation.hpp>
#include <boost/unordered/unordered_set.hpp>  // for unordered_set<>...
#include <string>                             // for allocator

#include "FactGenerator.hpp"     // for FactGenerator
#include "TypeAccumulator.hpp"   // for TypeAccumulator
#include "TypeVisitor.hpp"       // for TypeVisitor
#include "predicate_groups.hpp"  // for primitive_type

// Forward decls
namespace llvm {
class DataLayout;
}

using cclyzer::FactGenerator;
namespace pred = cclyzer::predicates;

void FactGenerator::writeTypes(const llvm::DataLayout& layout) {
  using llvm_utils::TypeAccumulator;

  // Add basic primitive types
  writeFact(pred::primitive_type::id, "void");
  writeFact(pred::primitive_type::id, "label");
  writeFact(pred::primitive_type::id, "metadata");
  writeFact(pred::primitive_type::id, "x86mmx");

  // Find types contained in the types encountered so far, but not
  // referenced directly

  TypeAccumulator alltypes;
  alltypes.accumulate(types.begin(), types.end());

  // Create type visitor
  TypeVisitor TV(*this, layout);

  // Record each type encountered
  for (const auto* ty : alltypes) {
    TV.visitType(ty);
  }
}
