#ifndef DEBUG_INFO_PREDICATE_GROUPS_H__
#define DEBUG_INFO_PREDICATE_GROUPS_H__

#include "predicate_groups.hpp"

namespace cclyzer::predicates {
//----------------------------------------------------
// Debug Info predicate group definitions
//----------------------------------------------------

struct di_entry : public predicate_group {};

// scope entries
struct di_scope_entry : public predicate_group {
  static pred_t id;
};

// variable entries
struct di_variable : public predicate_group {
  static pred_t id;
  static pred_t name;
  struct type {
    static pred_t node;
    static pred_t raw;
  };
  static pred_t file;
  static pred_t line;
  static pred_t scope;
};

struct di_local_var : public predicate_group {
  static pred_t id;
  static pred_t arg_num;
  static pred_t flag;
  static pred_t variable;
};

struct di_global_var : public predicate_group {
  static pred_t id;
  static pred_t variable;
  static pred_t linkage_name;
  static pred_t is_definition;
  static pred_t is_local_to_unit;
  static pred_t static_data_member_decl;
};

// file entries
struct di_file : public predicate_group {
  static pred_t id;
  static pred_t filename;
  static pred_t directory;
};

// namespace entries
struct di_namespace : public predicate_group {
  static pred_t id;
  static pred_t name;
  static pred_t file;
  static pred_t line;
  static pred_t scope;
};

// lexical block entries
struct di_lex_block : public predicate_group {
  static pred_t id;
  static pred_t file;
  static pred_t line;
  static pred_t column;
  static pred_t scope;
};

// lexical block file entries
struct di_lex_block_file : public predicate_group {
  static pred_t id;
  static pred_t file;
  static pred_t discriminator;
  static pred_t scope;
};

// subprogram entries
struct di_subprogram : public predicate_group {
  static pred_t id;
  static pred_t name;
  static pred_t linkage_name;
  static pred_t file;
  static pred_t line;
  struct scope {
    static pred_t node;
    static pred_t raw;
  };
  static pred_t scope_line;
  static pred_t type;
  struct containing_type {
    static pred_t node;
    static pred_t raw;
  };
  static pred_t declaration;
  static pred_t virtuality;
  static pred_t virtual_index;
  static pred_t flag;
  static pred_t template_param;
  static pred_t variable;
  static pred_t is_definition;
  static pred_t is_local_to_unit;
  static pred_t is_optimized;
  static pred_t function;
};

// type entries
struct di_type : public predicate_group {
  static pred_t id;
  static pred_t name;
  static pred_t line;
  static pred_t flag;
  static pred_t bitsize;
  static pred_t bitalign;
  static pred_t bitoffset;

  struct scope {
    static pred_t node;
    static pred_t raw;
  };
};

struct di_basic_type : public predicate_group {
  static pred_t id;
};

struct di_composite_type : public predicate_group {
  static pred_t id;
  static pred_t file;
  static pred_t abi_id;
  static pred_t field;
  static pred_t enumerator;
  static pred_t subrange;
  static pred_t template_param;

  struct vtable {
    static pred_t node;
    static pred_t raw;
  };

  struct basetype {
    static pred_t node;
    static pred_t raw;
  };

  // Kinds of composite types
  static pred_t kind;
  static pred_t structures;
  static pred_t classes;
  static pred_t arrays;
  static pred_t unions;
  static pred_t enumerations;
};

struct di_derived_type : public predicate_group {
  static pred_t id;
  static pred_t kind;
  static pred_t file;

  struct basetype {
    static pred_t node;
    static pred_t raw;
  };
};

struct di_subroutine_type : public predicate_group {
  static pred_t id;
  static pred_t type_elem;
  static pred_t raw_type_elem;
};

// template parameter entries
struct di_template_param : public predicate_group {
  static pred_t id;
  static pred_t name;

  struct type {
    static pred_t node;
    static pred_t raw;
  };
};

struct di_template_type_param : public predicate_group {
  static pred_t id;
};

struct di_template_value_param : public predicate_group {
  static pred_t id;
  static pred_t value;
  static pred_t elements;
};

// enumerator entries
struct di_enumerator : public predicate_group {
  static pred_t id;
  static pred_t name;
  static pred_t value;
};

// subrange entries
struct di_subrange : public predicate_group {
  static pred_t id;
  static pred_t lower_bound;
  static pred_t count;
};

// imported entity entries
struct di_imported_entity : public predicate_group {
  static pred_t id;
  static pred_t name;
  static pred_t line;
  static pred_t scope;

  struct entity {
    static pred_t node;
    static pred_t raw;
  };
};

// compilation unit entries
struct di_compile_unit : public predicate_group {
  // TODO
};

// location entries
struct di_location : public predicate_group {
  static pred_t id;
  static pred_t line;
  static pred_t column;
  static pred_t scope;
  static pred_t inlined_at;
};

}  // namespace cclyzer::predicates

#endif
