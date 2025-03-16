/**
  baselined from code posted here by "GolamMostafa"
    https://forum.arduino.cc/t/spi-slave-mode-example-code/66617/5

  acts as SPI slave. prints retreived SPI data to Serial

  - this code does not handle SS, pleaes enable it though other means (jumper wire, etc)
*/

#include <SPI.h>
#include <stdint.h>

#define BAUD 9600

volatile int i = 0;
uint8_t buffer;

void setup() {
  Serial.begin(BAUD);

  //pinMode(SS, INPUT_PULLUP);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, INPUT);
  SPCR |= _BV(SPE);
  
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  SPI.attachInterrupt();
}

void loop() {
  if(i != 0){
    i = 0;
    Serial.print(i);
    Serial.print("\t: ");
    Serial.println((int)buffer);
  }

  delay(200);
}

ISR (SPI_STC_vect) {
  i++;
  buffer = SPDR;
}
