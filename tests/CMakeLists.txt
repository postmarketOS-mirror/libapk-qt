add_executable(test_openclose test_openclose.cpp)
target_link_libraries(test_openclose apk-qt Qt5::Core)
target_include_directories(test_openclose PRIVATE ../src)
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_openclose PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

add_executable(test_db_update test_db_update.cpp)
target_link_libraries(test_db_update apk-qt Qt5::Core)
target_include_directories(test_db_update PRIVATE ../src)
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_db_update PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

add_executable(test_fakeroot test_fakeroot.cpp)
target_link_libraries(test_fakeroot apk-qt Qt5::Core)
target_include_directories(test_fakeroot PRIVATE ../src)
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_fakeroot PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

###################################
# Tests are executed in order, so:
# 1) ceate fakeroot
# 2) run run tests
# 3) remove fakeroot

set(FAKEROOT_DIR ${CMAKE_CURRENT_BINARY_DIR}/fakeroot)

add_test(NAME prepare_fakeroot
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/testdata/create_fakeroot.sh ${FAKEROOT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME test_openclose
    COMMAND test_openclose --root ${FAKEROOT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME test_db_update
    COMMAND test_db_update --root ${FAKEROOT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME clean_fakeroot
    COMMAND rm -rf ${FAKEROOT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
