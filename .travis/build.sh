#!/bin/bash

set -e

if [ "$CC" = "clang" ]; then
  if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    build -t XCODE5 -a IA32 -a X64 -b DEBUG -b RELEASE -p AppleModulePkg/AppleModulePkg.dsc -q
  else
    build -t CLANG35 -a AARCH64 -a ARM -b DEBUG -b RELEASE -p AppleModulePkg/AppleModulePkg.dsc -q
  fi
elif [ "$CC" = "gcc" ]; then
  build -t GCC49 -a AARCH64 -a IA32 -a X64 -a ARM -b DEBUG -b RELEASE -p AppleModulePkg/AppleModulePkg.dsc -q
fi
