#pragma once

#include <llvm/ADT/Optional.h>
#include <llvm/IR/Module.h>

#include <boost/filesystem.hpp>
#include <boost/flyweight.hpp>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ContextSensitivity.hpp"

namespace fs = boost::filesystem;

// Return both the directory with the facts and the identifying information
// generated for the pointers being analyzed.
auto factgen_module(
    llvm::Module &,
    const fs::path &,
    const llvm::Optional<boost::filesystem::path> &,
    const ContextSensitivity)
    -> std::tuple<
        boost::filesystem::path,
        std::map<boost::flyweight<std::string>, const llvm::Value *>>;
