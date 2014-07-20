#!/usr/bin/env sh
ORIG_DIR=`pwd`
cd $(dirname $(readlink -f $0))
########################################################################
# Manually creates a source file containing the resources from the qrc #
########################################################################

# Adjust as needed
QT_PATH=../../Qt/5.3/gcc_64/bin
GEN_PATH=./generated

if [ ! -d "$GEN_PATH" ]; then
	mkdir "$GEN_PATH"
fi

$QT_PATH/rcc -name core "./sbi.qrc" -o "$GEN_PATH/qrc_sbi.cc"


cd $ORIG_DIR

