# libapk-qt

Alpine Package Keeper (apk) C++/Qt bindings

Aim is to provide a simple high-level
(sometimes maybe even over-simplified)
interface to Alpine Linux's libapk
(which does not even exist yet).

## Building

Need to fetch Alpine's apk-tools sources from another repository:

```
git submodule init
git submodule update
```

Install build dependencies (example for Alpine):
```
sudo apk add make cmake gcc g++
sudo apk add openssl-dev zlib-dev qt5-qtbase-dev
```

Then a usual cmake build process, for example:

```
mkdir build
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=lib
cmake --build build/ -j8
sudo cmake --build build/ --target install
```

Requirements are only:

 * OpenSSL
 * ZLIB
 * Qt5 Core

Other useful cmake build options:

 * APKTOOLS_DIR Path to Alpine apk-tools source directory, if you are not using default git submodule
 * BUILD_SHARED_LIBS (default ON) shared libs, that's obviously what we want
 * BUILD_TESTING (default OFF) build tests and enable `make test` target.
 * BUILD_DEVELOPER_MODE (default OFF) developer-only extra debug information. Don't enable in production

## Overriding fake root usage from environment variable

`QTAPK_FAKEROOT` can be set in environment to the path of fake chroot directory to force usage of fake root by library.
