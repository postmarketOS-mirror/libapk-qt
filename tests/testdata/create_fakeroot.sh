#!/bin/sh
ROOT=$1

if [ x$ROOT == "x" ]; then
    echo "Usage: $0 FAKEROOT_DIR"
    exit 1
fi

if [ ! -d $ROOT ]; then
    echo "[$ROOT] is not a directory!"
    exit 1
fi

mkdir -p $ROOT/etc/apk/keys
mkdir -p $ROOT/lib/apk/db
mkdir -p $ROOT/var/cache/apk
mkdir -p $ROOT/var/cache/misc

cp -a files/*.pub         $ROOT/etc/apk/keys/

cp -a files/arch          $ROOT/etc/apk/
cp -a files/repositories  $ROOT/etc/apk/
cp -a files/world         $ROOT/etc/apk/

cp -a files/installed     $ROOT/lib/apk/db
cp -a files/lock          $ROOT/lib/apk/db
cp -a files/scripts.tar   $ROOT/lib/apk/db
cp -a files/triggers      $ROOT/lib/apk/db

# APKINDEX cache should match repositories file
cp -aR files/APKINDEX.*.tar.gz  $ROOT/var/cache/apk
