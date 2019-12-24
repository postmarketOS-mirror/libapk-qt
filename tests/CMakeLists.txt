add_executable(test_openclose test_openclose.cpp)
target_link_libraries(test_openclose apk-qt Qt5::Core)
target_include_directories(test_openclose PRIVATE ../src)
