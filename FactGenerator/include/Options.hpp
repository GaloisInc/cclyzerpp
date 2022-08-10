#ifndef OPTIONS_HPP__
#define OPTIONS_HPP__

#include <llvm/ADT/Optional.h>

#include <boost/filesystem.hpp>
#include <string>

#include "ContextSensitivity.hpp"

namespace cclyzer {
class Options;
}

class cclyzer::Options {
 public:
  using path = boost::filesystem::path;
  typedef std::vector<path> InputFileCollection;
  using input_file_iterator = InputFileCollection::const_iterator;

  /* Constructor given command-line options */
  Options(int argc, char* argv[]);  // NOLINT(modernize-avoid-c-arrays)

  [[nodiscard]] auto delimiter() const -> const std::string& { return delim; }

  [[nodiscard]] auto output_dir() const -> const path& { return outdir; }

  [[nodiscard]] auto get_signatures() const -> const llvm::Optional<path>& {
    return signatures;
  }

  [[nodiscard]] auto get_context_sensitivity() const
      -> const ContextSensitivity& {
    return context_sensitivity;
  }

  [[nodiscard]] auto input_file_begin() const -> input_file_iterator {
    return inputFiles.begin();
  }

  [[nodiscard]] auto input_file_end() const -> input_file_iterator {
    return inputFiles.end();
  }

 protected:
  /* Set input files */
  template <typename FileIt>
  void set_input_files(FileIt file_begin, FileIt file_end, bool shouldRecurse);

  /* Set output directory */
  void set_output_dir(path path, bool shouldForce);

  /* Set signatures file */
  void set_signatures(path path);

 private:
  /* Parsing failure exit code */
  const static int ERROR_IN_COMMAND_LINE = 1;

  /* CSV Delimiter */
  std::string delim;

  /* Points-to signatures */
  llvm::Optional<boost::filesystem::path> signatures;

  /* Output Directory for generated facts */
  boost::filesystem::path outdir;

  /* LLVM Bitcode IR Input Files */
  std::vector<boost::filesystem::path> inputFiles;

  ContextSensitivity context_sensitivity;
};

#endif
