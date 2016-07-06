#!/bin/sh

GIT_SHA1=`(git rev-parse --short HEAD 2> /dev/null || echo 00000000) | head -n1`
BUILD_ID=`uname -n`"-"`date +%s`

test -f src/release.h || touch src/release.h
(cat src/release.h | grep SHA1 | grep $GIT_SHA1) && exit 0
echo "#define KJU_GIT_SHA1 \"$GIT_SHA1\"" > src/release.h
echo "#define KJU_BUILD_ID \"$BUILD_ID\"" >> src/release.h
touch src/release.c
