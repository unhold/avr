F_CPU = 16500000
MCU = attiny45
CSOURCES = usbdrv/usbdrv.c usbdrv/oddebug.c main.c
ASSOURCES = usbdrv/usbdrvasm.S
CFLAGS = -Os -I.
FBOOT = fboot -d /dev/ttyUSB0

include ../../../lib/Makefile.avr.inc
