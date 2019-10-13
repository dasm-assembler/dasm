#!/bin/sh
# this script builds platform-specific dasm binaries for all three supported OSs (Linux, Windows and macOS)

cd ..
mkdir -p bin/32bit
mkdir -p bin/64bit
cd src

#step 1: build dasm for Linux 32-bit and 64-bit
make
cp dasm ../bin/64bit/dasm.Linux
make clean
export CFLAGS=' -m32'
export LDFLAGS=' -m32 -L/usr/lib32'
make
cp dasm ../bin/32bit/dasm.Linux
make clean
unset CFLAGS
unset LDFLAGS

#step 2: build dasm for Windows 32-bit and 64-bit
export CC=x86_64-w64-mingw32-gcc
#TODO: fix warnings when compiling for win64 using x86_64-w64-mingw32-gcc
#      the compiler warnings have to do with default size of 'long' on windows
make
cp dasm ../bin/64bit/dasm.exe
make clean
export CC=i686-w64-mingw32-gcc
export CFLAGS=' -m32'
export LDFLAGS=' -m32 -L/usr/lib32'
make
cp dasm ../bin/32bit/dasm.exe
make clean
unset CC
unset CFLAGS
unset LDFLAGS

#step 3: build dasm for macOS 32-bit and 64-bit
export LD=$CC
export CC=i686-apple-darwin10-gcc
make
cp dasm ../bin/64bit/dasm.macOS
make clean
export CFLAGS=' -m32 -arch i386 '
export LDFLAGS=' -m32 -L/usr/i686-apple-darwin10/lib -arch i386'
make
cp dasm ../bin/32bit/dasm.macOS
make clean
unset LD
unset CC
unset CFLAGS
unset LDFLAGS
