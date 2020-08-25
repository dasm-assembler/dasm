
test: lights.asm tetris.asm
	@../run_tests.sh -R ../../bin -F 3 -I ../../machines/channel-f/

lights.bin: lights.asm
	../../bin/dasm lights.asm -f3 -I../../machines/channel-f/ -olights.bin
tetris.bin: tetris.asm
	../../bin/dasm tetris.asm -f3 -I../../machines/channel-f/ -otetris.bin

clean:
	rm -rf lights.bin tetris.bin *.diff *.hex *.list.txt *.out
