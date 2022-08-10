#ifndef DEMANGLER_HPP__
#define DEMANGLER_HPP__

#include <cxxabi.h>

#include <memory>
#include <string>

class Demangler {
 public:
  inline auto demangle(const char* name) -> std::string {
    int status = -1;

    std::unique_ptr<char, void (*)(void*)> res{
        abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};
    return (status == 0) ? res.get() : std::string(name);
  }

  inline auto demangle(const std::string& name) -> std::string {
    return demangle(name.c_str());
  }
};

#endif /* DEMANGLER_HPP__ */
