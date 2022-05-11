include(AddCXXFlag)

# ---------------------------------------------------------------------------------------
# C++ standard
# ---------------------------------------------------------------------------------------
# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------------------
# Warning
# ---------------------------------------------------------------------------------------
add_cxx_compiler_flag("-Wall")
add_cxx_compiler_flag("-Wextra")
add_cxx_compiler_flag("-Wpedantic")
add_cxx_compiler_flag("-Werror")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  add_cxx_compiler_flag("-Wno-error=unused-command-line-argument")
endif()

# ---------------------------------------------------------------------------------------
# Linker
# ---------------------------------------------------------------------------------------
# FIXME
if(CMAKE_COMPILER_IS_GNUCXX
   AND ((${CMAKE_BUILD_TYPE} STREQUAL "Release") OR (${CMAKE_BUILD_TYPE}
                                                     STREQUAL "MinSizeRel")))
  execute_process(
    COMMAND ld.bfd --version
    OUTPUT_VARIABLE LINKER_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "\n" ";" LINKER_VERSION ${LINKER_VERSION})
  list(GET LINKER_VERSION 0 LINKER_VERSION)
  message(STATUS "Linker: ${LINKER_VERSION}")

  add_cxx_linker_flag("-fuse-ld=bfd")
else()
  execute_process(
    COMMAND ld.mold --version
    OUTPUT_VARIABLE LINKER_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Linker: ${LINKER_VERSION}")

  add_cxx_linker_flag("-fuse-ld=mold")
endif()

# ---------------------------------------------------------------------------------------
# General options
# ---------------------------------------------------------------------------------------
add_cxx_compiler_flag("-pipe")

add_cxx_compiler_flag("-maes")
add_cxx_compiler_flag("-march=haswell")

add_cxx_linker_flag("-static-libgcc")
add_cxx_linker_flag("-static-libstdc++")

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------------------------------------------------
# Link time optimization
# ---------------------------------------------------------------------------------------
# https://github.com/ninja-build/ninja/blob/master/CMakeLists.txt
if(((${CMAKE_BUILD_TYPE} STREQUAL "Release") OR (${CMAKE_BUILD_TYPE} STREQUAL
                                                 "MinSizeRel"))
   AND NOT KEPUB_SANITIZER)
  include(CheckIPOSupported)
  check_ipo_supported(
    RESULT LTO_SUPPORTED
    OUTPUT ERROR
    LANGUAGES CXX)

  if(LTO_SUPPORTED)
    message(STATUS "Link time optimization: enabled")

    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    if(CMAKE_COMPILER_IS_GNUCXX)
      add_cxx_compiler_flag("-flto=auto")
    endif()
  else()
    message(FATAL_ERROR "Link time optimization not supported: ${ERROR}")
  endif()
else()
  message(STATUS "Link time optimization: disable")
endif()

# ---------------------------------------------------------------------------------------
# Strip
# ---------------------------------------------------------------------------------------
if((${CMAKE_BUILD_TYPE} STREQUAL "Release") OR (${CMAKE_BUILD_TYPE} STREQUAL
                                                "MinSizeRel"))
  message(STATUS "Discard symbols and other data from object files")
  add_cxx_linker_flag("-s")
endif()

# ---------------------------------------------------------------------------------------
# Sanitizer
# ---------------------------------------------------------------------------------------
if(KEPUB_SANITIZER)
  message(STATUS "Build with AddressSanitizer and UndefinedSanitizer")

  add_cxx_compiler_flag("-fsanitize=address")
  add_cxx_compiler_flag("-fsanitize-address-use-after-scope")
  add_cxx_compiler_flag("-fno-omit-frame-pointer")
  add_cxx_compiler_flag("-fno-optimize-sibling-calls")

  add_cxx_compiler_flag("-fsanitize=undefined")
  add_cxx_compiler_flag("-fno-sanitize-recover=all")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_cxx_compiler_flag("-fsanitize=float-divide-by-zero")
    add_cxx_compiler_flag("-fsanitize=unsigned-integer-overflow")
    add_cxx_compiler_flag("-fsanitize=implicit-conversion")
    add_cxx_compiler_flag("-fsanitize=local-bounds")
    add_cxx_compiler_flag("-fsanitize=nullability")

    add_cxx_compiler_flag("-fsanitize-recover=unsigned-integer-overflow")
    add_cxx_compiler_flag("-fsanitize-recover=implicit-conversion")
  endif()

  if(CMAKE_COMPILER_IS_GNUCXX)
    add_cxx_linker_flag("-static-libasan")
    add_cxx_linker_flag("-static-liblsan")
    add_cxx_linker_flag("-static-libubsan")
  else()
    add_cxx_linker_flag("-static-libsan")
  endif()
endif()

# ---------------------------------------------------------------------------------------
# Coverage
# ---------------------------------------------------------------------------------------
if(KEPUB_BUILD_COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    add_cxx_compiler_flag("--coverage")
  else()
    add_cxx_compiler_flag("-fprofile-instr-generate")
    add_cxx_compiler_flag("-fcoverage-mapping")
  endif()
endif()
