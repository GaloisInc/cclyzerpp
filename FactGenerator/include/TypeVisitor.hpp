// Forward decls
namespace llvm {
class ArrayType;
class DataLayout;
class FunctionType;
class PointerType;
class StructType;
class Type;
class VectorType;
}  // namespace llvm

namespace cclyzer {
// Forward declaration
class FactGenerator;

// Processor of type entities
class TypeVisitor {
 public:
  TypeVisitor(FactGenerator &generator, const llvm::DataLayout &DL)
      : gen(generator), layout(DL) {}

  /* Type visitor methods */

  void visitType(const llvm::Type *);
  void visitPointerType(const llvm::PointerType *);
  void visitArrayType(const llvm::ArrayType *);
  void visitStructType(const llvm::StructType *);
  void visitFunctionType(const llvm::FunctionType *);
  void visitVectorType(const llvm::VectorType *);

 private:
  /* Instance of outer CSV generator */
  FactGenerator &gen;

  /**
   * Data layout. To compute the byte size of each type, we need a
   * data layout object, that is associated with the given LLVM
   * module.
   */
  const llvm::DataLayout &layout;
};
}  // namespace cclyzer
