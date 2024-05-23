#!/bin/sh

BUILDDIR='build/'

mkdir -p $BUILDDIR
cd $BUILDDIR
cmake $1 ..
cmake --build .
cd ../

