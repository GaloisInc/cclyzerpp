#include "predicate_groups.hpp"
using namespace cclyzer::predicates;

// GROUP_BEGIN, GROUP_END not needed
//
// Try: gcc -I include -E src/predicate_groups.cpp
#define GROUP_BEGIN(g)
#define GROUP_END(group_end)
// TODO(lb): Derive filename from group and predicate, e.g.,
// #define PREDICATE(g, p, f) pred_t g::p (std::string(#g) + std::string("_") +
// std::string(#p));
#define PREDICATE(g, p, f) pred_t g::p(f);
#include "predicates.inc"
