NAME ?= main

# default flags
CFLAGS += -Wall -g -mmcu=$(MCU) -DF_CPU=$(F_CPU)
ASFLAGS += $(CFLAGS)
LDFLAGS += -Wl

# switch the compiler (for the internal make rules)
CC = avr-gcc
AS = avr-gcc
LD = avr-ld

OBJECTS = $(CSOURCES:.c=.o) $(ASSOURCES:.S=.o)
GENFILES = $(NAME).ppc $(NAME).elf $(NAME).lst $(NAME).hex $(NAME).d $(OBJECTS)

# first target
.PHONY: all
all: $(NAME).ppc $(NAME).elf $(NAME).lst $(NAME).d $(NAME).hex

# rules for files

# preprocessor output
$(NAME).ppc: $(CSOURCES) 
	$(CC) $(CFLAGS) -E $(CSOURCES) > $(NAME).ppc

$(NAME).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

$(NAME).hex: $(NAME).elf
	avr-objcopy -O ihex $^ $@

$(NAME).lst: $(NAME).elf
	avr-objdump -dSt $^ > $@
	avr-size $^

# remove all generated files
.PHONY: clean
clean:
	-rm -f $(GENFILES)

# flash programming

AVRDUDE ?= avrdude

AD_FLASH = -U flash:w:$(NAME).hex:i
.PHONY: flash
flash: $(NAME).hex
	$(AVRDUDE) $(AD_FLASH)

.PHONY: verify
verify: $(NAME).hex
	$(AVRDUDE) -U flash:v:$(NAME).hex:i

.PHONY: read
read: $(NAME).hex
	$(AVRDUDE) -U flash:r:read.hex:i

AD_FUSE = -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m
.PHONY: fuse
fuse:
	$(AVRDUDE) $(AD_FUSE)

AD_VFUSE = -U hfuse:v:$(HFUSE):m -U lfuse:v:$(LFUSE):m
.PHONY: vfuse
vfuse:
	$(AVRDUDE) $(AD_VFUSE)

AD_RFUSE = -U hfuse:r:-:h -U lfuse:r:-:h 
.PHONY: rfuse
rfuse:
	$(AVRDUDE) $(AD_RFUSE) 2> /dev/null

AD_LOCK = -U lock:w:$(LOCK):m
.PHONY : lock
lock:
	$(AVRDUDE) $(AD_LOCK)

.PHONY : write
write: 
	$(AVRDUDE) $(AD_FLASH) $(AD_FUSE) $(AD_LOCK)

# fboot bootloader

FBOOT ?= fboot

.PHONY: fbootp
fbootp: all
	$(FBOOT) -p $(NAME).hex

.PHONY: fbootv
fbootv: all
	$(FBOOT) -v $(NAME).hex

# generate project dependencies

$(NAME).d: $(CSOURCES)
	$(CC) -MM $(CFLAGS) $(CSOURCES) > $(NAME).d
ifdef ASSOURCES
	$(CC) -MM $(ASFLAGS) $(ASSOURCES) >> $(NAME).d
endif

# include project dependencies
include $(NAME).d
