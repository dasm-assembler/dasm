test: spritesample.bin
	-diff spritesample.bin spritesample.bin.ref

spritesample.bin: spritesample.asm
	../../bin/dasm spritesample.asm -f3 -I../../machines/atari7800/ -ospritesample.bin

clean:
	rm -rf spritesample.bin
