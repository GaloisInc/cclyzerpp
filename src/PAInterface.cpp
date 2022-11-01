#include "PAInterface.h"

#include <souffle/SouffleInterface.h>

// Public-facing interface to creating an instance
auto PAInterface::create(const std::string& dl_base_file)
    -> std::unique_ptr<PAInterface> {
  auto program = souffle::ProgramFactory::newInstance(dl_base_file);
  if (program == nullptr) {
    std::cerr << "Failed to create Souffle instance for " << dl_base_file
              << std::endl;
    return nullptr;
  }

  return std::unique_ptr<PAInterface>(new PAInterface(program));
}

// Private constructor
PAInterface::PAInterface(souffle::SouffleProgram* program)
    : souffle_program_(std::unique_ptr<souffle::SouffleProgram>(program)) {}

PAInterface::~PAInterface() = default;

//------------------------------------------------------------------------------
// Main entry point for running the pointer analysis, after factgen has
// completed
auto PAInterface::runPointerAnalysis(
    const boost::filesystem::path& p, const PAFlags flags) -> int {
  // Ensure we use an appropriate amount of parallelism
  unsigned threads = std::thread::hardware_concurrency();
  if (threads != 0) {
    souffle_program_->setNumThreads(threads);
  }

  // Now we can tell Souffle to load the files, including the configuration
  // file, and to run the pointer analysis.
  souffle_program_->loadAll(p.string());
  souffle_program_->run();

  if (flags & PAFlags::WRITE_ALL) {
    // This writes all of the pointer analysis results to files.
    souffle_program_->printAll(p.string());
  }

  return 0;
}

//------------------------------------------------------------------------------
// Assertions

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
constexpr static char const* assertion_relations[] = {
    "assert_every_allocation_has_a_type",
    // Type Compatibility
    "assert_type_compatible_constant_points_to",
    "assert_type_compatible_ptr_points_to",
    // Inhabitedness
    "assert_every_gep_constant_expr_points_to_something",
    "assert_every_pointer_constant_points_to_something",
    // Globals
    "assert_global_allocations_have_static_types",
    "assert_global_allocations_have_dynamic_types",
    "assert_global_allocations_have_one_type",
    // Reachability
    "assert_var_points_to_implies_reachable",
    "assert_bitcast_operand_in_same_function",
    // Contexts
    "subset._merge.assert_reachable_calls_have_contexts",
    "subset._merge.assert_reachable_calls_have_context_items",
    // Subset- and unification-based analyses
    "assert_subset_var_points_to_inhabited_implies_unification",
    "assert_subset_aliases_are_unification_aliases",
    "assert_unification_var_points_to_unique",
    "assert_every_allocation_has_a_region",
    "assert_every_allocation_has_one_region",
    "assert_has_repr",
    "assert_unique_repr",
    "assert_basic_or_path",
    // Schema
    "debuginfo_invalid_entry",
    "schema_invalid_alias",
    "schema_invalid_constant",
    "schema_invalid_func",
    "schema_invalid_global_var",
    "schema_invalid_instr",
    "schema_invalid_type",
    "user_option_invalid"};

void PAInterface::checkAssertions(bool expect) {
  for (const auto* relation_name : assertion_relations) {
    const auto* relation = souffle_program_->getRelation(relation_name);
    if (relation == nullptr) {
      if (!expect) {
        continue;
      }
      std::string error_message("Expected to find assertion relation: ");
      error_message += relation_name;
      throw std::logic_error(error_message);
    }
    if (relation->size() != 0) {
      std::string error_message("Datalog assertion failed: ");
      error_message += relation_name;
      throw std::logic_error(error_message);
    }
  }
}
