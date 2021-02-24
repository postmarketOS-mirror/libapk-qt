# SPDX-License-Identifier: GPL-2.0-or-later

cmake_minimum_required(VERSION 3.5)

set(QTAPK_VERSION_MAJOR 0)
set(QTAPK_VERSION_MINOR 4)
set(QTAPK_VERSION_PATCH 0)
set(QTAPK_VERSION_STRING "${QTAPK_VERSION_MAJOR}.${QTAPK_VERSION_MINOR}.${QTAPK_VERSION_PATCH}")

project(apk-qt
    LANGUAGES C CXX
    VERSION ${QTAPK_VERSION_MAJOR}.${QTAPK_VERSION_MINOR}.${QTAPK_VERSION_PATCH}
)

# CMP0048: The project() command manages VERSION variables. (NEW - enable)
cmake_policy(SET CMP0048 NEW)
# CMP0071: Let AUTOMOC and AUTOUIC process GENERATED files. (NEW - enable)
cmake_policy(SET CMP0071 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
include(FeatureSummary)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTING "Build tests (for developers)" OFF)
option(USE_STATIC_LIBAPK "Use statically linked version of libapk for safer upgrades" OFF)

# some really really useful settings from KDE's extra-cmake-modules
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# Required by ApkQt
find_package(Qt5 CONFIG REQUIRED COMPONENTS Core)
find_package(LibApk REQUIRED)

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

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/apk-qt.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/apk-qt.pc"
    @ONLY
)

install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ApkQtConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ApkQtConfigVersion.cmake"
    DESTINATION ${APKQT_CMAKE_CONFIG_INSTALL_DIR}
)

install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/apk-qt.pc"
    DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig"
)

add_subdirectory(src)

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
