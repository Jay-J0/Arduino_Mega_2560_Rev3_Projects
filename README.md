# Arduino_Mega_2560_Rev3_Projects

Datasheets:   
Arduino: https://docs.arduino.cc/hardware/mega-2560/   
Atmel: https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf   

Linux commands:   
**ls /dev/ | grep ACM** → Check which USB port connects to the board  
**avrdude -c wiring -p m2560 -P /dev/ttyACM0 -b 115200 -v** → Test connection with the board   
**make** → Compiles everything   
**make upload** → Flashes the .hex file to your Arduino   
**make clean** → Deletes .elf and .hex files to rebuild from scratch      
**screen /dev/ttyACM0 9600** → Open a Serial Terminal with screen
