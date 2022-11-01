#include <iostream>

[[noreturn]] static void malformedModule(const std::string &explanation) {
  std::cerr << "[error] Malformed LLVM module, " << explanation << std::endl;
  std::cerr << "[error] Try opt -disable-output -verify prog.bc" << std::endl;
  std::cerr << "[error] Or report a bug: "
               "https://github.com/GaloisInc/cclyzerpp/issues"
            << std::endl;
  std::exit(1);
}
