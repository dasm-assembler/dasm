#
# Makefile for testing hexadecimal notation on command line args
#

DASM=../../bin/dasm
FTOHEX=../../bin/ftohex

test: 
	@../run_tests.sh -R ../../bin -D RAM_START=0x60

%.bin: %.asm
	$(DASM) $< -o$@ -f1 -T1 -s$*.sym -DRAM_START=0x60

%.hex: %.bin
	$(FTOHEX) 1 $< $@

clean:
	rm -rf *.bin *.hex *.list.txt *.diff *.sym *.out
