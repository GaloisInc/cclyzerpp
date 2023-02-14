// Try: gcc -I include -E src/PredicateGroups.cpp

#include "PredicateGroups.hpp"

namespace cclyzer::predicates {
// Define the predicates

// GROUP_BEGIN, GROUP_END not needed
#define GROUP_BEGIN(g)
#define GROUP_END(group_end)
#define PREDICATE(g, p, f) pred_t g::p(#f);
#include "predicates.inc"

// Register the predicates

// GROUP_BEGIN, GROUP_END not needed
#define GROUP_BEGIN(g)
#define GROUP_END(group_end)
#define PREDICATE(g, p, f) &g::p,

Registry<pred_t> const predicates_reg({
#include "predicates.inc"
});
}  // namespace cclyzer::predicates
