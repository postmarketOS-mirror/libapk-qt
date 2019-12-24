# libapk-qt

Alpine Package Keeper (apk) Qt bindings

# Building

Need to fetch Alpine's apk-tools sources from another repository:

```
git submodule init
git submodule update
```

Then a usual cmake build process. Requirements are only:
 * OpenSSL
 * ZLIB
 * Qt Core
