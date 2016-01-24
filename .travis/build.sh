#!/bin/bash

set -e

if [ "${CC}" = "clang" ]; then
  if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
    build -t XCODE5 -b DEBUG -b RELEASE -a IA32 -a X64 -p AppleModulePkg/AppleModulePkg.dsc
  else
    build -t CLANG35 -b DEBUG -b RELEASE -a IA32 -a X64 -p AppleModulePkg/AppleModulePkg.dsc
  fi
elif [ "${CC}" = "gcc" ]; then
  build -t GCC49 -b DEBUG -b RELEASE -a IA32 -a X64 -p AppleModulePkg/AppleModulePkg.dsc
fi
