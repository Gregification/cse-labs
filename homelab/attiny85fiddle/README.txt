inspired by "bradform hamilton"s work in a article for The Medium. 
https://medium.com/@bradford_hamilton/bare-metal-programming-attiny85-22be36f4e9ca

thought of the time: 
"ive got a uv misquito zapper lamp but my cat, huah-huah, keeps staring into it so i cant leave it on meaning the misquitos are eating me :( "

DATASHEET
    https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf

REFRENCES
    avr-lib : https://github.com/avrdudes/avr-libc/
    winAVR user manual : if you downloaded the winAVR version there is a usermanual html file that is where ever winAVR is installed

THINGS TO DOWNLOAD
    avrdude : https://github.com/avrdudes/avrdude
    avr-gcc and avr-binutils : https://www.nongnu.org/avr-libc/user-manual/overview.html

COMPILING & FLASHING
    1. Compile for attiny85 - optimize for size
        avr-gcc -Wall -Os -mmcu=attiny85 main.c

    1.1 (optional) Get the assembly
        avr-gcc -S -Os -mmcu=attiny85 main.c -o assembly.asm

    2. Dump hex from out file
        $ hexdump a.out
        > certutil -encodehex a.out a.hex

    3. avrdude gui
        https://github.com/ZakKemble/AVRDUDESS
