#!/bin/bash
SOURCE_DIR=$1
PATCH_FILE=$2
PATCH_STATUS=`head $SOURCE_DIR/tcpdump.c | grep "PATCH FOR STELLAR-DUMP"`

if [ -z "$PATCH_STATUS" ] 
then
    echo "starting patch tcpdump..."
    patch -N -p1 -d $SOURCE_DIR -i $PATCH_FILE
else
    echo "alread patched, skip... "
fi
