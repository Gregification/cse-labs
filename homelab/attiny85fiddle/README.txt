inspired by "bradform hamilton"s work in a article for The Medium. 
https://medium.com/@bradford_hamilton/bare-metal-programming-attiny85-22be36f4e9ca

thought of the time: 
DATASHEET
    https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf

REFRENCES
    avr-lib : https://github.com/avrdudes/avr-libc/
    winAVR user manual : if you downloaded the winAVR version there is a usermanual html file that is where ever winAVR is installed

THINGS TO DOWNLOAD
    avrdude : https://github.com/avrdudes/avrdude
    avr-gcc and avr-binutils : https://www.nongnu.org/avr-libc/user-manual/overview.html

COMPILING & FLASHING
    1. Compile for attiny85 - optimize for size , dont forget inclueds
        avr-gcc --std=c99 -Wall -Os -mmcu=attiny85 main.c

    2. make Intel hex from the output
        avr-objcopy -O ihex -j .text -j .data a.out a.hex

    3. avrdude will flash the ihex
        https://github.com/ZakKemble/AVRDUDESS

DUMP HEX
    $ hexdump a.out
    > certutil -encodehex a.out a.hex

GET assembly
    avr-gcc -S -Os -mmcu=attiny85 main.c -o a.asm
    