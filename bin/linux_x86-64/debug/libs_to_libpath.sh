#!/usr/bin/env sh
ORIG_DIR=`pwd`
cd $(dirname $(readlink -f $0))

UID=$(id -u)
if [ $UID -ne 0 ]; then
	echo "This script must be run as root!"
	echo "e.g. $ sudo $(readlink -f $0)"
	exit
fi

# your system and preferences may vary; adjust as needed
SOURCE_PATH=../../../lib/linux_x86-64/debug
DEST_PATH=/usr/local/lib/sbi

# add standard library path, and ensure it can be found
if [ ! -d $DEST_PATH ]; then
	mkdir $DEST_PATH
	echo "$DEST_PATH" > /etc/ld.so.conf.d/sbi.conf
	exec ldconfig
fi


# third-party requirements (copy, don't move)
cp ../../../third-party/libconfig/lib/libconfig++.so $DEST_PATH
cp ../../../third-party/boost/lib/libboost_system.so $DEST_PATH

# finally, copy the libraries into their destination (don't move, for rebuild)
cp $SOURCE_PATH/*.so $DEST_PATH


cd $ORIG_DIR
