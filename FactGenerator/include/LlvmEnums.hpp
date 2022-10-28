#ifndef LLVM_ENUMS_HPP__
#define LLVM_ENUMS_HPP__

#include <llvm/IR/CallingConv.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>

#include <string>

namespace cclyzer::utils {
// Functions that convert the various LLVM enums to strings
auto to_string(llvm::GlobalValue::LinkageTypes LT) -> std::string;
auto to_string(llvm::GlobalValue::VisibilityTypes Vis) -> std::string;
auto to_string(llvm::GlobalVariable::ThreadLocalMode TLM) -> std::string;
auto to_string(llvm::CallingConv::ID CC) -> std::string;
auto to_string(llvm::AtomicOrdering AO) -> std::string;

}  // namespace cclyzer::utils

#endif /* LLVM_ENUMS_HPP__ */
