test: boing26.bin
	-diff boing26.bin boing26.bin.ref

boing26.bin: boing26.asm
	../../bin/dasm boing26.asm -f3 -I../../machines/atari2600/ -oboing26.bin

clean:
	rm -rf boing26.bin
