#include "Wrapper.hpp"

#include <cstdio>
#include <iostream>

#include "FactGenerator.hpp"
#include "FactWriter.hpp"

namespace fs = boost::filesystem;

auto factgen_module(
    llvm::Module &module,
    const fs::path &output_dir,
    const llvm::Optional<boost::filesystem::path> &signatures,
    ContextSensitivity sensitivity)
    -> std::tuple<
        fs::path,
        std::map<boost::flyweight<std::string>, const llvm::Value *>> {
  using cclyzer::FactGenerator;
  using cclyzer::FactWriter;

  std::cerr << "Writing facts to: " << output_dir << "...\n";

  // initialize factgen and output writer
  FactWriter writer(output_dir, "\t");
  FactGenerator &gen = FactGenerator::getInstance(writer);
  const std::string &real_path = module.getSourceFileName();

  // do the fact generation
  auto res_maps = gen.processModule(module, real_path, signatures, sensitivity);

  const llvm::DataLayout &layout = module.getDataLayout();
  gen.writeTypes(layout);

  return std::make_tuple(output_dir, std::move(res_maps));
}
