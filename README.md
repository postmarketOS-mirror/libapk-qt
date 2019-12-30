# libapk-qt

Alpine Package Keeper (apk) Qt bindings

# Building

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

Then a usual cmake build process:

```
mkdir build
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
cd build
cmake --build . -j8
sudo cmake --build . --target install
```

Requirements are only:

 * OpenSSL
 * ZLIB
 * Qt Core

Other useful cmake build options:

 * BUILD_SHARED_LIBS (default ON) shared libs, that's obviously what we want
 * BUILD_TESTING (default OFF) developer's tests. Those are manual tests, not automatic
 * BUILD_DEVELOPER_MODE (default OFF) developer-only extra debug information. Don't enable in production
