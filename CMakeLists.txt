cmake_minimum_required(VERSION 3.5)
project(apk-qt
    LANGUAGES C CXX
    VERSION 2.10.4)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0071 OLD)

include(GNUInstallDirs)
include(FeatureSummary)

set(APKTOOLS_DIR "apk-tools" CACHE STRING "Path to Alpine apk-tools source directory")
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTING "Build tests (for developers)" OFF)
option(BUILD_DEVELOPER_MODE "Developer build (extra debug output, larger binaries, only for developers!)" OFF)

# some really really useful settings from KDE's extra-cmake-modules
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Qt5 CONFIG REQUIRED COMPONENTS Core)

add_definitions(
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    -DQT_STRICT_ITERATORS
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -D_GNU_SOURCE
)

add_library(fetch STATIC
    ${APKTOOLS_DIR}/libfetch/common.c
    ${APKTOOLS_DIR}/libfetch/common.h
    ${APKTOOLS_DIR}/libfetch/fetch.c
    ${APKTOOLS_DIR}/libfetch/fetch.h
    ${APKTOOLS_DIR}/libfetch/file.c
    ${APKTOOLS_DIR}/libfetch/ftp.c
    ${APKTOOLS_DIR}/libfetch/http.c
    ${APKTOOLS_DIR}/libfetch/openssl-compat.c
    ${APKTOOLS_DIR}/libfetch/openssl-compat.h
    # source files for preprocessing
    ${APKTOOLS_DIR}/libfetch/errlist.sh
    ${APKTOOLS_DIR}/libfetch/ftp.errors
    ${APKTOOLS_DIR}/libfetch/http.errors
    # the foolowing files should be generated
    ${APKTOOLS_DIR}/libfetch/httperr.h
    ${APKTOOLS_DIR}/libfetch/ftperr.h
)

set_source_files_properties(
    ${APKTOOLS_DIR}/libfetch/errlist.sh
    ${APKTOOLS_DIR}/libfetch/ftp.errors
    ${APKTOOLS_DIR}/libfetch/http.errors
    PROPERTIES
    HEADER_FILE_ONLY ON
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}/libfetch/httperr.h
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}/libfetch/http.errors
    COMMENT "Generating error header httperr.h"
    VERBATIM
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}/libfetch/errlist.sh http_errlist http ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}/libfetch/http.errors > ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}/libfetch/httperr.h
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}/libfetch/ftperr.h
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}/libfetch/ftp.errors
    COMMENT "Generating error header ftperr.h"
    VERBATIM
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}/libfetch/errlist.sh ftp_errlist ftp ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}/libfetch/ftp.errors > ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}/libfetch/ftperr.h
)

add_custom_target(
    apply_apk_tools_patches
    COMMENT "Applying custom patches for apk-tools..."
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${APKTOOLS_DIR}
    VERBATIM
    COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/apk-tools-patches/cpp-compat.patch || true
)

# this source files list can be used to build apk executable itself
# it is currently not used in this CMakeLists.txt, but left here for
# future reference
set(APKTOOLS_APKEXE_SOURCES
    ${APKTOOLS_DIR}/src/add.c
    ${APKTOOLS_DIR}/src/apk.c
    ${APKTOOLS_DIR}/src/audit.c
    ${APKTOOLS_DIR}/src/cache.c
    ${APKTOOLS_DIR}/src/del.c
    ${APKTOOLS_DIR}/src/dot.c
    ${APKTOOLS_DIR}/src/fetch.c
    ${APKTOOLS_DIR}/src/fix.c
    ${APKTOOLS_DIR}/src/index.c
    ${APKTOOLS_DIR}/src/info.c
    ${APKTOOLS_DIR}/src/list.c
    ${APKTOOLS_DIR}/src/manifest.c
    ${APKTOOLS_DIR}/src/policy.c
    ${APKTOOLS_DIR}/src/search.c
    ${APKTOOLS_DIR}/src/stats.c
    ${APKTOOLS_DIR}/src/update.c
    ${APKTOOLS_DIR}/src/upgrade.c
    ${APKTOOLS_DIR}/src/ver.c
    ${APKTOOLS_DIR}/src/verify.c
)

# we only statically build libapk, so we use only library sources
set(APKTOOLS_LIB_SOURCES
    ${APKTOOLS_DIR}/src/blob.c
    ${APKTOOLS_DIR}/src/commit.c
    ${APKTOOLS_DIR}/src/common.c
    ${APKTOOLS_DIR}/src/database.c
    ${APKTOOLS_DIR}/src/hash.c
    ${APKTOOLS_DIR}/src/io.c
    ${APKTOOLS_DIR}/src/io_archive.c
    ${APKTOOLS_DIR}/src/io_gunzip.c
    ${APKTOOLS_DIR}/src/io_url.c
    ${APKTOOLS_DIR}/src/package.c
    ${APKTOOLS_DIR}/src/print.c
    ${APKTOOLS_DIR}/src/solver.c
    ${APKTOOLS_DIR}/src/version.c
)

