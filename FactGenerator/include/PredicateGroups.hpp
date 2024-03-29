#ifndef PREDICATE_GROUPS_H__
#define PREDICATE_GROUPS_H__

#include "Predicate.hpp"

namespace cclyzer::predicates {
// type aliases
typedef Predicate pred_t;

// A struct is used instead of a namespace, so that it can be
// passed as a template argument
struct predicate_group {};

// The predicates
extern Registry<pred_t> const predicates_reg;

//----------------------------------------------------
// Predicate group definitions, from this point on.
//----------------------------------------------------

#define GROUP_BEGIN(g) struct g : public predicate_group {
#define GROUP_END(g) \
  }                  \
  ;
#define PREDICATE(g, p, f) static pred_t p;
#include "./predicates.inc"

}  // namespace cclyzer::predicates

#endif
