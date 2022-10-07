cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project(cclyzer++ LANGUAGES C CXX)

find_package(LLVM 10.0 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem program_options
                                            iostreams)

find_package(OpenMP REQUIRED)

# Allow these to be overriden at the command line:
if(NOT (DEFINED SOUFFLE_BIN))
  set(SOUFFLE_BIN souffle)
endif()
if(NOT (DEFINED SOUFFLE_INCLUDE))
  set(SOUFFLE_INCLUDE /usr/include/souffle)
endif()

# -----------------------------------------------------------------------------

add_subdirectory(datalog)
# TODO(lb): Remove --disable-transformers=ExpandEqrelsTransformer when
# https://github.com/souffle-lang/souffle/issues/2257 is fixed and Soufflé gets
# upgraded.
set(SOUFFLE_FLAGS -jauto -PSIPS:max-bound
                  --disable-transformers=ExpandEqrelsTransformer)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/subset-and-unification.cpp
  COMMAND
    ${SOUFFLE_BIN}
    ${CMAKE_CURRENT_LIST_DIR}/datalog/subset-and-unification.project
    ${SOUFFLE_FLAGS} -g ${CMAKE_CURRENT_BINARY_DIR}/subset-and-unification.cpp
  DEPENDS ${DL_SOURCES})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/subset.cpp
  COMMAND ${SOUFFLE_BIN} ${CMAKE_CURRENT_LIST_DIR}/datalog/subset.project
          ${SOUFFLE_FLAGS} -g ${CMAKE_CURRENT_BINARY_DIR}/subset.cpp
  DEPENDS ${DL_SOURCES})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/unification.cpp
  COMMAND ${SOUFFLE_BIN} ${CMAKE_CURRENT_LIST_DIR}/datalog/unification.project
          ${SOUFFLE_FLAGS} -g ${CMAKE_CURRENT_BINARY_DIR}/unification.cpp
  DEPENDS ${DL_SOURCES})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/debug.cpp
  COMMAND ${SOUFFLE_BIN} ${CMAKE_CURRENT_LIST_DIR}/datalog/debug.project
          ${SOUFFLE_FLAGS} -g ${CMAKE_CURRENT_BINARY_DIR}/debug.cpp
  DEPENDS ${DL_SOURCES})

add_library(
  SoufflePAObject OBJECT
  ${CMAKE_CURRENT_BINARY_DIR}/debug.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/subset-and-unification.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/subset.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/unification.cpp)

target_compile_features(SoufflePAObject PUBLIC cxx_std_17)
set_target_properties(SoufflePAObject PROPERTIES POSITION_INDEPENDENT_CODE True)

set(SOUFFLE_COMPILE_FLAGS -fopenmp)

# This build takes an especially long time with any optimization level above
# -O0, but high optimization levels have an outsized (positive) impact on
# performance.
set(SOUFFLE_COMPILE_FLAGS ${SOUFFLE_COMPILE_FLAGS} "$<$<CONFIG:Debug>:-O0>"
                          "$<$<CONFIG:RelWithDebInfo>:-O3>"
                          "$<$<CONFIG:Release>:-O3>")
target_include_directories(SoufflePAObject SYSTEM PUBLIC ${SOUFFLE_INCLUDE})

target_compile_options(SoufflePAObject PRIVATE ${SOUFFLE_COMPILE_FLAGS})
target_compile_definitions(SoufflePAObject PRIVATE __EMBEDDED_SOUFFLE__)
target_compile_definitions(SoufflePAObject PRIVATE USE_LIBZ)

add_library(SoufflePA SHARED PAInterface.cpp PAInterface.h
                             $<TARGET_OBJECTS:SoufflePAObject>)

target_link_libraries(
  SoufflePA PRIVATE Boost::system Boost::filesystem Boost::program_options
                    Boost::iostreams ${OpenMP_CXX_LIBRARIES})

# Use C++17 to compile our pass (i.e., supply -std=c++17).
target_compile_features(SoufflePA PUBLIC cxx_std_17)

target_compile_options(
  SoufflePA
  PUBLIC # -Werror
         -pedantic-errors
         -Wall
         -Wextra
         -Wconversion
         -Wsign-conversion
         -Wreturn-type
         -fopenmp)

target_include_directories(SoufflePA SYSTEM PUBLIC ${SOUFFLE_INCLUDE})

add_subdirectory(FactGenerator)

