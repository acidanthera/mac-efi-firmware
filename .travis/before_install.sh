#!/bin/bash

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
  cd /tmp/
  wget https://releases.linaro.org/components/toolchain/binaries/latest-5.2/aarch64-linux-gnu/gcc-linaro-5.2-2015.11-1-x86_64_aarch64-linux-gnu.tar.xz -O gcc-aarch64.tar.gz
  tar -xf gcc-aarch64.tar.gz
  export PATH=$PATH:${PWD}/gcc-linaro-5.2-2015.11-1-x86_64_aarch64-linux-gnu/bin/

  if [ "$CC" = "gcc" ]; then
    wget https://releases.linaro.org/components/toolchain/binaries/latest-5.2/arm-linux-gnueabi/gcc-linaro-5.2-2015.11-1-x86_64_arm-linux-gnueabi.tar.xz -O gcc-arm.tar.gz 
    tar -xf gcc-arm.tar.gz
    export PATH=$PATH:${PWD}/gcc-linaro-5.2-2015.11-1-x86_64_arm-linux-gnueabi/bin/

    mkdir gcc-bin
    cd gcc-bin/

    export GCC49_ARM_PREFIX=arm-linux-gnueabi-
    export GCC49_AARCH64_PREFIX=aarch64-linux-gnu-
    export GCC49_BIN=${PWD}/
    export PATH=$PATH:$GCC49_BIN

    ln -s /usr/bin/addr2line    addr2line
    ln -s /usr/bin/as           as
    ln -s /usr/bin/c++filt      c++filt
    ln -s /usr/bin/dwp          dwp
    ln -s /usr/bin/elfedit      elfedit
    ln -s /usr/bin/gold         gold
    ln -s /usr/bin/gprof        gprof
    ln -s /usr/bin/ld           ld
    ln -s /usr/bin/ld.bfd       ld.bfd
    ln -s /usr/bin/ld.gold      ld.gold
    ln -s /usr/bin/objcopy      objcopy
    ln -s /usr/bin/objdump      objdump
    ln -s /usr/bin/readelf      readelf
    ln -s /usr/bin/size         size
    ln -s /usr/bin/strings      strings
    ln -s /usr/bin/strip        strip
    ln -s /usr/bin/make         arm-linux-gnueabi-make
    ln -s /usr/bin/make         aarch64-linux-gnu-make
    ln -s /usr/bin/make         make
    ln -s /usr/bin/gcc-5        gcc
    ln -s /usr/bin/gcc-ar-5     ar
    ln -s /usr/bin/gcc-ar-5     gcc-ar
    ln -s /usr/bin/gcc-nm-5     nm
    ln -s /usr/bin/gcc-nm-5     gcc-nm
    ln -s /usr/bin/gcc-ranlib-5 ranlib
    ln -s /usr/bin/gcc-ranlib-5 gcc-ranlib
    ln -s /usr/bin/gcov-5       gcov
    ln -s /usr/bin/gcov-tool-5  gcov-tool
  elif [ "$CC" = "clang" ]; then
    mkdir clang-bin
    cd clang-bin/

    export CLANG35_AARCH64_PREFIX=aarch64-linux-gnu-
    export CLANG35_BIN=${PWD}/
    export PATH=$PATH:$CLANG35_BIN

    ln -s /usr/bin/asan_symbolize-3.6           asan_symbolize
    ln -s /usr/bin/c-index-test-3.6             c-index-test
    ln -s /usr/bin/clang++-3.6                  clang++
    ln -s /usr/bin/clang-3.6                    clang
    ln -s /usr/bin/clang-apply-replacements-3.6 clang-apply-replacements
    ln -s /usr/bin/clang-check-3.6              clang-check
    ln -s /usr/bin/clang-query-3.6              clang-query
    ln -s /usr/bin/clang-rename-3.6             clang-rename
    ln -s /usr/bin/clang-tblgen-3.6             clang-tblgen
    ln -s /usr/bin/clang-tidy-3.6               clang-tidy
    ln -s /usr/bin/pp-trace-3.6                 pp-trace
    ln -s /usr/bin/scan-build-3.6               scan-build
    ln -s /usr/bin/scan-view-3.6                scan-view
  fi

  cd $TRAVIS_BUILD_DIR/
fi 
