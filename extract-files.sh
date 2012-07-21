#!/bin/sh

VENDOR=asus
DEVICE=grouper

BASE=../../../vendor/$VENDOR/$DEVICE/proprietary

while getopts ":nh" options
do
  case $options in
    n ) NC=1 ;;
    h ) echo "Usage: `basename $0` [OPTIONS] "
        echo "  -n  No cleanup"
        echo "  -h  Show this help"
        exit ;;
    * ) ;;
  esac
done

if [ "x$NC" != "x1" ];
then
    rm -rf $BASE/*
fi

for FILE in `cat proprietary-files.txt | grep -v ^# | grep -v ^$`; do
    DIR=`dirname $FILE`
    if [ ! -d $BASE/$DIR ]; then
        mkdir -p $BASE/$DIR
    fi
    echo -n "$FILE: "
    adb pull /system/$FILE $BASE/$FILE
done

./setup-makefiles.sh
