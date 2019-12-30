add_executable(test_openclose test_openclose.cpp)
target_link_libraries(test_openclose apk-qt Qt5::Core)
target_include_directories(test_openclose PRIVATE ../src)
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_openclose PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()