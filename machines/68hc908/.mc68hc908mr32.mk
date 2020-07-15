#
# include for Makefile
#
CPU_VARIANT = hc908mr32

# true size
NATIVE_RAM_START = 0x60
NATIVE_RAM_SIZE = 768

# with bootloader
RAM_START = 0x68
RAM_SIZE = 760
STACK_LOC = 0x035E

# NATIVE_RAM_SIZE + NATIVE_RAM_START -1

FLASH_START = 0x8000
NATIVE_FLASH_SIZE = 32256
FLASH_SIZE = 31744

BOOT_LOADER_SIZE = 512

MONI_START = 0xFE10
MONI_SIZE = 240

MHZ = 1
# MHZ = 1.22875 # exact

# MEM_MODEL = --stack-auto --stack-probe --int-long-rent --xram-loc 0x100 --xram-size 576
MEM_MODEL = --model-large --stack-auto --idata-loc 0x68 

