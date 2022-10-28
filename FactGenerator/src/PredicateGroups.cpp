#include "PredicateGroups.hpp"
using namespace cclyzer::predicates;

// Try: gcc -I include -E src/PredicateGroups.cpp
//
// GROUP_BEGIN, GROUP_END not needed
#define GROUP_BEGIN(g)
#define GROUP_END(group_end)
// TODO(#42): Derive relation/file name from group + predicate name, e.g.,
// #define PREDICATE(g, p, f) pred_t g::p (std::string(#g) + std::string("_") +
// std::string(#p));
#define Q(x) #x
#define QUOTE(x) Q(x)
#define PREDICATE(g, p, f) pred_t g::p(#f);
#include "predicates.inc"
