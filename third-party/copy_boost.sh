#!/usr/bin/env sh
ORIG_DIR=`pwd`
cd $(dirname $(readlink -f $0))

#================================
# setup variables
#================================

BOOST_VERSION=1_55_0
BOOST_DYN_VERSION=1.55.0
BOOST_PATH=../../boost/$BOOST_VERSION
DEST_PATH=./boost
LIB_PATH=$DEST_PATH/lib
# these are default paths when building on linux; consider customization
BOOST_STATIC_BUILD_PATH=build/gcc-4.8/release/link-static/threading-multi
BOOST_MULTI_BUILD_PATH=build/gcc-4.8/release/threading-multi
# static link
STATIC_DATE_TIME_PATH=$BOOST_PATH/bin.v2/libs/date_time/$BOOST_STATIC_BUILD_PATH
STATIC_SYSTEM_PATH=$BOOST_PATH/bin.v2/libs/system/$BOOST_STATIC_BUILD_PATH
STATIC_REGEX_PATH=$BOOST_PATH/bin.v2/libs/regex/$BOOST_STATIC_BUILD_PATH
# dyn link
MULTI_DATE_TIME_PATH=$BOOST_PATH/bin.v2/libs/date_time/$BOOST_MULTI_BUILD_PATH
MULTI_SYSTEM_PATH=$BOOST_PATH/bin.v2/libs/system/$BOOST_MULTI_BUILD_PATH
MULTI_REGEX_PATH=$BOOST_PATH/bin.v2/libs/regex/$BOOST_MULTI_BUILD_PATH

#================================
# wipe out old files
#================================
if [ ! -d "$DEST_PATH" ]; then
	mkdir "$DEST_PATH"
fi

#================================
# copy inclusions
#================================
cp -R "$BOOST_PATH/boost" "$DEST_PATH/boost/";

#================================
# copy libraries
#================================
if [ ! -d "$LIB_PATH" ]; then
	mkdir "$LIB_PATH"
fi

# Copy only the libraries we specifically need, as altogether these are massive
cp "$STATIC_DATE_TIME_PATH/libboost_date_time.a" "$LIB_PATH/"
cp "$STATIC_SYSTEM_PATH/libboost_system.a" "$LIB_PATH/"
cp "$STATIC_REGEX_PATH/libboost_regex.a" "$LIB_PATH/"
cp "$MULTI_DATE_TIME_PATH/libboost_date_time.so.$BOOST_DYN_VERSION" "$LIB_PATH/"
cp "$MULTI_SYSTEM_PATH/libboost_system.so.$BOOST_DYN_VERSION" "$LIB_PATH/"
cp "$MULTI_REGEX_PATH/libboost_regex.so.$BOOST_DYN_VERSION" "$LIB_PATH/"


cd $ORIG_DIR

