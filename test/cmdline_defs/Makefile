#
# Makefile for testing hexadecimal notation on command line args
#

DASM=../../bin/dasm
FTOHEX=../../bin/ftohex

test: ramstart.bin

%.bin: %.asm
	$(DASM) $< -o$@ -f1 -DRAM_START=0x60

%.hex: %.bin
	$(FTOHEX) 1 $< $@

clean:
	rm -rf *.bin *.hex *.list.txt
