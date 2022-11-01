#ifndef FACT_GENERATOR_HPP__
#define FACT_GENERATOR_HPP__

#include <llvm/ADT/Optional.h>

#include <boost/filesystem.hpp>
#include <string>

#include "ContextSensitivity.hpp"

namespace cclyzer {
// Main fact-generation routines

template <typename FileIt>
void factgen(
    FileIt firstFile,
    FileIt endFile,
    const boost::filesystem::path &outputDir,
    const llvm::Optional<boost::filesystem::path> &signatures,
    const ContextSensitivity &context_sensitivity) {
  return factgen(
      firstFile, endFile, outputDir, signatures, context_sensitivity, "\t");
}

template <typename FileIt>
void factgen(
    FileIt firstFile,
    FileIt endFile,
    const boost::filesystem::path &outputDir,
    const llvm::Optional<boost::filesystem::path> &signatures,
    const ContextSensitivity &context_sensitivity,
    const std::string &delim);
}  // namespace cclyzer

#endif /* FACT_GENERATOR_HPP__ */
