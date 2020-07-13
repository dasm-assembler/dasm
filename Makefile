#
# the DASM macro assembler (aka small systems cross assembler)
#
# Copyright (c) 1988-2002 by Matthew Dillon.
# Copyright (c) 1995 by Olaf "Rhialto" Seibert.
# Copyright (c) 2003-2008 by Andrew Davie.
# Copyright (c) 2008 by Peter H. Froehlich.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# Simple hack to build everything, test everything, make a beta
# distribution, or make a real distribution. Default is to just
# build everything. Installation is not implemented yet.

# TODO: need to do documentation stuff as well; don't forget to
# delete automatically generated documentation in clean: below

# just an alias for build really...
all: build
	echo "Build complete, use 'make test' to run tests."

# install, currently not implemented
install: build
	echo "Installation not implemented, you're on your own, sorry."

# just run all the tests
test: build
	@echo "Running tests..."
	(cd test; $(MAKE); cd ..)
	@echo "Tests were run, but testing is not fully automated yet."
	@echo "In other words, don't rely on what you saw too much."

# just build everything and copy binaries to trunk/bin/
build:
	(cd src; $(MAKE); cd ..)
	mkdir -p bin
	cp src/dasm bin/dasm
	cp src/ftohex bin/ftohex

# release version to use for archive name
# supply this to make when you run it as
#   RELEASE=2.20.12
# on the command line, that's the easiest
# thing to do
RELEASE=unknown-version-number
# architecture for including binaries/executables
# supply this to make when you run it as
#   BINARY=osx-ppc  or  BINARY=beos-x86  or  ...
# on the command line, that's the easiest
# thing to do; note that this only affects
# the name of the archive, the binaries
# are always put into a trunk/bin folder
# if left empty, source distribution is assumed...
BINARY=

# binaries
BINS=bin/*
# documentation
DOCS=AUTHORS ChangeLog LICENSE CREDITS NEWS README  doc/*
# support files for various machines
MACS=machines/atari2600/* machines/channel-f/*
# source files for dasm and ftohex
SRCS=src/*.h src/*.c src/Makefile # src/TODO? src/HEADER? src/PATCHES?
# test files for dasm and ftohex
TSTS=test/*.asm test/*.bin.ref test/*.hex.ref test/Makefile test/run_tests.sh test/atari2600/Makefile test/atari2600/README test/atari2600/*.asm test/atari2600/*.ref
# other files
OTHS=Makefile

ifeq ($(strip $(BINARY)),)
# source release, no binaries
CONTENTS=$(DOCS) $(MACS) $(SRCS) $(TSTS) $(OTHS)
DIRNAME=dasm-$(RELEASE)
ZIPNAME=dasm-$(RELEASE)
else
# binary release for specific platform
CONTENTS=$(BINS) $(DOCS) $(MACS) $(SRCS) $(TSTS) $(OTHS)
DIRNAME=dasm-$(RELEASE)
ZIPNAME=dasm-$(RELEASE)-$(BINARY)
endif

# create a distribution archive for publication 
dist: build
	mkdir $(DIRNAME)
	cp -p -r --parents $(CONTENTS) $(DIRNAME)
	tar cvf - $(DIRNAME) | gzip -9 >$(ZIPNAME).tar.gz
#	tar cvf - $(DIRNAME) | bzip2 -9 >$(ZIPNAME).tar.bz2
	rm -rf $(DIRNAME)

# prepare a beta release containing source code and tests;
# machine files are included since tests may need them;
# nothing else is in the archive since it is not intended
# for the public, just designated volunteers
beta:
	echo "This is an incomplete beta release of the DASM assembler." >README.BETA
	echo "The purpose is to identify regressions, nothing more." >>README.BETA
	echo "Please do *not* re-distribute this release in any form!" >>README.BETA
	echo "Please do *not* distribute binaries derived from it either!" >>README.BETA
	-tar zcvf dasm-beta-`date +%F`.tar.gz README.BETA $(MACS) $(OTHS) $(SRCS) $(TSTS)
	rm -rf README.BETA

# remove beta archives and bin/ directory created by
# regular build from this Makefile; don't delete the
# "real" distribution archives
clean:
	(cd src; $(MAKE) clean; cd ..)
	(cd test; $(MAKE) clean; cd ..)
	-rm -rf dasm-beta-*.tar.gz bin/
