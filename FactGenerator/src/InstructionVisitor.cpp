#include "InstructionVisitor.hpp"

#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Operator.h>

#include <cassert>
#include <string>

#include "MalformedModule.hpp"
#include "PredicateGroups.hpp"
#include "Unknown.hpp"

namespace pred = cclyzer::predicates;

using cclyzer::InstructionVisitor;
using llvm::dyn_cast;
using llvm::isa;
using pred::pred_t;
using std::string;

//------------------------------------------------------------------------------
// Generic instruction writing utility methods
//------------------------------------------------------------------------------

auto InstructionVisitor::recordInstruction(
    const pred_t &pred, const llvm::Instruction &instr) -> cclyzer::refmode_t {
  // Get refmode of enclosing instruction
  refmode_t iref = gen.refmode<llvm::Instruction>(instr);

  gen.writeFact(pred, iref);
  return iref;
}

auto InstructionVisitor::recordValue(const llvm::Value *Val)
    -> cclyzer::refmode_t {
  refmode_t refmode;
  const llvm::Type *type = Val->getType();

  if (const auto *c = dyn_cast<llvm::Constant>(Val)) {
    // Compute refmode for constant value.

    // !! Note: We avoid sharing of different constants of the
    // same value by prepending their refmodes with a unique
    // identifier that consists of the instruction id together
    // with an auto-incrementing counter (which is used
    // exclusively for constants) !!
    refmode = gen.writeConstant(*c);
  } else if (const auto *asmVal = dyn_cast<llvm::InlineAsm>(Val)) {
    // Compute refmode for ASM string
    refmode = gen.writeAsm(*asmVal);
  } else {
    // Compute refmode for variable value
    refmode = gen.refmode<llvm::Value>(*Val);

    // Record variable value
    gen.recordVariable(refmode, type);
  }

  return refmode;
}

auto InstructionVisitor::writeInstrOperand(
    const pred_t &predicate,  // the operand predicate
    const refmode_t &instr,   // the instruction refmode
    const llvm::Value *operand) -> cclyzer::refmode_t  // the operand value
{
  refmode_t refmode = recordValue(operand);

  // Write value fact
  gen.writeFact(predicate, instr, refmode);

  return refmode;
}

auto InstructionVisitor::writeInstrOperand(
    const pred_t &predicate,               // the operand predicate
    const refmode_t &instr,                // the instruction refmode
    const llvm::Value *operand,            // the operand value
    unsigned index) -> cclyzer::refmode_t  // the operand index
{
  refmode_t refmode = recordValue(operand);

  // Write value fact
  gen.writeFact(predicate, instr, index, refmode);

  return refmode;
}

//------------------------------------------------------------------------------
// Fact-writing methods for each instruction type
//------------------------------------------------------------------------------

void InstructionVisitor::visitTruncInst(const llvm::TruncInst &I) {
  _visitCastInst<pred::trunc>(I);
}

void InstructionVisitor::visitZExtInst(const llvm::ZExtInst &I) {
  _visitCastInst<pred::zext>(I);
}

void InstructionVisitor::visitSExtInst(const llvm::SExtInst &I) {
  _visitCastInst<pred::sext>(I);
}

void InstructionVisitor::visitFPTruncInst(const llvm::FPTruncInst &I) {
  _visitCastInst<pred::fptrunc>(I);
}

void InstructionVisitor::visitFPExtInst(const llvm::FPExtInst &I) {
  _visitCastInst<pred::fpext>(I);
}

void InstructionVisitor::visitFPToUIInst(const llvm::FPToUIInst &I) {
  _visitCastInst<pred::fptoui>(I);
}

void InstructionVisitor::visitFPToSIInst(const llvm::FPToSIInst &I) {
  _visitCastInst<pred::fptosi>(I);
}

void InstructionVisitor::visitUIToFPInst(const llvm::UIToFPInst &I) {
  _visitCastInst<pred::uitofp>(I);
}

