#!/bin/sh

KEEP_HELP="no"
if [ "$1" = "--keep-help" ]; then
    KEEP_HELP="yes"
    shift
fi

DEST=$1

rm -rf $DEST
mkdir -p $DEST/.tx
cp *.h $DEST
cp *.cpp $DEST
cp CHANGES $DEST
cp COPYING $DEST
cp Doxyfile $DEST
cp Doxyfile_Metakit $DEST
cp INSTALL $DEST
cp README.txt $DEST
cp metakit-2.4.9.7.patch $DEST
cp portabase.ico $DEST
cp portabase.iss $DEST
cp portabase.pro $DEST
cp portabase.rc $DEST
cp .tx/config $DEST/.tx
cp -R calc $DEST
cp -R color_picker $DEST
cp -R debian $DEST
cp -R doc $DEST
cp -R encryption $DEST
cp -R image $DEST
cp -R metakit $DEST
rm -f $DEST/metakit/builds/*.a
rm -f $DEST/metakit/builds/*.o
rm -f $DEST/metakit/builds/config.*
rm -f $DEST/metakit/builds/Makefile
cp -R qqutil $DEST
cp -R packaging $DEST
cp -R resources $DEST
if [ "$KEEP_HELP" = "no" ]; then
    rm -rf $DEST/resources/help/_build
fi
find $DEST -name '*.DS_Store' -type f -delete
