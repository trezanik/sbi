#!/usr/bin/env sh
ORIG_DIR=`pwd`
cd $(dirname $(readlink -f $0))

#================================
# setup variables
#================================

set +x
LIBCONFIG_VERSION=1.4.9
LIBCONFIG_PATH=../../libconfig/$LIBCONFIG_VERSION
DEST_PATH=./libconfig

#================================
# wipe out old files
#================================
rm -rf "$DEST_PATH"
mkdir "$DEST_PATH"

#================================
# copy inclusions
#================================
mkdir "$DEST_PATH/libconfig/"
# Sadly, libconfig stored project files and other things in the same folder
# as the headers and the source, Wildcard the header files in as a fix.
for i in `find $LIBCONFIG_PATH/lib/*.h`; do
	cp $i $DEST_PATH/libconfig/;
done
for i in `find $LIBCONFIG_PATH/lib/*.hh`; do
	cp $i $DEST_PATH/libconfig/;
done
for i in `find $LIBCONFIG_PATH/lib/*.h++`; do
	cp $i $DEST_PATH/libconfig/;
done

#================================
# copy libraries
#================================
mkdir "$DEST_PATH/lib"
if [ -d "$LIBCONFIG_PATH/lib/.libs" ]; then
	cp "$LIBCONFIG_PATH/lib/.libs/libconfig++.a" "$DEST_PATH/lib/"
	cp "$LIBCONFIG_PATH/lib/.libs/libconfig++.so" "$DEST_PATH/lib/"
	cp "$LIBCONFIG_PATH/lib/.libs/libconfig++.a" "$DEST_PATH/lib/"
	cp "$LIBCONFIG_PATH/lib/.libs/libconfig++.so" "$DEST_PATH/lib/"
fi


cd $ORIG_DIR

