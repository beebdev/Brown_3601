/*  Button codes : ON = A90, Volume up = 490, volume down = C90 */
//#include <IRremote.h>                                           // infrared library
int RECV_PIN = 2;                                               // connect TSOP data pin to Arduino pin 2
int LED_pin = 5;

int read_val = 0;
int line_count = 0;

void setup()
{
  Serial.begin(9600);                                           // start serial monitor at a baud rate of 9600
  pinMode(RECV_PIN, INPUT);
  pinMode(LED_pin, OUTPUT);
}

void loop() {

  read_val = digitalRead(RECV_PIN);
  Serial.print(read_val);
  line_count++;
  if (line_count == 50) {
    line_count = 0;
    Serial.print('\n');
  }
  
  if (digitalRead(RECV_PIN) == LOW) {
      digitalWrite(LED_pin, HIGH);
  } else {
      digitalWrite(LED_pin, LOW);
  }
//  delay(20);
}
