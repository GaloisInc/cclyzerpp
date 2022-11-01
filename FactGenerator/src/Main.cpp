#include <llvm/Config/llvm-config.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

#include <boost/filesystem.hpp>
#include <iostream>
#include <string>

#include "ContextSensitivity.hpp"
#include "FactGenerator.hpp"
#include "FactWriter.hpp"
#include "Factgen.hpp"
#include "Options.hpp"
#include "ParseException.hpp"

// Type aliases
namespace fs = boost::filesystem;

//--------------------------------------------------------------------------
// Driver Fact-Generation Routine
//--------------------------------------------------------------------------

template <typename FileIt>
void cclyzer::factgen(
    FileIt firstFile,
    FileIt endFile,
    const fs::path &outputDir,
    const llvm::Optional<fs::path> &signatures,
    const ContextSensitivity &context_sensitivity,
    const std::string &delim) {
  using cclyzer::FactGenerator;
  using cclyzer::FactWriter;

  llvm::LLVMContext context;
  llvm::SMDiagnostic err;

  // Create fact writer
  FactWriter writer(outputDir, delim);

  // Create CSV generator
  FactGenerator &gen = FactGenerator::getInstance(writer);

  // Loop over each input file
  for (FileIt it = firstFile; it != endFile; ++it) {
    const fs::path &inputFile = *it;

    // Parse input file
    std::unique_ptr<llvm::Module> module =
        llvm::parseIRFile(inputFile.string(), err, context);

    // Check if parsing succeeded
    if (!module) {
      throw ParseException(inputFile);
    }

    // Canonicalize path
    std::string realPath = fs::canonical(inputFile).string();

    // Generate facts for this module
    gen.processModule(*module, realPath, signatures, context_sensitivity);

    // Get data layout of this module
    const llvm::DataLayout &layout = module->getDataLayout();

    // Write types
    gen.writeTypes(layout);
  }
}

auto main(int argc, char *argv[]) -> int {
  using cclyzer::Options;

  // Parse command line
  Options options(argc, argv);

  try {
    cclyzer::factgen(
        options.input_file_begin(),
        options.input_file_end(),
        options.output_dir(),
        options.get_signatures(),
        options.get_context_sensitivity(),
        options.delimiter());
  } catch (const ParseException &error) {
    std::cerr << error.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

// Alternate version that doesn't use templates so that the python bindings work

void factgen2(
    std::vector<fs::path> files,
    const fs::path &outputDir,
    const std::string &delim) {
  cclyzer::factgen(
      files.begin(),
      files.end(),
      outputDir,
      llvm::Optional<boost::filesystem::path>(),
      INSENSITIVE,
      delim);
}
