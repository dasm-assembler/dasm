#!/bin/sh
# builds dasm for all three supported platforms (linux, windows, macOS/darwin)

#cd ~
#rm -rf dasm
#git clone --single-branch --branch master https://github.com/dasm-assembler/dasm.git
#cd dasm
cd ..

#step 1: build dasm for Linux x86 32-bit
export CFLAGS=' -m32'
export LDFLAGS=' -m32 -L/usr/lib32'
make
cp src/dasm ~/dasm.Linux.x86
make clean

#step 2: build dasm for macOS x86 32-bit
export CC=i686-w64-mingw32-gcc
export CFLAGS=' -m32'
export LDFLAGS=' -m32 -L/usr/lib32'
make
cp src/dasm ~/dasm.exe
make clean

#step 3: build dasm for macOS x86 32-bit
export LD=$CC
export CC=i686-apple-darwin10-gcc
export CFLAGS=' -m32 -arch i386 '
export LDFLAGS='-m32 -L/usr/i686-apple-darwin10/lib -arch i386'
make
cp src/dasm ~/dasm.Darwin.x86
make clean

#
unset LD
unset CC
unset CFLAGS
unset LDFLAGS

