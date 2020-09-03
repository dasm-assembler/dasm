# demo.asm doesn't compile!
# no useful reference output for suitef8.asm right now

DASM=../bin/dasm
FTOHEX=../bin/ftohex

test:
	./run_tests.sh -S 7 | tee test-report.txt

%.bin: %.asm
	$(DASM) $< -o$@ -f1 -DINEEPROM

%.hex: %.bin
	$(FTOHEX) 1 $< $@

clean:
	$(MAKE) -C atari2600 clean
	$(MAKE) -C atari7800 clean
	$(MAKE) -C cmdline_defs clean
	$(MAKE) -C selftest clean
	$(MAKE) -C channel-f clean
	rm -rf *.bin *.hex *.list.txt *.diff *.out
