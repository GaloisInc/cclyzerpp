#ifndef FORWARDING_FACT_WRITER_H__
#define FORWARDING_FACT_WRITER_H__

#include "FactWriter.hpp"

namespace cclyzer {
class ForwardingFactWriter {
 public:
  ForwardingFactWriter(FactWriter &writer) : writer(writer) {}

  /* Delegation to fact writer instance */

  void writeFact(const Predicate &pred, const refmode_t &refmode) {
    writer.writeFact(pred, refmode);
  }

  template <typename V, typename... Vs>
  void writeFact(
      const Predicate &pred,
      const refmode_t &refmode,
      const V val,
      const Vs &... vals) {
    writer.writeFact(pred, refmode, val, vals...);
  }

 protected:
  auto getWriter() -> FactWriter & { return writer; }

 private:
  /* Delegate CSV Writer */
  FactWriter &writer;
};
}  // namespace cclyzer

#endif /* FORWARDING_FACT_WRITER_H__ */
