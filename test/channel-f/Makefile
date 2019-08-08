test: lights.bin tetris.bin
	-diff lights.bin lights.bin.ref
	-diff tetris.bin tetris.bin.ref

lights.bin: lights.asm
	../../bin/dasm lights.asm -f3 -I../../machines/channel-f/ -olights.bin
tetris.bin: tetris.asm
	../../bin/dasm tetris.asm -f3 -I../../machines/channel-f/ -otetris.bin

clean:
	rm -rf lights.bin tetris.bin
