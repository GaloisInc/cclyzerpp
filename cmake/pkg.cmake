# Packaging cclyzer++ with fpm

# Allow this to be overriden at the command line:
if(NOT (DEFINED FPM_BIN))
  set(FPM_BIN fpm)
endif()

set(DEB cclyzer++_${CMAKE_PROJECT_VERSION}-1_amd64.deb)
add_custom_command(
  OUTPUT ${DEB}
  COMMAND
  ${FPM_BIN}
  -s dir
  -t deb
  -p ${DEB}
  --version ${CMAKE_PROJECT_VERSION}
  --name cclyzer++
  --license bsd3
  --depends llvm-${LLVM_MAJOR_VERSION}
  --depends libboost-system-dev
  --depends libboost-filesystem-dev
  --depends libboost-iostreams-dev
  --depends libboost-program-options-dev
  --depends libomp-${CLANG_VERSION}-dev
  --architecture x86_64
  --description cclyzer++
  --url https://galoisinc.github.io/cclyzerpp/
  ${CMAKE_CURRENT_BINARY_DIR}/factgen-exe=/usr/bin/factgen-exe
  ${CMAKE_CURRENT_BINARY_DIR}/libSoufflePA.so=/usr/lib/libSoufflePA.so
  ${CMAKE_CURRENT_BINARY_DIR}/libPAPass.so=/usr/lib/libPAPass.so
  DEPENDS factgen-exe PAPass SoufflePA)

add_custom_target(deb DEPENDS ${DEB})