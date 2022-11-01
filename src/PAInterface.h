#pragma once

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <souffle/SouffleInterface.h>

#include <boost/filesystem.hpp>
#include <boost/flyweight.hpp>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

enum PAFlags { NONE = 0, WRITE_ALL = 1 << 0 };
inline constexpr auto operator|(PAFlags lhs, PAFlags rhs) -> PAFlags {
  return static_cast<PAFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

//------------------------------------------------------------------------------
// Templates

// Extract a single element of type T from this tuple/row.
template <class T>
auto extract_from_row(
    souffle::tuple &row,
    const std::map<boost::flyweight<std::string>, const llvm::Value *>
        &llvm_val_map) -> T {
  if constexpr (std::is_same<T, int>::value) {
    int t0;
    row >> t0;
    return t0;
  } else {  // NOLINT: clang-tidy doesn't know about "if constexpr"
    std::string t0;
    row >> t0;
    if constexpr (std::is_same<T, const llvm::Value *>::value) {
      try {
        return llvm_val_map.at(boost::flyweight<std::string>(t0));
      } catch (const std::out_of_range &) {
        std::cout << "Failed value* lookup: " << t0 << std::endl;
        exit(EXIT_FAILURE);
      }
    } else if constexpr (std::is_same<T, boost::flyweight<std::string>>::
                             value) {
      return boost::flyweight<std::string>(t0);
    } else {
      return t0;
    }
  }
}

template <typename T, typename... Ts>
auto extract_row(
    souffle::tuple &row,
    const std::map<boost::flyweight<std::string>, const llvm::Value *>
        &llvm_val_map) -> std::tuple<T, Ts...> {
  auto t = extract_from_row<T>(row, llvm_val_map);
  if constexpr (sizeof...(Ts) == 0) {
    return std::make_tuple(t);
  } else {  // NOLINT: clang-tidy doesn't know about "if constexpr"
    return std::tuple_cat(
        std::make_tuple(t), extract_row<Ts...>(row, llvm_val_map));
  }
}

template <typename T, typename... Ts>
auto relation_to_vector(
    const souffle::Relation *rel,
    const std::map<boost::flyweight<std::string>, const llvm::Value *>
        &llvm_val_map) -> std::vector<std::tuple<T, Ts...>> {
  assert(rel != nullptr);
  assert(rel->getArity() == sizeof...(Ts) + 1);
  std::vector<std::tuple<T, Ts...>> vec;
  for (auto &row : *rel) {
    vec.push_back(extract_row<T, Ts...>(row, llvm_val_map));
  }
  return vec;
}

//------------------------------------------------------------------------------
// Interface

class PAInterface {
 public:
  static auto create(const std::string &) -> std::unique_ptr<PAInterface>;
  ~PAInterface();

  // Main entry point for the pointer analysis.  Assumes facts have been
  // generated, and so calls out to Souffle to run on them.
  auto runPointerAnalysis(const boost::filesystem::path &, const PAFlags)
      -> int;

  // Check any assertions that are embedded in the Datalog code. Must be called
  // after runPointerAnalysis. Throws std::logic_error if an assertion fires.
  //
  // If the argument is true and the assertion relations couldn't be found in
  // the output, throws std::logic_error.
  void checkAssertions(bool expect);

  // Getters for the various kinds of results we support

  template <typename T, typename... Ts>
  auto relationToVector(
      const std::string &relation,
      const std::map<boost::flyweight<std::string>, const llvm::Value *>
          &llvm_val_map) const -> std::vector<std::tuple<T, Ts...>> {
    return relation_to_vector<T, Ts...>(
        souffle_program_->getRelation(relation), llvm_val_map);
  }

 private:
  // The Souffle data
  explicit PAInterface(souffle::SouffleProgram *);

  //------------------------------------------------------------------------------
  // Variables

  std::unique_ptr<souffle::SouffleProgram> souffle_program_;
};
