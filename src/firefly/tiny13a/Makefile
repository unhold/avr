F_CPU = 128000
MCU = attiny13a
CSOURCES = main.c
CFLAGS = -Os
#AVRDUDE = avrdude -p t13 -x devcode=0x33
AVRDUDE = avrdude -p t13 -i 100
FBOOT = fboot -b 600 -d /dev/ttyUSB0

HFUSE = 0xef
LFUSE = 0x7b
LOCK  = 0x3c

include ../../../lib/Makefile.avr.inc

