cmake_minimum_required(VERSION 3.5)

project(test_ApkQt LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)

find_package(Qt5 CONFIG REQUIRED COMPONENTS Core)
find_package(ApkQt CONFIG REQUIRED)

add_executable(test_ApkQt
    test_libapk-qt.cpp
)

target_link_libraries(test_ApkQt PRIVATE
    Qt5::Core
    apk-qt
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