void InstructionVisitor::visitSIToFPInst(const llvm::SIToFPInst &I) {
  _visitCastInst<pred::sitofp>(I);
}

void InstructionVisitor::visitPtrToIntInst(const llvm::PtrToIntInst &I) {
  _visitCastInst<pred::ptrtoint>(I);
}

void InstructionVisitor::visitIntToPtrInst(const llvm::IntToPtrInst &I) {
  _visitCastInst<pred::inttoptr>(I);
}

void InstructionVisitor::visitBitCastInst(const llvm::BitCastInst &I) {
  _visitCastInst<pred::bitcast>(I);
}

void InstructionVisitor::visitAdd(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::add>(BI);
}

void InstructionVisitor::visitFAdd(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::fadd>(BI);
}

void InstructionVisitor::visitSub(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::sub>(BI);
}

void InstructionVisitor::visitFSub(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::fsub>(BI);
}

void InstructionVisitor::visitMul(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::mul>(BI);
}

void InstructionVisitor::visitFMul(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::fmul>(BI);
}

void InstructionVisitor::visitSDiv(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::sdiv>(BI);
}

void InstructionVisitor::visitFDiv(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::fdiv>(BI);
}

void InstructionVisitor::visitUDiv(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::udiv>(BI);
}

void InstructionVisitor::visitSRem(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::srem>(BI);
}

void InstructionVisitor::visitFRem(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::frem>(BI);
}

void InstructionVisitor::visitURem(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::urem>(BI);
}

void InstructionVisitor::visitShl(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::shl>(BI);
}

void InstructionVisitor::visitLShr(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::lshr>(BI);
}

void InstructionVisitor::visitAShr(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::ashr>(BI);
}

void InstructionVisitor::visitAnd(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::and_>(BI);
}

void InstructionVisitor::visitOr(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::or_>(BI);
}

void InstructionVisitor::visitXor(const llvm::BinaryOperator &BI) {
  _visitBinaryInst<pred::xor_>(BI);
}

void InstructionVisitor::visitReturnInst(const llvm::ReturnInst &RI) {
  refmode_t iref = recordInstruction(pred::ret::instr, RI);

  if (RI.getReturnValue() != nullptr) {  // with returned value
    writeInstrOperand(pred::ret::operand, iref, RI.getReturnValue());
  } else {  // w/o returned value
    gen.writeFact(pred::ret::void_, iref);
  }
}

void InstructionVisitor::visitBranchInst(const llvm::BranchInst &BI) {
  refmode_t iref = recordInstruction(pred::br::instr, BI);

  if (BI.isConditional()) {  // conditional branch
    // br i1 <cond>, label <iftrue>, label <iffalse>
    gen.writeFact(pred::br::cond, iref);

    // Condition Operand
    writeInstrOperand(pred::br::condition, iref, BI.getCondition());

    // 'iftrue' and 'iffalse' labels
    writeInstrOperand(pred::br::true_label, iref, BI.getOperand(1));
    writeInstrOperand(pred::br::false_label, iref, BI.getOperand(2));
  } else {  // unconditional branch
    // br label <dest>
    gen.writeFact(pred::br::uncond, iref);
    writeInstrOperand(pred::br::uncond_label, iref, BI.getOperand(0));
  }
}

void InstructionVisitor::visitSwitchInst(const llvm::SwitchInst &SI) {
  // switch <intty> <value>, label <defaultdest> [ <intty> <val>, label <dest>
  // ... ]
  refmode_t iref = recordInstruction(pred::switch_::instr, SI);

  writeInstrOperand(pred::switch_::operand, iref, SI.getOperand(0));
  writeInstrOperand(pred::switch_::default_label, iref, SI.getOperand(1));

  // 'case list' [constant, label]
  unsigned index = 0;

  for (auto Case : SI.cases()) {
    writeInstrOperand(
        pred::switch_::case_value, iref, Case.getCaseValue(), index);

    writeInstrOperand(
        pred::switch_::case_label, iref, Case.getCaseSuccessor(), index++);
  }

  gen.writeFact(pred::switch_::ncases, iref, SI.getNumCases());
}

