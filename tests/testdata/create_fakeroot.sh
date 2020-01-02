#!/bin/sh
ROOT=$1
SCRIPT_DIR=$(realpath $(dirname $0) )

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

cp -a $SCRIPT_DIR/files/*.pub         $ROOT/etc/apk/keys/

cp -a $SCRIPT_DIR/files/arch          $ROOT/etc/apk/
cp -a $SCRIPT_DIR/files/repositories  $ROOT/etc/apk/
cp -a $SCRIPT_DIR/files/world         $ROOT/etc/apk/

cp -a $SCRIPT_DIR/files/installed     $ROOT/lib/apk/db
cp -a $SCRIPT_DIR/files/lock          $ROOT/lib/apk/db
cp -a $SCRIPT_DIR/files/scripts.tar   $ROOT/lib/apk/db
cp -a $SCRIPT_DIR/files/triggers      $ROOT/lib/apk/db

# APKINDEX cache should match repositories file
cp -aR $SCRIPT_DIR/files/APKINDEX.*.tar.gz  $ROOT/var/cache/apk
