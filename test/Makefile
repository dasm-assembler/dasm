# demo.asm doesn't compile!
# no useful reference output for suitef8.asm right now

DASM=../bin/dasm
FTOHEX=../bin/ftohex

test:
	(cd atari2600; $(MAKE) test; cd ..)
	(cd atari7800; $(MAKE) test; cd ..)
	(cd cmdline_defs; $(MAKE) test; cd ..)
	./run_tests.sh

%.bin: %.asm
	$(DASM) $< -o$@ -f1 -DINEEPROM

%.hex: %.bin
	$(FTOHEX) 1 $< $@

clean:
	(cd atari2600; $(MAKE) clean; cd ..)
	(cd atari7800; $(MAKE) clean; cd ..)
	rm -rf *.bin *.hex *.list.txt
