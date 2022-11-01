#include "Options.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <cassert>
#include <iostream>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

using cclyzer::Options;

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
Options::Options(int argc, char* argv[]) {
  const std::string appName = fs::basename(argv[0]);
  fs::path outdir;
  fs::path signatures;
  fs::path signatures_sentinel("SENTINEL");

  // Define and parse the program options
  po::options_description genericOpts("Options");

  genericOpts.add_options()("help,h", "Print help message")(
      "delim,d",
      po::value<std::string>(&delim)->default_value("\t"),
      "CSV file delimiter (default \\t)")(
      "out-dir,o",
      po::value<fs::path>(&outdir)->required(),
      "Output directory for generated facts")(
      "signatures,s",
      po::value<fs::path>(&signatures)->default_value(signatures_sentinel),
      "File with points-to signatures")(
      "context-sensitivity",
      po::value<ContextSensitivity>(&context_sensitivity)
          ->default_value(INSENSITIVE),
      "Set context sensitivity")(
      "recursive,r", "Recurse into input directories")(
      "force,f", "Remove existing contents of output directory");

  // hidden options group - don't show in help
  po::options_description hiddenOpts("hidden options");
  hiddenOpts.add_options()(
      "input-files",
      po::value<std::vector<fs::path> >()->required(),
      "LLVM bitcode input files");

  po::options_description cmdline_options;
  cmdline_options.add(genericOpts).add(hiddenOpts);

  // positional arguments
  po::positional_options_description positionalOptions;
  positionalOptions.add("input-files", -1);

  po::variables_map vm;

  try {
    po::store(
        po::command_line_parser(argc, argv)
            .options(cmdline_options)
            .positional(positionalOptions)
            .run(),
        vm);

    // --help option
    if (vm.count("help") != 0U) {
      std::cout << "Usage: " << appName << " [OPTIONS] INPUT_FILE...\n\n"
                << genericOpts;

      exit(EXIT_SUCCESS);
    }

    // may throw error
    po::notify(vm);
  } catch (boost::program_options::required_option& e) {
    std::cerr << e.what() << " from option: " << e.get_option_name()
              << std::endl;
    exit(ERROR_IN_COMMAND_LINE);
  } catch (boost::program_options::error& e) {
    std::cerr << e.what() << std::endl;
    exit(ERROR_IN_COMMAND_LINE);
  }

  // Sanity checks
  assert(vm.count("out-dir"));
  assert(vm.count("input-files"));

  // Compute input files and create output directories
  std::vector<fs::path> paths = vm["input-files"].as<std::vector<fs::path> >();
  set_output_dir(outdir, vm.count("force") != 0U);
  set_input_files(paths.begin(), paths.end(), vm.count("recursive") != 0U);

  if (signatures != signatures_sentinel) {
    set_signatures(std::move(signatures));
  }
}

template <typename FileIt>
void Options::set_input_files(
    FileIt file_begin, FileIt file_end, bool shouldRecurse) {
  // Delete old contents
  inputFiles.clear();

  // Iterate over every given path
  for (FileIt it = file_begin; it != file_end; ++it) {
    fs::path path = *it;

    // Check for existence
    if (!fs::exists(path)) {
      std::cerr << "Path does not exist: " << path << std::endl;
      exit(ERROR_IN_COMMAND_LINE);
    }

    // Add normal files
    if (!fs::is_directory(path)) {
      inputFiles.push_back(path);
      continue;
    }

    if (!shouldRecurse) {
      std::cerr << "Input directory given, without -r option: " << path
                << std::endl;

      exit(ERROR_IN_COMMAND_LINE);
    }

    // Recurse into directories
    for (fs::recursive_directory_iterator iter(path), end; iter != end;
         ++iter) {
      if (!fs::is_directory(iter->path())) {
        inputFiles.push_back(iter->path());
      }
    }
  }
}

void Options::set_output_dir(fs::path path, bool shouldForce) {
  // Create non-existing directory
  if (!fs::exists(path)) {
    fs::create_directory(path);
  }

  if (!fs::is_directory(path)) {
    std::cerr << "Not a directory: " << path << std::endl;
    exit(ERROR_IN_COMMAND_LINE);
  }

  // Remove old contents
  if (shouldForce) {
    for (auto& entry :
         boost::make_iterator_range(fs::directory_iterator(path), {})) {
      remove_all(entry.path());
    }
  }

  // Ensure output directory is empty
  if (!fs::is_empty(path)) {
    std::cerr << "Directory not empty: " << path << std::endl;
    exit(ERROR_IN_COMMAND_LINE);
  }

  // Store output directory (CHECK: should we canonicalize path)
  outdir = std::move(path);
}

void Options::set_signatures(fs::path path) {
  if (!fs::exists(path)) {
    std::cerr << "No such signature file: " << path << std::endl;
    exit(ERROR_IN_COMMAND_LINE);
  }
  signatures = llvm::Optional<boost::filesystem::path>(std::move(path));
}
