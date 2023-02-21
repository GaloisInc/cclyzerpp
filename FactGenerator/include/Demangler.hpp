#ifndef DEMANGLER_HPP__
#define DEMANGLER_HPP__

#include <cxxabi.h>

#include <memory>
#include <string>

class Demangler {
 public:
  /* NOTE(ww): Stolen from Demangle.cpp (not present in LLVM 7).
   */
  static inline auto is_itanium_encoding(const std::string& MangledName)
      -> bool {
    size_t pos = MangledName.find_first_not_of('_');
    // A valid Itanium encoding requires 1-4 leading underscores, followed by
    // 'Z'.
    return pos > 0 && pos <= 4 && MangledName[pos] == 'Z';
  }

  static inline auto demangle(const char* name) -> std::string {
    int status = -1;

    std::unique_ptr<char, void (*)(void*)> res{
        abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};
    return (status == 0) ? res.get() : std::string(name);
  }

  static inline auto demangle(const std::string& name) -> std::string {
    return demangle(name.c_str());
  }
};

#endif /* DEMANGLER_HPP__ */
