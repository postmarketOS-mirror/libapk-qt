# Some useful compile definitions to write safer and cleaner Qt code
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

# public headers (also used for installing headers)
set(QTAPK_PUBLIC_HEADERS
    QtApk
    QtApkDatabase.h
    QtApkDatabaseAsync.h
    QtApkChangeset.h
    QtApkFlags.h
    QtApkPackage.h
    QtApkRepository.h
    QtApkTransaction.h
)

set(QTAPK_SOURCES
    QtApkDatabase.cpp
    QtApkDatabaseAsync.cpp
    QtApkChangeset.cpp
    QtApkPackage.cpp
    QtApkRepository.cpp
    QtApkTransaction.cpp
    QtApk_metatypes.cpp
    private/QtApkDatabase_private.h
    private/QtApkDatabase_private.cpp
    private/QtApkDatabaseAsync_private.h
    private/QtApkDatabaseAsync_private.cpp
    private/QtApkTransaction_private.h
    private/QtApkTransaction_private.cpp
    private/libapk_c_wrappers.h
    private/libapk_c_wrappers.c
)


add_library(apk-qt
    ${QTAPK_PUBLIC_HEADERS}
    ${QTAPK_SOURCES}
)


target_link_libraries(apk-qt PUBLIC Qt5::Core)

if (USE_STATIC_LIBAPK)
    target_link_libraries(apk-qt PRIVATE LibApk::LibApkStatic)
else()
    target_link_libraries(apk-qt PRIVATE LibApk::LibApk)
endif()


include(GenerateExportHeader)

generate_export_header(apk-qt
    EXPORT_FILE_NAME  qtapk_exports.h
    EXPORT_MACRO_NAME QTAPK_EXPORTS
)

configure_file(QtApk_version.h.in QtApk_version.h)

set_target_properties(apk-qt
    PROPERTIES
    EXPORT_NAME ApkQt
    CXX_VISIBILITY_PRESET hidden
    SOVERSION "${QTAPK_VERSION_MAJOR}.${QTAPK_VERSION_MINOR}"
    VERSION "${QTAPK_VERSION_STRING}"
)

if (BUILD_SHARED_LIBS)
    # Shared libraries should be build with -fPIC
    # applies also to a static library that is linked into shared one
    set_target_properties(apk-qt
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
        ${QTAPK_PUBLIC_HEADERS}
        ${PROJECT_BINARY_DIR}/src/qtapk_exports.h
        ${PROJECT_BINARY_DIR}/src/QtApk_version.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ApkQt
)
