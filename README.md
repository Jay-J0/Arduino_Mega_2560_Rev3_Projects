# Arduino_Mega_2560_Rev3_Projects

Datasheets:   
Arduino: https://docs.arduino.cc/hardware/mega-2560/   
Atmel: https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf   

LINUX COMMANDS:   
**ls /dev/ | grep ACM** → Check which USB port connects to the board  
**avrdude -c wiring -p m2560 -P /dev/ttyACM0 -b 115200 -v** → Test connection with the board   
**make** → Compiles everything   
**make upload** → Flashes the .hex file to your Arduino   
**make clean** → Deletes .elf and .hex files to rebuild from scratch     
   
________________________________________________________________________________________________    

SCREEN FOR USART:    
**screen /dev/ttyACM0 115200** → Open a Serial Terminal with screen    
**screen -ls** → List screen sessions    
**screen -X -S <session_id> quit** → Quit session    
**screen -r <session_id>** → Reattach to session    
**Ctrl + D** → While in screen session, terminate session    
**Ctrl + A, then D** → This will detach the screen session — it's still running in the background.    
