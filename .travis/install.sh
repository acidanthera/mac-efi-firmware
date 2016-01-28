#!/bin/bash

set -e

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
  cd /tmp/
  svn co https://llvm.org/svn/llvm-project/llvm/trunk/include/llvm-c/ /usr/local/include/llvm-c/ -q
  git clone https://github.com/opensource-apple/cctools.git
  cd cctools
  set +e  # the build of a library not used by this project usually fails.
  make -s
  set -e
  cd efitools
  make install -s
  cd $TRAVIS_BUILD_DIR
fi