void InstructionVisitor::visitIndirectBrInst(const llvm::IndirectBrInst &IBR) {
  // indirectbr <somety>* <address>, [ label <dest1>, label <dest2>, ... ]
  refmode_t iref = recordInstruction(pred::indirectbr::instr, IBR);

  // 'label' list
  for (unsigned i = 1; i < IBR.getNumOperands(); ++i) {
    writeInstrOperand(pred::indirectbr::label, iref, IBR.getOperand(i), i - 1);
  }

  gen.writeFact(pred::indirectbr::nlabels, iref, IBR.getNumOperands() - 1);
  writeInstrOperand(pred::indirectbr::address, iref, IBR.getOperand(0));
}

void InstructionVisitor::visitInvokeInst(const llvm::InvokeInst &II) {
  refmode_t iref = recordInstruction(pred::invoke::instr, II);

#if LLVM_VERSION_MAJOR > 10
  const llvm::Value *invokeOp = II.getCalledOperand();
#else
  const llvm::Value *invokeOp = II.getCalledValue();
#endif

  // invoke instruction function (also records type)
  writeInstrOperand(pred::invoke::func_operand, iref, invokeOp);

  // actual args
#if LLVM_VERSION_MAJOR > 13
  for (unsigned op = 0; op < II.arg_size(); ++op) {
#else
  for (unsigned op = 0; op < II.getNumArgOperands(); ++op) {
#endif
    writeInstrOperand(pred::invoke::arg, iref, II.getArgOperand(op), op);
  }

  writeInstrOperand(pred::invoke::normal_label, iref, II.getNormalDest());
  writeInstrOperand(pred::invoke::exception_label, iref, II.getUnwindDest());

  // TODO: Why not CallingConv::C
  if (II.getCallingConv() != llvm::CallingConv::C) {
    refmode_t cconv = gen.refmode(II.getCallingConv());
    gen.writeFact(pred::invoke::calling_conv, iref, cconv);
  }
}

void InstructionVisitor::visitResumeInst(const llvm::ResumeInst &RI) {
  refmode_t iref = recordInstruction(pred::resume::instr, RI);
  writeInstrOperand(pred::resume::operand, iref, RI.getValue());
}

void InstructionVisitor::visitUnreachableInst(const llvm::UnreachableInst &I) {
  recordInstruction(pred::unreachable::instr, I);
}

void InstructionVisitor::visitAllocaInst(const llvm::AllocaInst &AI) {
  refmode_t iref = recordInstruction(pred::alloca::instr, AI);
  refmode_t type = gen.recordType(AI.getAllocatedType());

  gen.writeFact(pred::alloca::type, iref, type);

  if (AI.isArrayAllocation()) {
    writeInstrOperand(pred::alloca::size, iref, AI.getArraySize());
  }

#if LLVM_VERSION_MAJOR > 14
  gen.writeFact(pred::alloca::alignment, iref, llvm::Log2(AI.getAlign()));
#else
  if (AI.getAlignment() != 0U) {
    gen.writeFact(pred::alloca::alignment, iref, AI.getAlignment());
  }
#endif
}

void InstructionVisitor::visitLoadInst(const llvm::LoadInst &LI) {
  refmode_t iref = recordInstruction(pred::load::instr, LI);

  writeInstrOperand(pred::load::address, iref, LI.getPointerOperand());

  if (LI.isAtomic()) {
    writeAtomicInfo<pred::load>(iref, LI);
  }

#if LLVM_VERSION_MAJOR > 14
  gen.writeFact(pred::load::alignment, iref, llvm::Log2(LI.getAlign()));
#else
  if (LI.getAlignment() != 0U) {
    gen.writeFact(pred::load::alignment, iref, LI.getAlignment());
  }
#endif

  if (LI.isVolatile()) {
    gen.writeFact(pred::load::is_volatile, iref);
  }
}

void InstructionVisitor::visitVAArgInst(const llvm::VAArgInst &VI) {
  refmode_t iref = recordInstruction(pred::va_arg::instr, VI);
  refmode_t type = gen.recordType(VI.getType());

  gen.writeFact(pred::va_arg::type, iref, type);
  writeInstrOperand(pred::va_arg::va_list, iref, VI.getPointerOperand());
}

void InstructionVisitor::visitExtractValueInst(
    const llvm::ExtractValueInst &EVI) {
  refmode_t iref = recordInstruction(pred::extractvalue::instr, EVI);

  // Aggregate Operand
  writeInstrOperand(pred::extractvalue::base, iref, EVI.getOperand(0));

  // Constant Indices
  int index = 0;

  for (const unsigned *i = EVI.idx_begin(), *e = EVI.idx_end(); i != e; ++i) {
    gen.writeFact(pred::extractvalue::index, iref, index++, *i);
  }

  gen.writeFact(pred::extractvalue::nindices, iref, EVI.getNumIndices());
}

void InstructionVisitor::visitStoreInst(const llvm::StoreInst &SI) {
  refmode_t iref = recordInstruction(pred::store::instr, SI);

  writeInstrOperand(pred::store::value, iref, SI.getValueOperand());
  writeInstrOperand(pred::store::address, iref, SI.getPointerOperand());

  if (SI.isAtomic()) {
    writeAtomicInfo<pred::store>(iref, SI);
  }

#if LLVM_VERSION_MAJOR > 14
  gen.writeFact(pred::store::alignment, iref, llvm::Log2(SI.getAlign()));
#else
  if (SI.getAlignment() != 0U) {
    gen.writeFact(pred::store::alignment, iref, SI.getAlignment());
  }
#endif

  if (SI.isVolatile()) {
    gen.writeFact(pred::store::is_volatile, iref);
  }
}

void InstructionVisitor::visitAtomicCmpXchgInst(
    const llvm::AtomicCmpXchgInst &AXI) {
  refmode_t iref = recordInstruction(pred::cmpxchg::instr, AXI);

  writeInstrOperand(pred::cmpxchg::address, iref, AXI.getPointerOperand());
  writeInstrOperand(pred::cmpxchg::cmp_value, iref, AXI.getCompareOperand());
  writeInstrOperand(pred::cmpxchg::new_value, iref, AXI.getNewValOperand());

  if (AXI.isVolatile()) {
    gen.writeFact(pred::cmpxchg::is_volatile, iref);
  }

  llvm::AtomicOrdering successOrd = AXI.getSuccessOrdering();
  llvm::AtomicOrdering failureOrd = AXI.getFailureOrdering();

  string successOrdStr = gen.refmode<llvm::AtomicOrdering>(successOrd);
  string failureOrdStr = gen.refmode<llvm::AtomicOrdering>(failureOrd);

  // default synchScope: crossthread
  if (AXI.getSyncScopeID() == llvm::SyncScope::SingleThread) {
    gen.writeFact(pred::instr::flag, iref, "singlethread");
  }

  if (!successOrdStr.empty()) {
    gen.writeFact(pred::cmpxchg::ordering, iref, successOrdStr);
  }

  if (!failureOrdStr.empty()) {  // change schema ordering preds
    gen.writeFact(pred::cmpxchg::ordering, iref, failureOrdStr);
  }

  // TODO: type?
}

void InstructionVisitor::visitAtomicRMWInst(const llvm::AtomicRMWInst &AWI) {
  refmode_t iref = recordInstruction(pred::atomicrmw::instr, AWI);

  writeInstrOperand(pred::atomicrmw::address, iref, AWI.getPointerOperand());
  writeInstrOperand(pred::atomicrmw::value, iref, AWI.getValOperand());

  if (AWI.isVolatile()) {
    gen.writeFact(pred::atomicrmw::is_volatile, iref);
  }

  writeAtomicRMWOp(iref, AWI.getOperation());
  writeAtomicInfo<pred::atomicrmw>(iref, AWI);
}

void InstructionVisitor::visitFenceInst(const llvm::FenceInst &FI) {
  refmode_t iref = recordInstruction(pred::fence::instr, FI);

  // fence [singleThread]  <ordering>
  writeAtomicInfo<pred::fence>(iref, FI);
}

void InstructionVisitor::visitGetElementPtrInst(
    const llvm::GetElementPtrInst &GEP) {
  refmode_t iref = recordInstruction(pred::getelementptr::instr, GEP);
  writeInstrOperand(pred::getelementptr::base, iref, GEP.getPointerOperand());

  for (unsigned index = 1; index < GEP.getNumOperands(); ++index) {
    const llvm::Value *GepOperand = GEP.getOperand(index);

    refmode_t opref = writeInstrOperand(
        pred::getelementptr::index, iref, GepOperand, index - 1);

    if (const auto *c = dyn_cast<llvm::Constant>(GepOperand)) {
      if (c->getUniqueInteger().isIntN(16)) {
        // Compute integer string representation
        // TODO(lb): Compute both signed and unsigned representations
#if LLVM_VERSION_MAJOR > 12
        llvm::SmallString<16> s;
        c->getUniqueInteger().toStringUnsigned(s, 10);
        std::string int_value = std::string(s);
#else
        std::string int_value = c->getUniqueInteger().toString(10, true);
#endif

        // Write constant to integer fact
        gen.writeFact(pred::constant::to_integer, opref, int_value);
      }
    }
  }

  gen.writeFact(pred::getelementptr::nindices, iref, GEP.getNumIndices());

  if (GEP.isInBounds()) {
    gen.writeFact(pred::getelementptr::inbounds, iref);
  }
}

void InstructionVisitor::visitPHINode(const llvm::PHINode &PHI) {
  // <result> = phi <ty> [ <val0>, <label0>], ...
  refmode_t iref = recordInstruction(pred::phi::instr, PHI);
  refmode_t type = gen.recordType(PHI.getType());

  // type
  gen.writeFact(pred::phi::type, iref, type);

  for (unsigned op = 0; op < PHI.getNumIncomingValues(); ++op) {
    writeInstrOperand(
        pred::phi::pair_value, iref, PHI.getIncomingValue(op), op);
    writeInstrOperand(
        pred::phi::pair_label, iref, PHI.getIncomingBlock(op), op);
  }

  gen.writeFact(pred::phi::npairs, iref, PHI.getNumIncomingValues());
}

void InstructionVisitor::visitSelectInst(const llvm::SelectInst &SI) {
  refmode_t iref = recordInstruction(pred::select::instr, SI);

  // Condition and operands
  writeInstrOperand(pred::select::condition, iref, SI.getOperand(0));
  writeInstrOperand(pred::select::first_operand, iref, SI.getOperand(1));
  writeInstrOperand(pred::select::second_operand, iref, SI.getOperand(2));
}

void InstructionVisitor::visitInsertValueInst(
    const llvm::InsertValueInst &IVI) {
  refmode_t iref = recordInstruction(pred::insertvalue::instr, IVI);

  writeInstrOperand(pred::insertvalue::base, iref, IVI.getOperand(0));
  writeInstrOperand(pred::insertvalue::value, iref, IVI.getOperand(1));

  // Constant Indices
  int index = 0;

  for (const unsigned *i = IVI.idx_begin(), *e = IVI.idx_end(); i != e; ++i) {
    gen.writeFact(pred::insertvalue::index, iref, index++, *i);
  }

  gen.writeFact(pred::insertvalue::nindices, iref, IVI.getNumIndices());
}

void InstructionVisitor::visitLandingPadInst(const llvm::LandingPadInst &LI) {
  refmode_t iref = recordInstruction(pred::landingpad::instr, LI);
  refmode_t type = gen.recordType(LI.getType());

  gen.writeFact(pred::landingpad::type, iref, type);

  // cleanup
  if (LI.isCleanup()) {
    gen.writeFact(pred::landingpad::cleanup, iref);
  }

  // #clauses
  for (unsigned i = 0; i < LI.getNumClauses(); ++i) {
    const pred_t &pred_clause = LI.isCatch(i) ? pred::landingpad::catch_clause
                                              : pred::landingpad::filter_clause;

    // Record clause
    writeInstrOperand(pred_clause, iref, LI.getClause(i), i);
  }

  gen.writeFact(pred::landingpad::nclauses, iref, LI.getNumClauses());
}

void InstructionVisitor::visitCallInst(const llvm::CallInst &CI) {
  refmode_t iref = recordInstruction(pred::call::instr, CI);

  // Call instructions are now divided into direct call instructions
  // (which have a statically known function as target) and indirect
  // ones (via function pointers). ASM calls cannot be represented
  // as indirect ones, since their function operand is not a
  // variable; neither can they be represented as direct
  // instructions due to the constraint that all direct calls must
  // be able to determine the function to be called.

#if LLVM_VERSION_MAJOR > 10
  const llvm::Value *callOp = CI.getCalledOperand();
#else
  const llvm::Value *callOp = CI.getCalledValue();
#endif

  // call instruction function (also records type)
  writeInstrOperand(pred::call::func_operand, iref, callOp);

#if LLVM_VERSION_MAJOR > 13
  for (unsigned op = 0; op < CI.arg_size(); ++op) {
#else
  for (unsigned op = 0; op < CI.getNumArgOperands(); ++op) {
#endif
    writeInstrOperand(pred::call::arg, iref, CI.getArgOperand(op), op);
  }

  if (CI.isTailCall()) {
    gen.writeFact(pred::call::tail_opt, iref);
  }

  if (CI.getCallingConv() != llvm::CallingConv::C) {
    refmode_t cconv = gen.refmode(CI.getCallingConv());
    gen.writeFact(pred::call::calling_conv, iref, cconv);
  }
}

void InstructionVisitor::visitDbgDeclareInst(const llvm::DbgDeclareInst &DDI) {
  // First visit it as a generic call instruction
  InstructionVisitor::visitCallInst(static_cast<const llvm::CallInst &>(DDI));

  // TODO Move the entire debug location logic to debuginfo_Variables.cpp
  const llvm::Value *address = DDI.getAddress();

  // Skip undefined values
  if (isa<llvm::UndefValue>(address)) {
    return;
  }

  // Obtain the refmode of the local variable
  refmode_t refmode = gen.refmode<llvm::Value>(*address);

  // Record source variable name
  if (const llvm::DILocalVariable *var = DDI.getVariable()) {
#if LLVM_VERSION_MAJOR > 10
    string name = var->getName().str();
#else
    string name = var->getName();
#endif

    gen.writeFact(pred::variable::source_name, refmode, name);
  }

  // Get debug location if available
  if (const llvm::DebugLoc &location = DDI.getDebugLoc()) {
    unsigned line = location.getLine();
    unsigned column = location.getCol();

    gen.writeFact(pred::variable::pos, refmode, line, column);
  }
}

void InstructionVisitor::visitICmpInst(const llvm::ICmpInst &I) {
  refmode_t iref = recordInstruction(pred::icmp::instr, I);

  // Record condition
  writeCondition(pred::icmp::condition, iref, I);

  // Record operands
  writeInstrOperand(pred::icmp::first_operand, iref, I.getOperand(0));
  writeInstrOperand(pred::icmp::second_operand, iref, I.getOperand(1));
}

void InstructionVisitor::visitFCmpInst(const llvm::FCmpInst &I) {
  refmode_t iref = recordInstruction(pred::fcmp::instr, I);

  // Record condition
  writeCondition(pred::fcmp::condition, iref, I);

  // Record operands
  writeInstrOperand(pred::fcmp::first_operand, iref, I.getOperand(0));
  writeInstrOperand(pred::fcmp::second_operand, iref, I.getOperand(1));
}

void InstructionVisitor::visitExtractElementInst(
    const llvm::ExtractElementInst &EEI) {
  refmode_t iref = recordInstruction(pred::extractelement::instr, EEI);

  writeInstrOperand(pred::extractelement::base, iref, EEI.getVectorOperand());
  writeInstrOperand(pred::extractelement::index, iref, EEI.getIndexOperand());
}

void InstructionVisitor::visitInsertElementInst(
    const llvm::InsertElementInst &IEI) {
  refmode_t iref = recordInstruction(pred::insertelement::instr, IEI);

  writeInstrOperand(pred::insertelement::base, iref, IEI.getOperand(0));
  writeInstrOperand(pred::insertelement::value, iref, IEI.getOperand(1));
  writeInstrOperand(pred::insertelement::index, iref, IEI.getOperand(2));
}

void InstructionVisitor::visitShuffleVectorInst(
    const llvm::ShuffleVectorInst &SVI) {
  refmode_t iref = recordInstruction(pred::shufflevector::instr, SVI);

  writeInstrOperand(pred::shufflevector::first_vector, iref, SVI.getOperand(0));
  writeInstrOperand(
      pred::shufflevector::second_vector, iref, SVI.getOperand(1));
  writeInstrOperand(pred::shufflevector::mask, iref, SVI.getOperand(2));
}

void InstructionVisitor::visitInstruction(const llvm::Instruction &I) {
  unknown("instruction", I.getOpcodeName());
}

//------------------------------------------------------------------------------
// Auxiliary Methods for non-instructions
//------------------------------------------------------------------------------

static auto icmp_pred_string(const llvm::CmpInst &I) -> std::string {
  switch (I.getPredicate()) {
    case llvm::FCmpInst::FCMP_FALSE:
      return "false";
    case llvm::FCmpInst::FCMP_OEQ:
      return "oeq";
    case llvm::FCmpInst::FCMP_OGT:
      return "ogt";
    case llvm::FCmpInst::FCMP_OGE:
      return "oge";
    case llvm::FCmpInst::FCMP_OLT:
      return "olt";
    case llvm::FCmpInst::FCMP_OLE:
      return "ole";
    case llvm::FCmpInst::FCMP_ONE:
      return "one";
    case llvm::FCmpInst::FCMP_ORD:
      return "ord";
    case llvm::FCmpInst::FCMP_UNO:
      return "uno";
    case llvm::FCmpInst::FCMP_UEQ:
      return "ueq";
    case llvm::FCmpInst::FCMP_UGT:
      return "ugt";
    case llvm::FCmpInst::FCMP_UGE:
      return "uge";
    case llvm::FCmpInst::FCMP_ULT:
      return "ult";
    case llvm::FCmpInst::FCMP_ULE:
      return "ule";
    case llvm::FCmpInst::FCMP_UNE:
      return "une";
    case llvm::FCmpInst::FCMP_TRUE:
      return "true";

    case llvm::ICmpInst::ICMP_EQ:
      return "eq";
    case llvm::ICmpInst::ICMP_NE:
      return "ne";
    case llvm::ICmpInst::ICMP_SGT:
      return "sgt";
    case llvm::ICmpInst::ICMP_SGE:
      return "sge";
    case llvm::ICmpInst::ICMP_SLT:
      return "slt";
    case llvm::ICmpInst::ICMP_SLE:
      return "sle";
    case llvm::ICmpInst::ICMP_UGT:
      return "ugt";
    case llvm::ICmpInst::ICMP_UGE:
      return "uge";
    case llvm::ICmpInst::ICMP_ULT:
      return "ult";
    case llvm::ICmpInst::ICMP_ULE:
      return "ule";
    case llvm::ICmpInst::BAD_FCMP_PREDICATE:
      malformedModule("bad fcmp predicate");
    case llvm::ICmpInst::BAD_ICMP_PREDICATE:
      malformedModule("bad icmp predicate");
  }  // -Wswitch prevents fallthrough, no need for default case
  assert(false);
}

void InstructionVisitor::writeCondition(
    const pred_t &pred, const refmode_t &iref, const llvm::CmpInst &I) {
  gen.writeFact(pred, iref, icmp_pred_string(I));
}

void InstructionVisitor::writeOptimizationInfo(
    const refmode_t &iref, const llvm::User *u) {
  using llvm::FPMathOperator;
  using llvm::OverflowingBinaryOperator;
  using llvm::PossiblyExactOperator;

  if (const auto *fpo = dyn_cast<const FPMathOperator>(u)) {
    if (fpo->isFast()) {
      gen.writeFact(pred::instr::flag, iref, "fast");
    } else {
      if (fpo->hasNoNaNs()) {
        gen.writeFact(pred::instr::flag, iref, "nnan");
      }
      if (fpo->hasNoInfs()) {
        gen.writeFact(pred::instr::flag, iref, "ninf");
      }
      if (fpo->hasNoSignedZeros()) {
        gen.writeFact(pred::instr::flag, iref, "nsz");
      }
      if (fpo->hasAllowReciprocal()) {
        gen.writeFact(pred::instr::flag, iref, "arcp");
      }
      if (fpo->hasAllowContract()) {
        gen.writeFact(pred::instr::flag, iref, "acon");
      }
      if (fpo->hasApproxFunc()) {
        gen.writeFact(pred::instr::flag, iref, "apfn");
      }
    }
  }

  if (const auto *obo = dyn_cast<OverflowingBinaryOperator>(u)) {
    if (obo->hasNoUnsignedWrap()) {
      gen.writeFact(pred::instr::flag, iref, "nuw");
    }

    if (obo->hasNoSignedWrap()) {
      gen.writeFact(pred::instr::flag, iref, "nsw");
    }
  } else if (const auto *div = dyn_cast<PossiblyExactOperator>(u)) {
    if (div->isExact()) {
      gen.writeFact(pred::instr::flag, iref, "exact");
    }
  }
}

static auto atomic_binop_string(const llvm::AtomicRMWInst::BinOp &op)
    -> std::string {
  switch (op) {
    case llvm::AtomicRMWInst::Xchg:
      return "xchg";
    case llvm::AtomicRMWInst::Add:
      return "add";
    case llvm::AtomicRMWInst::Sub:
      return "sub";
    case llvm::AtomicRMWInst::And:
      return "and";
    case llvm::AtomicRMWInst::Nand:
      return "nand";
    case llvm::AtomicRMWInst::Or:
      return "or";
    case llvm::AtomicRMWInst::Xor:
      return "xor";
    case llvm::AtomicRMWInst::Max:
      return "max";
    case llvm::AtomicRMWInst::Min:
      return "min";
    case llvm::AtomicRMWInst::UMax:
      return "umax";
    case llvm::AtomicRMWInst::UMin:
      return "umin";
    case llvm::AtomicRMWInst::FAdd:
      return "fadd";
    case llvm::AtomicRMWInst::FSub:
      return "fsub";
#if LLVM_VERSION_MAJOR > 14
    case llvm::AtomicRMWInst::FMax:
      return "fmax";
    case llvm::AtomicRMWInst::FMin:
      return "fmin";
#endif
    case llvm::AtomicRMWInst::BAD_BINOP:
      malformedModule("bad atomicrmw binop");
  }  // -Wswitch prevents fallthrough, no need for default case
  assert(false);
}

void InstructionVisitor::writeAtomicRMWOp(
    const refmode_t &instrref, llvm::AtomicRMWInst::BinOp op) {
  gen.writeFact(pred::atomicrmw::operation, instrref, atomic_binop_string(op));
}
