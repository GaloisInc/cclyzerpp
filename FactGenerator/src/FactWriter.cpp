#include "FactWriter.hpp"

#include <algorithm>

#include "PredicateGroups.hpp"

namespace fs = boost::filesystem;
namespace pred = cclyzer::predicates;

using cclyzer::csv_writer;
using cclyzer::FactWriter;

using pred::pred_t;

//-------------------------------------------------------------------
// Constant Definitions
//-------------------------------------------------------------------

const std::string FactWriter::FILE_EXTENSION = ".csv.gz";

//-------------------------------------------------------------------
// Delegating Constructors
//-------------------------------------------------------------------

FactWriter::FactWriter(
    const Registry<pred_t>& registry,
    path outputDirectory,
    string delimiter,
    BOOST_IOS::openmode mode)
    : delim(std::move(delimiter)),
      outdir(std::move(outputDirectory)),
      mode(mode) {
  // Initialize all CSV writers
  init_writers(registry);
}

FactWriter::FactWriter(const Registry<pred_t>& registry, path outputDirectory)
    : FactWriter(registry, std::move(outputDirectory), "\t") {}

FactWriter::FactWriter(const Registry<pred_t>& registry)
    : FactWriter(registry, fs::current_path()) {}

FactWriter::~FactWriter() {
  for (auto& [_, writer] : writers) {
    delete writer;
  }
}

//-------------------------------------------------------------------
// CSV Writers Management
//-------------------------------------------------------------------

auto FactWriter::getWriter(const pred_t& pred) -> csv_writer* {
  using namespace boost::filesystem;

  // Use predicate name as key
  const string& key = pred.getName();

  // Search for existing writer
  map<string, csv_writer*>::iterator it = writers.find(key);

  // Return existing writer
  if (it != writers.end()) {
    return it->second;
  }

  // Get filesystem path to CSV file
  path file = getPath(pred);
  create_directory(file.parent_path());

  // Create and return new writer
  return writers[key] = new csv_writer(file, delim, mode);
}

auto FactWriter::getPath(const pred_t& pred) -> fs::path {
  namespace fs = boost::filesystem;

  string basename(pred);

  // Replace all ':' characters with '-'
  std::replace(basename.begin(), basename.end(), ':', '-');

  // Add directory and extension
  fs::path path = outdir / basename;
  path += FILE_EXTENSION;

  return path;
}

void FactWriter::init_writers(const Registry<pred_t>& registry) {
  for (const pred_t* pred : registry) {
    getWriter(*pred);
  }

  // TODO: Consider closing streams and opening them lazily, so as
  // not to exceed the maximum number of open file descriptors
}
