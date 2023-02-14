#ifndef FACT_WRITER_H__
#define FACT_WRITER_H__

#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>
#include <string>

#include "CsvWriter.hpp"
#include "Predicate.hpp"
#include "RefmodeEngine.hpp"  // TODO replace

namespace cclyzer {
class FactWriter;
}

class cclyzer::FactWriter {
 public:
  /* Common type aliases */

  using path = boost::filesystem::path;
  using string = std::string;

  FactWriter(
      const Registry<Predicate>& registry,
      path outputDirectory,
      string delimiter,
      BOOST_IOS::openmode mode = BOOST_IOS::out);
  FactWriter(const Registry<Predicate>& registry, path outputDirectory);
  FactWriter(const Registry<Predicate>& registry);
  ~FactWriter();

  /* Non-default */
  FactWriter() = delete;

  /* Non-copyable */
  FactWriter(const FactWriter& other) = delete;
  auto operator=(const FactWriter&) -> FactWriter& = delete;

  /* Delegation to fact writer instance  */

  void writeFact(const Predicate& pred, const refmode_t& refmode) {
    getWriter(pred)->write(refmode);
  }

  template <typename V, typename... Vs>
  void writeFact(
      const Predicate& pred,
      const refmode_t& refmode,
      const V& val,
      const Vs&... vals) {
    getWriter(pred)->write(refmode, val, vals...);
  }

 protected:
  /* Get CSV writer instance for given predicate */
  auto getWriter(const Predicate& pred) -> csv_writer*;

  /* Filesystem path computation  */
  auto getPath(const Predicate& pred) -> path;

  /* Exhaustive CSV writer initialization */
  void init_writers(const Registry<Predicate>&);

 private:
  /* Map type */
  template <typename K, typename V>
  using map = boost::unordered_map<K, V>;

  /* Column Delimiter */
  const string delim;

  /* Output directory */
  const path outdir;

  /* Open mode for output CSV */
  const BOOST_IOS::openmode mode;

  /* Map of CSV writers with predicate name as key */
  map<string, csv_writer*> writers;

  /* CSV file extension */
  static const string FILE_EXTENSION;
};

#endif /* FACT_WRITER_H__ */
