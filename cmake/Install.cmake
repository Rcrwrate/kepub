# ---------------------------------------------------------------------------------------
# Install executable
# ---------------------------------------------------------------------------------------
include(GNUInstallDirs)

# https://stackoverflow.com/questions/30398238/cmake-rpath-not-working-could-not-find-shared-object-file
set_target_properties(
  ${SFACG_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${CIWEIMAO_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${ESJZONE_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${LIGHTNOVEL_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${MASIRO_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${GEN_EPUB_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${APPEND_EPUB_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${EXTRACT_EPUB_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

install(
  TARGETS ${KEPUB_LIBRARY}-shared
          ${SFACG_EXECUTABLE}
          ${CIWEIMAO_EXECUTABLE}
          ${ESJZONE_EXECUTABLE}
          ${LIGHTNOVEL_EXECUTABLE}
          ${MASIRO_EXECUTABLE}
          ${GEN_EPUB_EXECUTABLE}
          ${APPEND_EPUB_EXECUTABLE}
          ${EXTRACT_EPUB_EXECUTABLE}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(
  FILES ${KEPUB_SOURCE_DIR}/script/_${SFACG_EXECUTABLE}
        ${KEPUB_SOURCE_DIR}/script/_${CIWEIMAO_EXECUTABLE}
        ${KEPUB_SOURCE_DIR}/script/_${ESJZONE_EXECUTABLE}
        ${KEPUB_SOURCE_DIR}/script/_${LIGHTNOVEL_EXECUTABLE}
        ${KEPUB_SOURCE_DIR}/script/_${MASIRO_EXECUTABLE}
        ${KEPUB_SOURCE_DIR}/script/_${GEN_EPUB_EXECUTABLE}
        ${KEPUB_SOURCE_DIR}/script/_${APPEND_EPUB_EXECUTABLE}
        ${KEPUB_SOURCE_DIR}/script/_${EXTRACT_EPUB_EXECUTABLE}
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/zsh/vendor-completions)

# ---------------------------------------------------------------------------------------
# Support creation of installable packages
# ---------------------------------------------------------------------------------------
# https://cmake.org/cmake/help/latest/cpack_gen/deb.html
# https://cmake.org/cmake/help/latest/module/CPack.html
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_INSTALL_CMAKE_PROJECTS ${KEPUB_BINARY_DIR} ${PROJECT_NAME} ALL .)

# https://cmake.org/cmake/help/latest/cpack_gen/deb.html
set(CPACK_PACKAGE_CONTACT "Kaiser <kaiserlancelot123@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Crawl novels from sfacg, ciweimao, esjzone, lightnovel and masiro; generate, append and extract epub"
)
set(CPACK_PACKAGE_VERSION
    ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# https://cmake.org/cmake/help/latest/cpack_gen/archive.html
set(CPACK_GENERATOR "TXZ;DEB")
# https://cmake.org/cmake/help/latest/cpack_gen/deb.html
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
# https://cmake.org/cmake/help/latest/module/CPack.html#variable:CPACK_THREADS
set(CPACK_THREADS 0)
# https://cmake.org/cmake/help/latest/cpack_gen/deb.html#variable:CPACK_DEBIAN_PACKAGE_SHLIBDEPS
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
