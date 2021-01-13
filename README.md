# libapk-qt

Alpine Package Keeper (apk) C++/Qt bindings

Aim is to provide a simple high-level
(sometimes maybe even over-simplified)
interface to Alpine Linux's package
manager interface - libapk.

## Building

Install build dependencies (example for Alpine):
```
sudo apk add build-base cmake
sudo apk add qt5-qtbase-dev apk-tools-dev
```

Then a usual cmake build process, for example:

```
mkdir build
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=lib
cmake --build build/ -j$(nproc)
sudo cmake --build build/ --target install
```

Requirements are only:

 * Qt5Core
 * apk-tools development package

Other useful cmake build options:

 * BUILD_SHARED_LIBS (default ON) shared libs, that's often what we want
 * BUILD_TESTING (default OFF) build tests and enable `make test` target.
 * BUILD_DEVELOPER_MODE (default OFF) developer-only extra debug information. Don't enable in production!
 * USE_STATIC_LIBAPK (default OFF) Link static libapk.a for safer upgrades (do not depend on shared libapk.so)

### Running tests
After successful build with `-DBUILD_TESTING=ON` option set:
```
cd build/ && env CTEST_OUTPUT_ON_FAILURE=1 ctest -v
```

## Running
### Overriding fake root usage from environment variable

 * `QTAPK_FAKEROOT` can be set in environment to the path of fake chroot directory to force usage of fake root by library.
