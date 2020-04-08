add_executable(test_openclose test_openclose.cpp)
target_link_libraries(test_openclose apk-qt Qt5::Core)
target_include_directories(test_openclose PRIVATE ../src ${PROJECT_BINARY_DIR})
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_openclose PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

add_executable(test_db_update test_db_update.cpp)
target_link_libraries(test_db_update apk-qt Qt5::Core)
target_include_directories(test_db_update PRIVATE ../src ${PROJECT_BINARY_DIR})
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_db_update PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

add_executable(test_fakeroot test_fakeroot.cpp)
target_link_libraries(test_fakeroot apk-qt Qt5::Core)
target_include_directories(test_fakeroot PRIVATE ../src ${PROJECT_BINARY_DIR})
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_fakeroot PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

add_executable(test_add test_add.cpp)
target_link_libraries(test_add apk-qt Qt5::Core)
target_include_directories(test_add PRIVATE ../src ${PROJECT_BINARY_DIR})
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_add PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

add_executable(test_del test_del.cpp)
target_link_libraries(test_del apk-qt Qt5::Core)
target_include_directories(test_del PRIVATE ../src ${PROJECT_BINARY_DIR})
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_del PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

add_executable(test_print_packages test_print_packages.cpp)
target_link_libraries(test_print_packages apk-qt Qt5::Core)
target_include_directories(test_print_packages PRIVATE ../src ${PROJECT_BINARY_DIR})
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_print_packages PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

add_executable(test_changeset test_changeset.cpp)
target_link_libraries(test_changeset apk-qt Qt5::Core)
target_include_directories(test_changeset PRIVATE ../src ${PROJECT_BINARY_DIR})
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_changeset PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

add_executable(test_progressfd test_progressfd.cpp)
target_link_libraries(test_progressfd apk-qt Qt5::Core)
target_include_directories(test_progressfd PRIVATE ../src ${PROJECT_BINARY_DIR})
if (BUILD_DEVELOPER_MODE)
    target_compile_definitions(test_progressfd PRIVATE
        QTAPK_DEVELOPER_BUILD
    )
endif()

###################################
# Tests are executed in order, so:
# 1) ceate fakeroot
# 2) run run tests
# 3) remove fakeroot

set(FAKEROOT_DIR ${CMAKE_CURRENT_BINARY_DIR}/fakeroot)

# Run this "test" first so it prepares the test environment
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

add_test(NAME test_add
    COMMAND test_add --root ${FAKEROOT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME test_del
    COMMAND test_del --root ${FAKEROOT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME test_print_packages
    COMMAND test_print_packages --root ${FAKEROOT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME test_changeset
    COMMAND test_changeset --root ${FAKEROOT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME test_progressfd
    COMMAND test_progressfd --root ${FAKEROOT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Run this test last, so it can clean up the test environment
add_test(NAME clean_fakeroot
    COMMAND rm -rf ${FAKEROOT_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
