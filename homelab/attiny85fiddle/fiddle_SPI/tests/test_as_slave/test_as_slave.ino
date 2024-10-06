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
  Serial.print((uint8_t)SPI.transfer(count));

  delay(200);
}