# Get proper shared-library behavior (where symbols are not necessarily resolved
# when the shared library is linked) on OS X.
if(APPLE)
  target_link_options(SoufflePA PRIVATE -undefined dynamic_lookup)
endif(APPLE)

get_target_property(SPA_SOURCES SoufflePA SOURCES)
foreach(spa_source ${SPA_SOURCES})
  if(NOT (${spa_source} MATCHES ".+\.o"))
    get_filename_component(ABSOLUTE_SPA_SOURCE ${spa_source} ABSOLUTE)
    list(APPEND ABSOLUTE_SPA_SOURCES ${ABSOLUTE_SPA_SOURCE})
  endif()
endforeach()

if(CLANG_TIDY)
  add_custom_target(
    spa-tidy
    COMMAND ${CLANG_TIDY} -fix -p=${CMAKE_BINARY_DIR} -quiet
            ${ABSOLUTE_SPA_SOURCES}
    COMMENT "Linting with clang-tidy...")
endif()

if(CLANG_FORMAT)
  add_custom_target(
    spa-format
    COMMAND ${CLANG_FORMAT} -i ${ABSOLUTE_SPA_SOURCES}
    COMMENT "Formatting with clang-format...")
endif()

add_library(PAPassInterface INTERFACE)

target_sources(PAPassInterface
               INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/PointerAnalysis.h)

target_include_directories(PAPassInterface
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(PAPass MODULE PointerAnalysis.cpp)

# Use C++17 to compile our pass (i.e., supply -std=c++17).
target_compile_features(PAPass PUBLIC cxx_std_17)

# LLVM is (typically) built with no C++ RTTI. We need to match that; otherwise,
# we'll get linker errors about missing RTTI data.
if(NOT LLVM_ENABLE_RTTI)
  target_compile_options(PAPass PRIVATE -fno-rtti)
endif()

# Warnings
target_compile_options(
  PAPass
  PUBLIC -Werror
         -pedantic-errors
         -Wall
         -Wextra
         -Wconversion
         -Wsign-conversion
         -Wreturn-type
         -fopenmp)

target_link_libraries(PAPass PRIVATE PAPassInterface SoufflePA
                                     Boost::filesystem)

# Get proper shared-library behavior (where symbols are not necessarily resolved
# when the shared library is linked) on OS X.
if(APPLE)
  target_link_options(PAPass PRIVATE -undefined dynamic_lookup)
endif(APPLE)

add_executable(factgen-exe ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/main.cpp)
target_compile_features(factgen-exe PUBLIC cxx_std_17)
target_include_directories(
  factgen-exe SYSTEM PRIVATE ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/include)
llvm_map_components_to_libnames(factgen_llvm_libs support core irreader)
target_link_libraries(
  factgen-exe
  PRIVATE Boost::system Boost::filesystem Boost::program_options
          Boost::iostreams ${OpenMP_CXX_LIBRARIES} ${factgen_llvm_libs})

target_sources(
  factgen-exe
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/assembly.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/constants.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/ContextManager.hpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/ContextSensitivity.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/FactGenerator.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/FactWriter.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/functions.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/globals.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/InstructionVisitor.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/llvm_enums.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/main.cpp
          # ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/wrapper.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/types.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/Options.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/predicate.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/predicate_groups.cpp
          # ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/PyFactGen.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/RefmodeEngine.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/RefmodeEngineImpl.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/RefmodeEngineImpl.hpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/Signatures.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/TypeAccumulator.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/types.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/TypeVisitor.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FactGenerator/src/variables.cpp)

get_target_property(FACTGEN_SOURCES factgen-exe SOURCES)
foreach(factgen_source ${FACTGEN_SOURCES})
  get_filename_component(ABSOLUTE_FACTGEN_SOURCE ${factgen_source} ABSOLUTE)
  list(APPEND ABSOLUTE_FACTGEN_SOURCES ${ABSOLUTE_FACTGEN_SOURCE})
endforeach()

# -----------------------------------------------------------------------------

# Add clang-tidy target
find_program(
  CLANG_TIDY
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable.")

# Add clang-format target
find_program(
  CLANG_FORMAT
  NAMES "clang-format"
  DOC "Path to clang-format executable.")

if(CLANG_TIDY)
  add_custom_target(
    factgen-tidy
    COMMAND ${CLANG_TIDY} -fix -p=${CMAKE_BINARY_DIR} -quiet
            ${ABSOLUTE_FACTGEN_SOURCES}
    COMMENT "Linting with clang-tidy...")
endif()