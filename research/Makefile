CFLAGS=-O0 -std=c99 -Wall -Wextra # try -O3 for fun...

ALL = hash_compare compare_mod_and minunit_example get_set_progname sizeof bitsets loweror fsize #args

all: $(ALL)

clean:
	rm -rf *.o $(ALL)


MNEMONICS := $(wildcard ../src/mne*.c) ../src/globals.c

all_mnemonics.txt: $(MNEMONICS)
	cat $(MNEMONICS) | grep 'NULL,[ ]*v_' | cut -f3 -d, | sed 's/^[ ]*//g;s/"//g' | sort -u >all_mnemonics.txt