set(${APKTOOLS_HEADERS}
    ${APKTOOLS_DIR}/src/apk_applet.h
    ${APKTOOLS_DIR}/src/apk_archive.h
    ${APKTOOLS_DIR}/src/apk_blob.h
    ${APKTOOLS_DIR}/src/apk_database.h
    ${APKTOOLS_DIR}/src/apk_defines.h
    ${APKTOOLS_DIR}/src/apk_hash.h
    ${APKTOOLS_DIR}/src/apk_io.h
    ${APKTOOLS_DIR}/src/apk_openssl.h
    ${APKTOOLS_DIR}/src/apk_package.h
    ${APKTOOLS_DIR}/src/apk_print.h
    ${APKTOOLS_DIR}/src/apk_provider_data.h
    ${APKTOOLS_DIR}/src/apk_solver_data.h
    ${APKTOOLS_DIR}/src/apk_solver.h
    ${APKTOOLS_DIR}/src/apk_version.h
    ${APKTOOLS_DIR}/src/md5.h
)

# for some reason gcc does not recognize "restrict" in pointer
#     declarations, but recognizes "__restrict"
set_source_files_properties(
    ${APKTOOLS_DIR}/src/blob.c
    PROPERTIES
    COMPILE_DEFINITIONS restrict=__restrict
)

add_library(apk STATIC
    ${APKTOOLS_LIB_SOURCES}
    ${APKTOOLS_HEADERS}
)

# Always apply patches before trying to build libapk
add_dependencies(apk apply_apk_tools_patches)

target_compile_definitions(apk PRIVATE
    APK_VERSION=\"${PROJECT_VERSION}\"
)

target_include_directories(apk PRIVATE
    ${APKTOOLS_DIR}/libfetch
)

target_link_libraries(apk
    PRIVATE
        fetch
    PUBLIC
        ZLIB::ZLIB
        OpenSSL::SSL
        OpenSSL::Crypto
)

set(QTAPK_SOURCES
    src/QtApk.h
    src/QtApk.cpp
    src/QtApkChangeset.h
    src/QtApkChangeset.cpp
    src/QtApkPackage.h
    src/QtApkPackage.cpp
    src/QtApkRepository.h
    src/QtApkRepository.cpp
    src/errorstr.cpp
    src/apk_time_workaround.cpp
)


add_library(apk-qt
    ${QTAPK_SOURCES}
)

if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(apk-qt PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
    message(STATUS "You have enabled DEVELOPER_BUILD! Don't use it in production, it is for development only!")
endif()

target_include_directories(apk-qt PRIVATE
    ${APKTOOLS_DIR}/src
)

target_link_libraries(apk-qt
    PUBLIC
        Qt5::Core
    PRIVATE
        apk
)

include(GenerateExportHeader)

generate_export_header(apk-qt
    EXPORT_FILE_NAME  qtapk_exports.h
    EXPORT_MACRO_NAME QTAPK_EXPORTS
)

set_target_properties(apk-qt
    PROPERTIES
    CXX_VISIBILITY_PRESET hidden
)

if (BUILD_SHARED_LIBS)
    # Shared libraries should be build with -fPIC
    # applies also to a static library that is linked into shared one
    set_target_properties(fetch apk apk-qt
        PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
endif()

# Instal libraries
install(
    TARGETS apk-qt
    EXPORT  ApkQtTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install development headers
install(
    FILES
        src/QtApk.h
        src/QtApkChangeset.h
        src/QtApkPackage.h
        src/QtApkRepository.h
        ${PROJECT_BINARY_DIR}/qtapk_exports.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ApkQt
)

# Install cmake package configuration files
set(APKQT_CMAKE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/ApkQt")

install(
    EXPORT ApkQtTargets
    NAMESPACE ApkQt::
    DESTINATION ${APKQT_CMAKE_CONFIG_INSTALL_DIR}
    FILE ApkQtTargets.cmake
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/ApkQtConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ApkQtConfig.cmake"
    INSTALL_DESTINATION ${APKQT_CMAKE_CONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ApkQtConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ApkQtConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ApkQtConfigVersion.cmake"
    DESTINATION ${APKQT_CMAKE_CONFIG_INSTALL_DIR}
)


if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
