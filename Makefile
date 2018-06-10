#Change this variable to point to your Arduino device
#Mac - it may be different
DEVICE = /dev/tty.usbmodem1421

#Linux (/dev/ttyACM0 or possibly /dev/ttyUSB0)
#DEVICE = /dev/ttyACM0 

#Windows
#DEVICE = COM3 

#target for part 1

program_5: os.c serial.c synchro.c program5.c SdReader.c dbuffer.c ext2reader.c ext2.c os_util.c
	avr-gcc -mmcu=atmega2560 -DF_CPU=16000000 -O2 -o project5.elf os.c serial.c synchro.c program5.c SdReader.c dbuffer.c ext2reader.c ext2.c os_util.c
	avr-objcopy -O ihex project5.elf main.hex
	avr-size project5.elf

#flash the Arduino with the program
program: main.hex
	#Mac
	avrdude -D -pm2560 -P $(DEVICE) -c wiring -F -u -U flash:w:main.hex

#remove build files
clean:
	rm -fr *.elf *.hex *.o
