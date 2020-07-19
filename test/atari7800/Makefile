
test: spritesample.asm
	@../run_tests.sh -R ../../bin -F 3 -I ../../machines/atari7800/

spritesample.bin: spritesample.asm
	../../bin/dasm spritesample.asm -f3 -I../../machines/atari7800/ -ospritesample.bin

clean:
	rm -rf spritesample.bin *.diff *.hex *.list.txt *.out
