#include <llvm/Support/JSON.h>

#include <fstream>
#include <regex>
#include <set>
#include <string>

#include "ForwardingFactWriter.hpp"
#include "PredicateGroups.hpp"

auto preprocess_signatures(const boost::filesystem::path &signatures_path)
    -> std::vector<std::tuple<std::string, std::regex, llvm::json::Array>>;

void emit_signatures(
    cclyzer::ForwardingFactWriter &writer,
    const std::string &function_name,
    const llvm::json::Array &signatures);
