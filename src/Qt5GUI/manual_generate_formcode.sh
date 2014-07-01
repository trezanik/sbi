#!/usr/bin/env sh
ORIG_DIR=`pwd`
cd $(dirname $(readlink -f $0))
##################################################################################
# Manually executes the equivalent of what qmake does; should be handy if needed #
##################################################################################

# Adjust as needed
QT_PATH=../../../Qt/5.3/gcc_64/bin
FORMS_PATH=./forms
GEN_PATH=./generated

for i in `find $FORMS_PATH/*.ui`; do
	# remove directory paths and .ui suffix
	filename=$(basename "$i" .ui)
	echo "$i"
	# Generate the header
	$QT_PATH/uic "$i" -o "$GEN_PATH/ui_$filename.h"
	# Generate the moc
	$QT_PATH/moc "$filename.h" -o "$GEN_PATH/moc_$filename.cc"
done


cd $ORIG_DIR

