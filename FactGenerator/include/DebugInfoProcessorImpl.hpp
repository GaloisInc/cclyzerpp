#ifndef DEBUG_INFO_PROCESSOR_IMPL_HPP__
#define DEBUG_INFO_PROCESSOR_IMPL_HPP__

#include <llvm/IR/DebugInfo.h>

#include <map>

#include "DebugInfoProcessor.hpp"
#include "Demangler.hpp"
#include "FactWriter.hpp"
#include "ForwardingFactWriter.hpp"
#include "RefmodeEngine.hpp"

class cclyzer::DebugInfoProcessor::Impl : private Demangler,
                                          private ForwardingFactWriter {
 public:
  Impl(FactWriter &writer, RefmodeEngine &engine)
      : ForwardingFactWriter(writer), refmEngine(engine) {}

  /* Delegate to debug info finder */

  void processModule(const llvm::Module &module) {
    debugInfoFinder.processModule(module);
  }

  void processDeclare(
      const llvm::Module &module, const llvm::DbgDeclareInst *inst) {
    debugInfoFinder.processDeclare(module, inst);
    record_local_var_assoc(*inst);
  }

  void processValue(
      const llvm::Module &module, const llvm::DbgValueInst *inst) {
    debugInfoFinder.processValue(module, inst);
  }

  void reset() { debugInfoFinder.reset(); }

  /* Process debug info node and record its attributes */
  template <typename T, typename writer>
  struct di_recorder {
    using DIProc = Impl;

    static auto record(const T &dinode, DIProc &proc) -> refmode_t {
      const llvm::MDNode &node = llvm::cast<llvm::MDNode>(dinode);
      auto &nodeIds = proc.nodeIds;
      auto &eng = proc.refmEngine;

      // Check if node has been processed before
      auto search = nodeIds.find(&node);

      if (search != nodeIds.end()) return search->second;

      // Generate refmode for this node
      refmode_t nodeId = eng.refmode<llvm::MDNode>(node);

      // Cache the generated refmode and *then* process it
      // (i.e., record all its attributes, etc), so that it gets
      // not processed twice in the presence of cyclic
      // references
      writer::write(dinode, nodeIds[&node] = nodeId, proc);

      return nodeId;
    }
  };

  /* Fact-generating methods */

  struct write_di_node {
    using DIProc = Impl;
  };

  struct write_di_file : public write_di_node {
    static void write(const llvm::DIFile &, const refmode_t &, DIProc &);
  };

  struct write_di_namespace : public write_di_node {
    static void write(const llvm::DINamespace &, const refmode_t &, DIProc &);
  };

  struct write_di_scope : public write_di_node {
    static void write(const llvm::DIScope &, const refmode_t &, DIProc &);
  };

  struct write_di_type : public write_di_node {
    static void write(const llvm::DIType &, const refmode_t &, DIProc &);
  };

  struct write_di_basic_type : public write_di_node {
    static void write(const llvm::DIBasicType &, const refmode_t &, DIProc &);
  };

  struct write_di_composite_type : public write_di_node {
    static void write(
        const llvm::DICompositeType &, const refmode_t &, DIProc &);
  };

  struct write_di_derived_type : public write_di_node {
    static void write(const llvm::DIDerivedType &, const refmode_t &, DIProc &);
  };

  struct write_di_subroutine_type : public write_di_node {
    static void write(
        const llvm::DISubroutineType &, const refmode_t &, DIProc &);
  };

  struct write_di_tpl_param : public write_di_node {
    static void write(
        const llvm::DITemplateParameter &, const refmode_t &, DIProc &);
  };

  struct write_di_tpl_type_param : public write_di_node {
    static void write(
        const llvm::DITemplateTypeParameter &, const refmode_t &, DIProc &);
  };

  struct write_di_tpl_value_param : public write_di_node {
    static void write(
        const llvm::DITemplateValueParameter &, const refmode_t &, DIProc &);
  };

  struct write_di_variable : public write_di_node {
    static void write(const llvm::DIVariable &, const refmode_t &, DIProc &);
  };

  struct write_di_global_variable : public write_di_node {
    static void write(
        const llvm::DIGlobalVariable &, const refmode_t &, DIProc &);
  };

  struct write_di_local_variable : public write_di_node {
    static void write(
        const llvm::DILocalVariable &, const refmode_t &, DIProc &);
  };

  struct write_di_subprogram : public write_di_node {
    static void write(const llvm::DISubprogram &, const refmode_t &, DIProc &);
  };

  struct write_di_lex_block : public write_di_node {
    static void write(
        const llvm::DILexicalBlock &, const refmode_t &, DIProc &);
  };

  struct write_di_lex_block_file : public write_di_node {
    static void write(
        const llvm::DILexicalBlockFile &, const refmode_t &, DIProc &);
  };

  struct write_di_enumerator : public write_di_node {
    static void write(const llvm::DIEnumerator &, const refmode_t &, DIProc &);
  };

  struct write_di_subrange : public write_di_node {
    static void write(const llvm::DISubrange &, const refmode_t &, DIProc &);
  };

  struct write_di_imported_entity : public write_di_node {
    static void write(
        const llvm::DIImportedEntity &, const refmode_t &, DIProc &);
  };

  struct write_di_location : public write_di_node {
    static void write(const llvm::DILocation &, const refmode_t &, DIProc &);
  };

  /* Type aliases for common recording operations */

  using record_di_file = di_recorder<llvm::DIFile, write_di_file>;
  using record_di_namespace =
      di_recorder<llvm::DINamespace, write_di_namespace>;
  using record_di_scope = di_recorder<llvm::DIScope, write_di_scope>;
  using record_di_type = di_recorder<llvm::DIType, write_di_type>;
  using record_di_template_param =
      di_recorder<llvm::DITemplateParameter, write_di_tpl_param>;
  using record_di_variable = di_recorder<llvm::DIVariable, write_di_variable>;
  using record_di_subprogram =
      di_recorder<llvm::DISubprogram, write_di_subprogram>;
  using record_di_enumerator =
      di_recorder<llvm::DIEnumerator, write_di_enumerator>;
  using record_di_subrange = di_recorder<llvm::DISubrange, write_di_subrange>;
  using record_di_imported_entity =
      di_recorder<llvm::DIImportedEntity, write_di_imported_entity>;
  using record_di_location = di_recorder<llvm::DILocation, write_di_location>;

  void generateDebugInfo(const llvm::Module &, const std::string &);

 protected:
  /* Helper method to write common type attributes */
  void write_di_type_common(const llvm::DIType &, const refmode_t &);

  /* Helper method to write union attributes */
  template <typename P, typename writer, typename T>
  void recordUnionAttribute(const refmode_t &, const T *);

  /* Helper method to write bit flags */
  void recordFlags(const Predicate &, const refmode_t &, unsigned);

  /* Helper method to write constant */
  auto recordConstant(const llvm::Constant &) -> refmode_t;

  /* Record association between DI Node and LLVM local variable */
  void record_local_var_assoc(const llvm::DbgDeclareInst &);

  /* Write all local variable associations */
  void write_local_var_assocs();

 private:
  /* Debug Info */
  llvm::DebugInfoFinder debugInfoFinder;

  /* Mapping from DI Local Variable to LLVM variable */
  typedef std::multimap<const llvm::DILocalVariable *, refmode_t> LocalVarMap;
  LocalVarMap localVars;

  /* List of DI Local Variables that correspond to undefined addresses */
  std::list<const llvm::DILocalVariable *> undefVars;

  /* Refmode Engine */
  RefmodeEngine &refmEngine;

  /* Cache of processed nodes */
  std::map<const llvm::MDNode *, refmode_t> nodeIds;
};

#endif /* DEBUG_INFO_PROCESSOR_IMPL_HPP__ */
