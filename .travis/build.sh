#!/bin/bash

set -e

if [ "$CC" = "clang" ]; then
  if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    build -t XCODE5 -b DEBUG -b RELEASE -a IA32 -a X64 -p AppleModulePkg/AppleModulePkg.dsc -q
  else
    build -t CLANG35 -b DEBUG -b RELEASE -a AARCH64 -p AppleModulePkg/AppleModulePkg.dsc -q
  fi
elif [ "$CC" = "gcc" ]; then
  build -t GCC49 -b DEBUG -b RELEASE -a ARM -a IA32 -a AARCH64 -a X64 -p AppleModulePkg/AppleModulePkg.dsc -q
fi
