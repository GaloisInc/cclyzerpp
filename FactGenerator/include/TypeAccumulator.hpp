#ifndef TYPE_ACCUMULATOR_HPP__
#define TYPE_ACCUMULATOR_HPP__

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>

#include <boost/unordered_set.hpp>

namespace cclyzer::llvm_utils {
class TypeAccumulator;
}  // namespace cclyzer::llvm_utils

class cclyzer::llvm_utils::TypeAccumulator {
 public:
  typedef boost::unordered_set<const llvm::Type *> container_t;
  using iterator = container_t::iterator;
  using const_iterator = container_t::const_iterator;

  TypeAccumulator() = default;
  ~TypeAccumulator() = default;

  template <typename Iterator>
  void accumulate(Iterator first, Iterator last) {
    // Visit every type
    for (Iterator it = first; it != last; ++it) {
      // Iterator must be over LLVM Types
      const auto *type = static_cast<const llvm::Type *>(*it);

      // Process type
      visitType(type);
    }
  }

  // Iterator over all collected types

  auto begin() -> iterator { return types.begin(); }
  auto end() -> iterator { return types.end(); }

  [[nodiscard]] auto begin() const -> const_iterator { return types.begin(); }
  [[nodiscard]] auto end() const -> const_iterator { return types.end(); }

  // Visitor-like methods (do not yet implement the visitor pattern)

  void visitType(const llvm::Type *elementType) {
    if (types.count(elementType) != 0) return;

    // Add new type
    types.insert(elementType);

    // Nothing else needs to be done for simple types
    if (isPrimitiveType(elementType) || elementType->isIntegerTy()) return;

    // Recurse into component types if need be
    if (elementType->isArrayTy()) {
      visitType(elementType->getArrayElementType());
    } else if (elementType->isPointerTy()) {
#if LLVM_VERSION_MAJOR > 14
      if (!llvm::cast<llvm::PointerType>(elementType)->isOpaque()) {
        visitType(elementType->getPointerElementType());
      }
#else
      visitType(elementType->getPointerElementType());
#endif
    } else if (elementType->isStructTy()) {
      visitStructType(elementType);
    } else if (elementType->isVectorTy()) {
#if LLVM_VERSION_MAJOR > 10
      visitType(llvm::cast<llvm::VectorType>(elementType)->getElementType());
#else
      visitType(elementType->getVectorElementType());
#endif
    } else if (elementType->isFunctionTy()) {
      visitFunctionType(elementType);
    } else {
      llvm::errs() << "Unrecognized type: ";
      elementType->print(llvm::errs());
      llvm::errs() << "\n";
    }
  }

  void visitStructType(const llvm::Type *structType) {
    using llvm::StructType;
    const auto *strTy = llvm::cast<StructType>(structType);

    if (!strTy->isOpaque())
      for (size_t i = 0; i < strTy->getStructNumElements(); i++)
        visitType(strTy->getStructElementType(i));
  }

  void visitFunctionType(const llvm::Type *funcType) {
    using llvm::FunctionType;
    const auto *funcTy = llvm::dyn_cast<FunctionType>(funcType);

    visitType(funcTy->getReturnType());

    for (size_t i = 0; i < funcType->getFunctionNumParams(); i++)
      visitType(funcTy->getFunctionParamType(i));
  }

 protected:
  auto isPrimitiveType(const llvm::Type *type) -> bool {
    return type->isVoidTy() || type->isHalfTy() || type->isFloatTy() ||
           type->isDoubleTy() || type->isX86_FP80Ty() || type->isFP128Ty() ||
           type->isPPC_FP128Ty() || type->isLabelTy() || type->isMetadataTy() ||
           type->isX86_MMXTy();
  }

 private:
  container_t types;
};

#endif /* TYPE_ACCUMULATOR_HPP__ */
