#
# include for Makefile
#
CPU_VARIANT = hc908jk3

# without hc08sprg bootloader
NATIVE_RAM_START = 0x80
NATIVE_RAM_SIZE = 128

# with bootloader
RAM_START = 0x88
RAM_SIZE = 120

STACK_LOC = 0xFF
# STACKSIZE = 40

FLASH_START = 0xEC00
FLASH_SIZE = 4096

MONI_START = 0xFC00
MONI_SIZE = 1024

# Monitor Mode
# MHZ = 2
# user mode
MHZ = 1

MEM_MODEL = --model-small