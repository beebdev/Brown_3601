#include <SPI.h>

#define HS 9

void setup() {
  Serial.begin(115200);
  pinMode(HS, INPUT);
  digitalWrite(SS, HIGH); // disable Slave Select
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV4);//divide the clock by 4
}
 
void loop() {
//  if (digitalRead(HS) == HIGH) {
    char c[129];
    memset(c, 0, 128);
    sprintf(c, "Elton Shih");
    Serial.print("Sending data to slave...");
    digitalWrite(SS, LOW); // enable Slave Select
    for (int i = 0; i < 128; i++) {
      SPI.transfer(c[i]);
    }
    digitalWrite(SS, HIGH); // disable Slave Select
    Serial.print("OK\n");
//  }
  delay(2000);
}
