#include <llvm/ADT/Optional.h>        // for Optional
#include <llvm/Config/llvm-config.h>  // for LLVM_VERSION_MAJOR, LLVM...
#include <llvm/IR/LLVMContext.h>      // for LLVMContext
#include <llvm/IR/Module.h>           // for Module
#include <llvm/IRReader/IRReader.h>   // for parseIRFile
#include <llvm/Support/SourceMgr.h>   // for SMDiagnostic
#include <stdlib.h>                   // for EXIT_FAILURE, EXIT_SUCCESS

#include <boost/filesystem/operations.hpp>   // for canonical
#include <boost/filesystem/path.hpp>         // for path
#include <boost/filesystem/path_traits.hpp>  // for filesystem
#include <iostream>                          // for operator<<, endl, basic_...
#include <memory>                            // for unique_ptr
#include <string>                            // for string
#include <utility>                           // for move
#include <vector>                            // for vector

#include "ContextSensitivity.hpp"  // for ContextSensitivity, INSE...
#include "FactGenerator.hpp"       // for FactGenerator
#include "FactWriter.hpp"          // for FactWriter
#include "Options.hpp"             // for Options
#include "ParseException.hpp"      // for ParseException
#include "factgen.hpp"             // for factgen
namespace llvm {
class DataLayout;
}

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
    std::string delim) {
  using cclyzer::FactGenerator;
  using cclyzer::FactWriter;

#if LLVM_VERSION_MAJOR > 3 || \
    (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9)
  llvm::LLVMContext context;
#else
  llvm::LLVMContext &context = llvm::getGlobalContext();
#endif
  llvm::SMDiagnostic err;

  // Create fact writer
  FactWriter writer(outputDir, std::move(delim));

  // Create CSV generator
  FactGenerator &gen = FactGenerator::getInstance(writer);

  // Loop over each input file
  for (FileIt it = firstFile; it != endFile; ++it) {
    const fs::path &inputFile = *it;

    // Parse input file
    std::unique_ptr<llvm::Module> module =
        llvm::parseIRFile(inputFile.string(), err, context);

    // Check if parsing succeeded
    if (!module) throw ParseException(inputFile);

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
    std::vector<fs::path> files, const fs::path &outputDir, std::string delim) {
  cclyzer::factgen(
      files.begin(),
      files.end(),
      outputDir,
      llvm::Optional<boost::filesystem::path>(),
      INSENSITIVE,
      std::move(delim));
}
