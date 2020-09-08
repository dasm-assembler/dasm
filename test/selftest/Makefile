#
#  here are the tests for testing the test-script
#

DASM=../../bin/dasm
FTOHEX=../../bin/ftohex

test: 
	@../run_tests.sh -R ../../bin -S 7

%.bin: %.asm
	$(DASM) $< -o$@ -f1 

%.hex: %.bin
	$(FTOHEX) 1 $< $@

clean:
	rm -rf *.bin *.hex *.list.txt *.diff *.out
