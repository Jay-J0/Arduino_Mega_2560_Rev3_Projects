# Define the microcontroller model — this tells the compiler which AVR chip to target
MCU = atmega2560

# Define the CPU frequency — this is used for things like delay calculations
F_CPU = 16000000UL

# The serial port your Arduino is connected to (check with ls /dev/tty*)
PORT = /dev/ttyACM0

# Baud rate used for communication with the bootloader
BAUD = 115200

# Compiler to use for compiling AVR C code
CC = avr-gcc

# Compiler flags:
# -mmcu: target microcontroller
# -DF_CPU: set the clock frequency
# -Os: optimize for size
# -Wall: enable all warnings
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall

# avr-objcopy is used to convert compiled ELF files to Intel HEX format for flashing
OBJCOPY = avr-objcopy

# Base name of your main source file (without extension)
TARGET = main

# Default target to build when you run just `make`
# It builds the .hex file, which is the final format for uploading
all: $(TARGET).hex

# Rule to build an .elf (Executable and Linkable Format) file from your C source
# $< = first prerequisite (main.c)
# $@ = target (main.elf)
$(TARGET).elf: $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $<

# Rule to convert the .elf file to a .hex file
# -O ihex: output format = Intel HEX
# -R .eeprom: remove EEPROM data from the output
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Upload rule: sends the .hex file to the Arduino Mega 2560 using avrdude
upload: $(TARGET).hex
	@echo "Uploading to Arduino Mega 2560..."
	avrdude -v -c wiring -p m2560 -P $(PORT) -b $(BAUD) -D -U flash:w:$(TARGET).hex:i

# Clean rule: removes all build artifacts (helps when you want to start fresh)
clean:
	rm -f *.elf *.hex
