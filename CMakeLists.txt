# ---------------------------------------------------------------------------------------
# CMake version
# ---------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.22)
message(STATUS "CMake version: ${CMAKE_VERSION}")

# ---------------------------------------------------------------------------------------
# Start kepub project
# ---------------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Version)
kepub_extract_version()

project(
  kepub
  LANGUAGES ASM CXX
  VERSION ${KEPUB_VERSION}
  HOMEPAGE_URL https://github.com/KaiserLancelot/kepub)

message(STATUS "${PROJECT_NAME} version: ${KEPUB_VERSION}")

# ---------------------------------------------------------------------------------------
# Set default build to release
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Choose Debug, Release, RelWithDebInfo or MinSizeRel" FORCE)
endif()
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

# ---------------------------------------------------------------------------------------
# Set variables
# ---------------------------------------------------------------------------------------
set(KEPUB_LIBRARY lib${PROJECT_NAME})

set(SFACG_EXECUTABLE sfacg)
set(CIWEIMAO_EXECUTABLE ciweimao)
set(ESJZONE_EXECUTABLE esjzone)
set(LIGHTNOVEL_EXECUTABLE lightnovel)
set(MASIRO_EXECUTABLE masiro)

set(GEN_EPUB_EXECUTABLE gen-epub)
set(APPEND_EPUB_EXECUTABLE append-epub)
set(EXTRACT_EPUB_EXECUTABLE extract-epub)

set(KEPUB_TEST_EXECUTABLE ktest)

set(KEPUB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KEPUB_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB KEPUB_TEST_FILE CONFIGURE_DEPENDS "${KEPUB_SOURCE_DIR}/data/*")

# https://cmake.org/cmake/help/latest/command/file.html#glob-recurse
file(GLOB_RECURSE KEPUB_LIBRARY_SRC CONFIGURE_DEPENDS
     "${KEPUB_SOURCE_DIR}/src/*.cpp")

# ---------------------------------------------------------------------------------------
# Include CMake module
# ---------------------------------------------------------------------------------------
include(CTest)
include(GNUInstallDirs)
include(GenerateExportHeader)

include(Options)
include(Check)
include(Format)
include(CompilerOptions)

# ---------------------------------------------------------------------------------------
# Find package
# ---------------------------------------------------------------------------------------
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

find_package(fmt REQUIRED)
find_package(pugixml REQUIRED)
find_package(indicators REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS json)
find_package(klib REQUIRED)
find_package(simdjson REQUIRED)
find_package(Microsoft.GSL REQUIRED)
find_package(TBB REQUIRED)
find_package(re2 REQUIRED)

add_definitions(-DDBG_MACRO_NO_WARNING)
if(NOT (${CMAKE_BUILD_TYPE} STREQUAL "Debug"))
  add_definitions(-DDBG_MACRO_DISABLE)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(opencc REQUIRED IMPORTED_TARGET opencc)
pkg_check_modules(marisa REQUIRED IMPORTED_TARGET marisa)

if(NOT KEPUB_SANITIZER)
  message(STATUS "Use mimalloc")
  set(MIMALLOC_OBJECT "/usr/local/lib/mimalloc.o")
endif()

include_directories(${KEPUB_BINARY_DIR})

# ---------------------------------------------------------------------------------------
# Build blob file
# ---------------------------------------------------------------------------------------
file(COPY "${KEPUB_SOURCE_DIR}/blob/SourceHanSansSC-Bold.ttf"
     DESTINATION ${KEPUB_BINARY_DIR})
file(COPY "${KEPUB_SOURCE_DIR}/blob/style.css" DESTINATION ${KEPUB_BINARY_DIR})

add_library(blob STATIC "${KEPUB_SOURCE_DIR}/src/blob.s")
target_compile_options(blob PRIVATE "-Wno-unused-command-line-argument")

# ---------------------------------------------------------------------------------------
# Build static library
# ---------------------------------------------------------------------------------------
add_library(${KEPUB_LIBRARY} STATIC ${MIMALLOC_OBJECT} ${KEPUB_LIBRARY_SRC})

target_include_directories(${KEPUB_LIBRARY}
                           PUBLIC "${KEPUB_SOURCE_DIR}/include")
target_link_libraries(
  ${KEPUB_LIBRARY}
  PRIVATE blob
          fmt::fmt
          pugixml::pugixml
          indicators::indicators
          Microsoft.GSL::GSL
          klib::klib
          ${Boost_LIBRARIES}
          simdjson::simdjson
          PkgConfig::opencc
          PkgConfig::marisa
          TBB::tbb
          re2::re2)
set_target_properties(${KEPUB_LIBRARY} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# ---------------------------------------------------------------------------------------
# Build shared library
# ---------------------------------------------------------------------------------------
add_library(${KEPUB_LIBRARY}-shared SHARED ${MIMALLOC_OBJECT}
                                           ${KEPUB_LIBRARY_SRC})
generate_export_header(${KEPUB_LIBRARY}-shared BASE_NAME ${PROJECT_NAME})

target_include_directories(${KEPUB_LIBRARY}-shared
                           PUBLIC "${KEPUB_SOURCE_DIR}/include")
target_link_libraries(
  ${KEPUB_LIBRARY}-shared
  PRIVATE blob
          fmt::fmt
          pugixml::pugixml
          indicators::indicators
          Microsoft.GSL::GSL
          klib::klib
          ${Boost_LIBRARIES}
          simdjson::simdjson
          PkgConfig::opencc
          PkgConfig::marisa
          TBB::tbb
          re2::re2)
set_target_properties(
  ${KEPUB_LIBRARY}-shared
  PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
             VERSION ${KEPUB_VERSION}
             SOVERSION ${KEPUB_VERSION_MAJOR})

# ---------------------------------------------------------------------------------------
# Build executable
# ---------------------------------------------------------------------------------------
add_subdirectory(tool)

# ---------------------------------------------------------------------------------------
# Build test
# ---------------------------------------------------------------------------------------
if(BUILD_TESTING AND KEPUB_BUILD_TEST)
  message(STATUS "Build test")
  add_subdirectory(test)
endif()

# ---------------------------------------------------------------------------------------
# Install target
# ---------------------------------------------------------------------------------------
include(Install)

# https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake
configure_file("${KEPUB_SOURCE_DIR}/cmake/CMakeUninstall.cmake.in"
               "${KEPUB_BINARY_DIR}/CMakeUninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P
                                    ${KEPUB_BINARY_DIR}/CMakeUninstall.cmake)
