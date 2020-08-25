
test: boing26.asm
	@../run_tests.sh -R ../../bin -F 3 -I ../../machines/atari2600/

boing26.bin: boing26.asm
	../../bin/dasm boing26.asm -f3 -I../../machines/atari2600/ -oboing26.bin

clean:
	rm -rf boing26.bin *.diff *.hex *.list.txt *.out
