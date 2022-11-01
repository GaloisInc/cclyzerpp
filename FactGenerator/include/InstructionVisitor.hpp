#ifndef INSTR_VISITOR_H__
#define INSTR_VISITOR_H__

#include <llvm/IR/InstVisitor.h>

#include <string>

#include "FactGenerator.hpp"
#include "RefmodeEngine.hpp"

// Add to namespace
namespace cclyzer {
class InstructionVisitor;
}

// Processor of instructions
class cclyzer::InstructionVisitor
    : public llvm::InstVisitor<InstructionVisitor> {
  friend class FactGenerator;

 public:
  InstructionVisitor(FactGenerator &generator, const llvm::Module &M)
      : gen(generator), module(M) {}

  //--------------------------------------------------------------------------
  // Instruction Visitor methods
  //--------------------------------------------------------------------------

  // Binary Operations

  void visitAdd(const llvm::BinaryOperator &);
  void visitFAdd(const llvm::BinaryOperator &);
  void visitSub(const llvm::BinaryOperator &);
  void visitFSub(const llvm::BinaryOperator &);
  void visitMul(const llvm::BinaryOperator &);
  void visitFMul(const llvm::BinaryOperator &);
  void visitSDiv(const llvm::BinaryOperator &);
  void visitFDiv(const llvm::BinaryOperator &);
  void visitUDiv(const llvm::BinaryOperator &);
  void visitSRem(const llvm::BinaryOperator &);
  void visitFRem(const llvm::BinaryOperator &);
  void visitURem(const llvm::BinaryOperator &);

  // Bitwise Binary Operations

  void visitShl(const llvm::BinaryOperator &);
  void visitLShr(const llvm::BinaryOperator &);
  void visitAShr(const llvm::BinaryOperator &);
  void visitAnd(const llvm::BinaryOperator &);
  void visitOr(const llvm::BinaryOperator &);
  void visitXor(const llvm::BinaryOperator &);

  // Conversion Operations

  void visitTruncInst(const llvm::TruncInst &);
  void visitZExtInst(const llvm::ZExtInst &);
  void visitSExtInst(const llvm::SExtInst &);
  void visitFPTruncInst(const llvm::FPTruncInst &);
  void visitFPExtInst(const llvm::FPExtInst &);
  void visitFPToUIInst(const llvm::FPToUIInst &);
  void visitFPToSIInst(const llvm::FPToSIInst &);
  void visitUIToFPInst(const llvm::UIToFPInst &);
  void visitSIToFPInst(const llvm::SIToFPInst &);
  void visitPtrToIntInst(const llvm::PtrToIntInst &);
  void visitIntToPtrInst(const llvm::IntToPtrInst &);
  void visitBitCastInst(const llvm::BitCastInst &);

  // Terminator Instructions

  void visitReturnInst(const llvm::ReturnInst &);
  void visitBranchInst(const llvm::BranchInst &);
  void visitSwitchInst(const llvm::SwitchInst &);
  void visitIndirectBrInst(const llvm::IndirectBrInst &);
  void visitInvokeInst(const llvm::InvokeInst &);
  void visitResumeInst(const llvm::ResumeInst &);
  void visitUnreachableInst(const llvm::UnreachableInst &);

  // Aggregate Operations

  void visitInsertValueInst(const llvm::InsertValueInst &);
  void visitExtractValueInst(const llvm::ExtractValueInst &);

  // Memory Operations

  void visitAllocaInst(const llvm::AllocaInst &);
  void visitLoadInst(const llvm::LoadInst &);
  void visitStoreInst(const llvm::StoreInst &);
  void visitAtomicCmpXchgInst(const llvm::AtomicCmpXchgInst &);
  void visitAtomicRMWInst(const llvm::AtomicRMWInst &);
  void visitFenceInst(const llvm::FenceInst &);
  void visitGetElementPtrInst(const llvm::GetElementPtrInst &);

  // Other

  void visitICmpInst(const llvm::ICmpInst &);
  void visitFCmpInst(const llvm::FCmpInst &);
  void visitPHINode(const llvm::PHINode &);
  void visitSelectInst(const llvm::SelectInst &);
  void visitLandingPadInst(const llvm::LandingPadInst &);
  void visitCallInst(const llvm::CallInst &);
  void visitDbgDeclareInst(const llvm::DbgDeclareInst &);
  void visitVAArgInst(const llvm::VAArgInst &);

  // Vector Operations

  void visitExtractElementInst(const llvm::ExtractElementInst &);
  void visitInsertElementInst(const llvm::InsertElementInst &);
  void visitShuffleVectorInst(const llvm::ShuffleVectorInst &);

  // 'default' case
  void visitInstruction(const llvm::Instruction &I);

 protected:
  //--------------------------------------------------------------------------
  // Template methods that handle many similar instructions
  //--------------------------------------------------------------------------

  // Process cast instruction
  template <typename T>
  inline void _visitCastInst(const llvm::CastInst &instr) {
    typedef T pred;

    // Record instruction entity
    const refmode_t iref = recordInstruction(pred::instr, instr);

    // Record right-hand-side operand
    writeInstrOperand(pred::from_operand, iref, instr.getOperand(0));

    // Record type being casted to
    gen.writeFact(pred::to_type, iref, gen.recordType(instr.getType()));
  }

  // Process binary instruction
  template <typename T>
  inline void _visitBinaryInst(const llvm::BinaryOperator &instr) {
    using pred = T;

    // Record instruction entity
    const refmode_t iref = recordInstruction(pred::instr, instr);

    writeInstrOperand(pred::first_operand, iref, instr.getOperand(0));
    writeInstrOperand(pred::second_operand, iref, instr.getOperand(1));
    writeOptimizationInfo(iref, &instr);
  }

  // Record atomic instruction info
  template <typename P, typename I>
  void writeAtomicInfo(const refmode_t &iref, const I &instr) {
    using namespace llvm;

    AtomicOrdering order = instr.getOrdering();

    const refmode_t atomic = gen.refmode<AtomicOrdering>(order);

    // default synchScope: crossthread
    if (instr.getSyncScopeID() == SyncScope::SingleThread) {
      gen.writeFact(predicates::instr::flag, iref, "singlethread");
    }

    if (!atomic.empty()) gen.writeFact(P::ordering, iref, atomic);
  }

  //--------------------------------------------------------------------------
  // Auxiliary methods that record instruction entities and operands
  //--------------------------------------------------------------------------

  // Record instruction entity and generate refmode
  auto recordInstruction(const predicates::pred_t &, const llvm::Instruction &)
      -> refmode_t;

  // Record value and generate refmode
  auto recordValue(const llvm::Value *) -> refmode_t;

  // Record instruction operand and generate refmode
  auto writeInstrOperand(
      const predicates::pred_t &predicate,
      const refmode_t &instr,
      const llvm::Value *Operand) -> refmode_t;

  auto writeInstrOperand(
      const predicates::pred_t &predicate,
      const refmode_t &instr,
      const llvm::Value *Value,
      unsigned index) -> refmode_t;

  //--------------------------------------------------------------------------
  // Other auxiliary methods
  //--------------------------------------------------------------------------

  // Record several facts regarding optimizations
  void writeOptimizationInfo(const refmode_t &, const llvm::User *);

  // Record `atomicrmw` binary operator
  void writeAtomicRMWOp(const refmode_t &, llvm::AtomicRMWInst::BinOp);

  // Record predicate for comparison instruction
  void writeCondition(
      const predicates::pred_t &, const refmode_t &, const llvm::CmpInst &);

 private:
  /* Instance of outer fact-generator */
  FactGenerator &gen;

  /* Associated LLVM module */
  const llvm::Module &module;
};

#endif
