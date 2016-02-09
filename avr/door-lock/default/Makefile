###############################################################################
# Makefile for the project nock_avr
###############################################################################

## General Flags
PROJECT = nock_avr
MCU = atmega8
TARGET = nock_avr.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=nock_avr.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = buttons.o eepromworker.o leds.o main.o my_ext_irq.o nock_machine.o softtimer.o uart_logger.o colibrator.o send_machine.o rf73_spi.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) nock_avr.hex nock_avr.eep nock_avr.lss size

## Compile
buttons.o: ../buttons.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

eepromworker.o: ../eepromworker.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

leds.o: ../leds.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

main.o: ../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

my_ext_irq.o: ../my_ext_irq.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

nock_machine.o: ../nock_machine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

softtimer.o: ../softtimer.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uart_logger.o: ../uart_logger.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

colibrator.o: ../colibrator.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

send_machine.o: ../send_machine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

rf73_spi.o: ../rf73_spi.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<


##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) nock_avr.elf dep/* nock_avr.hex nock_avr.eep nock_avr.lss nock_avr.map


## Other dependencies
-include $(shell mkdir dep 2>NUL) $(wildcard dep/*)

