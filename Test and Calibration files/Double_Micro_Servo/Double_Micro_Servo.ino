/*
   With this file you can test the maximum and minimum range of the servo motors (two motor at a time)
*/

#include <Servo.h>

Servo myservoOne;  // create servo object to control a servo
Servo myservoTwo;  // create servo object to control a servo

int servoOnePin = 11;     // the first servo I/O pin
int servoTwoPin = 10;     // the second servo I/O pin

int firstStartDeg = 150;    // variable to store the starting position for the first servo
int firstMaxDeg = 170;     // variable to store the maximum position for the first servo
int firstMinDeg = 50;       // variable to store the minimum position for the first servo

int secondStartDeg = 20;    // variable to store the starting position for the second servo
int secondMaxDeg = 40;     // variable to store the maximum position for the second servo
int secondMinDeg = -20;      // variable to store the minimum position for the second servo

int firstPos = firstStartDeg;   // initialzing the loop variable for the first time for the first servo
int secondPos = secondStartDeg; // initialzing the loop variable for the first time for the second servo
const double rat = 0.67;        //the min/max angle ratio between the two servo motors

int shortDelay = 7;  // the dealy between the positions
int longDelay = 400; // the delay between the loops

void setup() {
  myservoOne.attach(servoOnePin);  // attaches the servo on pin 9 to the servo object
  myservoTwo.attach(servoTwoPin);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);        // to print the current position
}

void loop() {

  for ( firstPos ; firstPos <= firstMaxDeg; firstPos += 20) {    //goes to maximum
    myservoOne.write(firstPos);
    Serial.println("first pos: "+String(firstPos));
    delay(shortDelay);

    for ( secondPos ; secondPos <= secondMaxDeg; secondPos +=1) {    //goes to maximum
      // in steps of 1 degree
      myservoTwo.write(ratio(secondPos,firstPos));
      Serial.println("\t Second pos: "+String(ratio(secondPos,firstPos)));
      delay(shortDelay);
    }
    delay(longDelay);
    for ( secondPos; secondPos >= secondMinDeg; secondPos -= 1) { //goes to minimum
      myservoTwo.write(ratio(secondPos,firstPos));
      Serial.println("\t Second pos: "+String(ratio(secondPos,firstPos)));
      delay(shortDelay);
    }
    delay(longDelay);
  }
  
  delay(longDelay);
  
  for ( firstPos; firstPos >= firstMinDeg; firstPos -= 20) {    //goes to maximum
    myservoOne.write(firstPos);
    Serial.println("first pos: "+String(firstPos));
    delay(shortDelay);

    for ( secondPos ; secondPos <= secondMaxDeg; secondPos += 1) {    //goes to maximum
      // in steps of 1 degree
      myservoTwo.write(ratio(secondPos,firstPos));
      Serial.println("\t Second pos: "+String(ratio(secondPos,firstPos)));
      delay(shortDelay);
    }
    delay(longDelay);
    for ( secondPos; secondPos >= secondMinDeg; secondPos -= 1) { //goes to minimum
      myservoTwo.write(ratio(secondPos,firstPos));
      Serial.println("\t Second pos: "+String(ratio(secondPos,firstPos)));
      delay(shortDelay);
    }
    delay(longDelay);
  }
}

int ratio(int inner, int outer){ //this function should be customized based on your structure
  return int((outer*rat)+inner);            //0.67 is the ratio of angles between the two motors
}
