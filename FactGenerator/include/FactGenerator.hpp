#ifndef CSV_GENERATOR_H__
#define CSV_GENERATOR_H__

#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

#include <boost/flyweight.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <set>
#include <string>

#include "ContextSensitivity.hpp"
#include "Demangler.hpp"
#include "FactWriter.hpp"
#include "ForwardingFactWriter.hpp"
#include "PredicateGroups.hpp"
#include "RefmodeEngine.hpp"
#include "Signatures.hpp"

namespace cclyzer {
class FactGenerator;
}

class cclyzer::FactGenerator : private RefmodeEngine,
                               private Demangler,
                               private ForwardingFactWriter {
  friend class InstructionVisitor;
  friend class TypeVisitor;
  using RefmodeEngine::refmode;

 public:
  /* Non-copyable */
  FactGenerator(FactGenerator const &) = delete;
  auto operator=(FactGenerator const &) -> FactGenerator & = delete;

  /* Get fact generator instance for a given fact writer */
  static auto getInstance(FactWriter &) -> FactGenerator &;

  /* Fact Writing Methods */
  auto writeConstant(const llvm::Constant &) -> refmode_t;
  auto writeAsm(const llvm::InlineAsm &) -> refmode_t;

  // std::map<boost::flyweight<std::string>, const llvm::Value *>

  auto processModule(
      const llvm::Module &Mod,
      const std::string &path,
      const llvm::Optional<boost::filesystem::path> &signatures,
      const ContextSensitivity &sensitivity)
      -> std::map<boost::flyweight<std::string>, const llvm::Value *>;
  void writeLocalVariables();
  void writeTypes(const llvm::DataLayout &layout);

 protected:
  /* Common type aliases */
  using type_cache_t = boost::unordered_map<std::string, const llvm::Type *>;
  using pred_t = predicates::pred_t;

  /* Constructor must initialize output file streams */
  FactGenerator(FactWriter &writer) : ForwardingFactWriter(writer) {}

  /* Recording variables and types */
  void recordVariable(const std::string &id, const llvm::Type *type) {
    variableTypes[id] = type;
  }

  auto recordType(const llvm::Type *type) -> refmode_t {
    types.insert(type);
    return refmode<llvm::Type>(*type);
  }

  /* Auxiliary fact writing methods */

  template <typename PredGroup, class ConstantType>
  void writeConstantWithOperands(const ConstantType &, const refmode_t &);

  void writeFunction(const llvm::Function &, const refmode_t &);
  void writeConstantArray(const llvm::ConstantArray &, const refmode_t &);
  void writeConstantStruct(const llvm::ConstantStruct &, const refmode_t &);
  void writeConstantVector(const llvm::ConstantVector &, const refmode_t &);
  void writeConstantExpr(const llvm::ConstantExpr &, const refmode_t &);
  void writeGlobalAlias(const llvm::GlobalAlias &, const refmode_t &);
  void writeGlobalVar(const llvm::GlobalVariable &, const refmode_t &);

  std::map<boost::flyweight<std::string>, const llvm::Value *> result_map_;

 private:
  /* Initialize output file streams */
  void initStreams();

  auto processSignatures(const boost::filesystem::path &signatures)
      -> std::vector<std::tuple<std::string, std::regex, llvm::json::Array>>;
  void emitSignatures(
      const std::string &func, const llvm::json::Array &signatures);

  /* Caches for variable types */
  type_cache_t variableTypes;

  /* Auxiliary methods */

  boost::unordered_set<const llvm::Type *> types;

  /* A RAII object for recording the current context. */
  struct Context {
    Context(FactGenerator &generator, const llvm::Value &v) : gen(generator) {
      gen.enterContext(v);
    }

    ~Context() { gen.exitContext(); }

   private:
    FactGenerator &gen;
  };

  struct ModuleContext {
    ModuleContext(
        FactGenerator &generator,
        const llvm::Module &m,
        const std::string &path)
        : gen(generator) {
      gen.enterModule(m, path);
    }

    ~ModuleContext() { gen.exitModule(); }

   private:
    FactGenerator &gen;
  };
};

#endif
