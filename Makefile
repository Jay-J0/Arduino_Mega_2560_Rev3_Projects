MCU = atmega2560
F_CPU = 16000000UL
PORT = /dev/ttyACM0
BAUD = 115200

CC = avr-gcc
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall
OBJCOPY = avr-objcopy

TARGET = main

all: $(TARGET).hex

$(TARGET).elf: $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

upload: $(TARGET).hex
	@echo "Uploading to Arduino Mega 2560..."
	avrdude -v -c wiring -p m2560 -P $(PORT) -b $(BAUD) -D -U flash:w:$(TARGET).hex:i

clean:
	rm -f *.elf *.hex
