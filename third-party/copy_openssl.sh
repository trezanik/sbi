#!/usr/bin/env sh
ORIG_DIR=`pwd`
cd $(dirname $(readlink -f $0))

#================================
# setup variables
#================================

set +x
OPENSSL_VERSION=1.0.1h
OPENSSL_PATH=../../openssl/$OPENSSL_VERSION
DEST_PATH=./openssl

#================================
# wipe out old files
#================================
rm -rf "$DEST_PATH"
mkdir "$DEST_PATH"

#================================
# copy inclusions
#================================
mkdir "$DEST_PATH/openssl/"
for i in `find $OPENSSL_PATH/include`; do
	cp $i $DEST_PATH/openssl/;
done

#================================
# copy libraries
#================================
mkdir "$DEST_PATH/lib"
cp "$OPENSSL_PATH/libcrypto.a" "$DEST_PATH/lib/"
cp "$OPENSSL_PATH/libssl.a" "$DEST_PATH/lib/"


cd $ORIG_DIR

