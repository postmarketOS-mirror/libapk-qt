# SPDX-License-Identifier: GPL-2.0-or-later
#[=======================================================================[.rst:
FindLibApk
--------

Find apk-tools development library.

IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines :prop_tgt:`IMPORTED` target ``LibApk::LibApk``, if
libapk.so has been found.

This module defines :prop_tgt:`IMPORTED` target ``LibApk::LibApkStatic``, if
libapk.a has been found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``LibApk_FOUND``
  True if LIBAPK_INCLUDE_DIR & LIBAPK_LIBRARY are found

``LibApk_LIBRARIES``
  List of libraries when using libapk

``LibApk_INCLUDE_DIRS``
  Where to find the libapk headers.

#]=======================================================================]

find_path(LIBAPK_INCLUDE_DIR
    NAMES apk_version.h
    HINTS /usr/include/apk
    DOC "LibApk include directory"
)

find_library(LIBAPK_LIBRARY
    NAMES libapk.so
    HINTS /lib
    DOC "Libapk shared library"
)

find_library(LIBAPK_LIBRARY_STATIC
    NAMES libapk.a
    HINTS /lib
    DOC "Libapk static library"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    LibApk
    REQUIRED_VARS
        LIBAPK_INCLUDE_DIR 
        LIBAPK_LIBRARY
)

if(LibApk_FOUND)
    set(LibApk_LIBRARIES ${LIBAPK_LIBRARY})
    set(LibApk_INCLUDE_DIRS ${LIBAPK_INCLUDE_DIR})
    if(NOT TARGET LibApk::LibApk)
        add_library(LibApk::LibApk UNKNOWN IMPORTED)
        set_target_properties(LibApk::LibApk PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${LibApk_INCLUDE_DIRS}"
            IMPORTED_LOCATION "${LIBAPK_LIBRARY}"
        )
    endif()
endif()

if(LibApk_FOUND AND LIBAPK_LIBRARY_STATIC)
    if(NOT TARGET LibApk::LibApkStatic)
        add_library(LibApk::LibApkStatic UNKNOWN IMPORTED)
        set_target_properties(LibApk::LibApkStatic PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${LibApk_INCLUDE_DIRS}"
            IMPORTED_LOCATION "${LIBAPK_LIBRARY_STATIC}"
        )
    endif()
endif()

mark_as_advanced(LIBAPK_INCLUDE_DIR LIBAPK_LIBRARY LIBAPK_LIBRARY_STATIC)
