#!/bin/bash

set -e

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
  sudo svn co https://llvm.org/svn/llvm-project/llvm/trunk/include/llvm-c/ /usr/include/llvm-c/ -q
  cd /tmp
  git clone https://github.com/opensource-apple/cctools
  cd cctools
  set +e # the build of a library not used by this project usually fails.
  make
  set -e
  cd efitools
  make install
  cd $TRAVIS_BUILD_DIR
fi
