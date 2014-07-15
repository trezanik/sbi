#!/usr/bin/env sh
ORIG_DIR=`pwd`
cd $(dirname $(readlink -f $0))

#================================
# setup variables
#================================

set +x
JSON_SPIRIT_VERSION=4.08
JSON_SPIRIT_PATH=../../json_spirit/$JSON_SPIRIT_VERSION
DEST_PATH=./json_spirit

#================================
# wipe out old files
#================================
rm -rf "$DEST_PATH"
mkdir "$DEST_PATH"

#================================
# copy inclusions
#================================
mkdir "$DEST_PATH/json_spirit/"
# Only want the headers from the json_spirit folder; no need to link with the
# object files, project uses header-only
for i in `find $JSON_SPIRIT_PATH/json_spirit/*.h`; do
	cp $i $DEST_PATH/json_spirit/;
done


cd $ORIG_DIR
