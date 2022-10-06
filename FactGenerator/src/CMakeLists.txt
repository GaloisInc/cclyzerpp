target_sources(
  SoufflePA
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/assembly.cpp
          ${CMAKE_CURRENT_LIST_DIR}/constants.cpp
          ${CMAKE_CURRENT_LIST_DIR}/ContextManager.hpp
          ${CMAKE_CURRENT_LIST_DIR}/ContextSensitivity.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactWriter.cpp
          ${CMAKE_CURRENT_LIST_DIR}/functions.cpp
          ${CMAKE_CURRENT_LIST_DIR}/globals.cpp
          ${CMAKE_CURRENT_LIST_DIR}/InstructionVisitor.cpp
          ${CMAKE_CURRENT_LIST_DIR}/llvm_enums.cpp
          # ${CMAKE_CURRENT_LIST_DIR}/main.cpp
          ${CMAKE_CURRENT_LIST_DIR}/wrapper.cpp
          ${CMAKE_CURRENT_LIST_DIR}/types.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Options.cpp
          ${CMAKE_CURRENT_LIST_DIR}/predicate.cpp
          ${CMAKE_CURRENT_LIST_DIR}/predicate_groups.cpp
          # ${CMAKE_CURRENT_LIST_DIR}/PyFactGen.cpp
          ${CMAKE_CURRENT_LIST_DIR}/RefmodeEngine.cpp
          ${CMAKE_CURRENT_LIST_DIR}/RefmodeEngineImpl.cpp
          ${CMAKE_CURRENT_LIST_DIR}/RefmodeEngineImpl.hpp
          ${CMAKE_CURRENT_LIST_DIR}/Signatures.cpp
          ${CMAKE_CURRENT_LIST_DIR}/TypeAccumulator.cpp
          ${CMAKE_CURRENT_LIST_DIR}/types.cpp
          ${CMAKE_CURRENT_LIST_DIR}/TypeVisitor.cpp
          ${CMAKE_CURRENT_LIST_DIR}/variables.cpp)
