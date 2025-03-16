/**
  acts as SPI master. prints retreived SPI data to Serial
*/

#include <SPI.h>
#include <stdint.h>

#define BAUD 9600

void setup() {
  Serial.begin(BAUD);

  SPI.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0));
  SPI.begin();
}

void loop() {
  static uint8_t count = 0;
  Serial.println();
  Serial.print(count++);
  Serial.print("\t: ");
  uint8_t a,b;
  a = SPI.transfer(count);
  b = SPI.transfer(count);
  Serial.print(b, HEX);
  Serial.print(" ");
  Serial.print(a, HEX);

  delay(200);
}
