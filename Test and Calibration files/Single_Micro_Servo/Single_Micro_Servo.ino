/*
   With this file you can test the maximum and minimum range of the servo motors (one motor at a time)
*/

#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int servoPin = 11;     // the servo I/O pin
int startDeg = 70;    // variable to store the starting position
int maxDeg = 100;     // variable to store the maximum position
int minDeg = 40;       // variable to store the minimum position

int shortDelay = 20;  // the dealy between the positions
int longDelay = 2000; // the delay between the loops

int pos = startDeg;   // initialzing the loop variable for the first time
void setup() {
  myservo.attach(servoPin);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);        // to print the current position
}

void loop() {
  for ( pos ; pos <= maxDeg; pos += 1) {    //goes to maximum
    // in steps of 1 degree
    myservo.write(pos);
    Serial.println(pos);
    delay(shortDelay);
  }
  delay(longDelay);
  for ( pos; pos >= minDeg; pos -= 1) { //goes to minimum
    myservo.write(pos);
    Serial.println(pos);
    delay(shortDelay);
  }
  delay(longDelay);
}
