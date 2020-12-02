//libraries
#include <Servo.h>

//Declaring the I/O pins
#define speakerPin 5    //the pin for the piezo speaker(PMW)
#define ledRedPin 6     //RGB led pins (red)
#define ledGreenPin 7   //RGB led pins (green)
#define ledBluePin 8    //RGB led pins (blue)

#define flexPin A2   //Flux sensor pin (analog)

//The arm's mode {Manual mode, Learning mode, auto mode}
volatile boolean modeMan = true;    //this switch mode between manual and learning
boolean modeAuto = false;           //this switch mode between learning and auto

//delays
const int learningPause = 40;  //this is the amount of delay between each learning
const int autoPause = 10;      //this is the amount of delay between each auto perform

//learning configuration
const int numberOfServos = 4 ;                   //this is the number of servos used in the arm
const int numberOfRecordings = 250;              //the number of learn values for each motor.
//numberOfRecordings*learningPause= total miliseconds of learning length
int learnt[numberOfServos][numberOfRecordings];  //an array that stores the moves for all the motors in the given tries
const int learntDefault = 45;                    //default value for learnt

//servo motor configurations
Servo servos[4];     //[0]:base servo (horizental move), [1]:first node (vertical move), [2]:second node (verrical move), [3]:claw servo (horizental move)
int servoPins[] = {13, 12, 11, 10};

//inputs from the gyroscope sensor
struct gyroSensor {
  int x_position;
  int y_position;
  int z_posotion;
  int x_acceleration;
  int y_acceleration;
  int z_acceleration;
};
//creating a structure for the gyroscope sensor
gyroSensor gyroValues;

void setup() {
  //Starting serial communications
  Serial.begin(9600);
  Serial.println("Starting...");

  //setting the I/O pin mode
  pinMode(speakerPin,  OUTPUT);
  pinMode(ledRedPin,   OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin,  OUTPUT);

  //Attaching servo pin
  for (int i = 0; i < sizeof(servoPins); i++) {
    servos[i].attach(servoPins[i]);
  }

  //setting default values for learnt
  for (int i = 0; i < numberOfServos; i++) {
    for (int j = 0; j < numberOfRecordings; j++) {
      learnt[i][j] = learntDefault;
    }
  }
  attachInterrupt(0, ISR0, RISING); //attaching an interrupt for changing mode
  Serial.print("Ready!");
  speakerToneOne();                //indicating the start of program

}//end of void setup

void loop() {
  //------------------------------------------------------------------------
  //Manual Mode
  if (modeMan) {
    speakerToneThree(); //indicating change in mode
    rgbLed('r'); //color coding the mode, Manual red
    Serial.println("RGB LED color set to red");
    //setting the inputs

  }
    /* Learning not implemented yet 
  else {
    //----------------------------------------------------------------------
    //Automatic Mode
    if (modeAuto) {
      speakerToneThree(); //indicating change in mode
      rgbLed('b'); //color coding the mode, auto blue
      Serial.println("RGB LED color set to blue");
      //mimicing what was previously learnt
      for ( int counter = 0; counter < numberOfRecordings; counter++) {
        //setting the inputs
        delay(autoPause);
      }
    }
    //-----------------------------------------------------------------------
    //Learning Mode
    else {
      speakerToneThree(); //indicating change in mode
      rgbLed('g'); //color coding the mode, learning green
      Serial.println("RGB LED color set to green");
      //starting the learning
      for ( int counter = 0; counter < numberOfRecordings; counter++) {
        //getting the inputs
        delay(learningPause);
      }
      speakerToneTwo(); //speaker indicating the learning is over
      modeAuto = true;
      delay(3000);
    }
  }
  //Learning not implemented yet */
  delay(50);
}//end of void loop 


//intterupt Zero (ISR0)
void ISR0() {
  detachInterrupt(0); //turning off the interrupt
  if (modeMan) {
    modeMan = false;         //setting the mode from manual to learning
    Serial.println("Set mode to learning");
  } else {
    modeMan = true;       //set the mode from learning to manual
    modeAuto = false;     //set the mode auto off
    Serial.println("Set mode to manual");
  }
  delay(100);
  attachInterrupt(0, ISR0, RISING);
}

//flex sensor input method
int flexSensor() {
  int flexInput = analogRead(flexPin);
  return map(flexInput, 800, 1023, 0, 179);
}
//Gyroscope Sensor input method
void gyroSensor() {

}

//a method for the rgb led color settings
void rgbLed(char color) {
  digitalWrite(ledRedPin, LOW);
  digitalWrite(ledGreenPin, LOW);
  digitalWrite(ledBluePin, LOW);
  switch (color) {
    case 'r':
      digitalWrite(ledRedPin, HIGH);
      break;
    case 'g':
      digitalWrite(ledGreenPin, HIGH);
      break;
    case 'b':
      digitalWrite(ledBluePin, HIGH);
      break;
  }
}
//tone one ( ..-)
void speakerToneOne() {
  tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
  delay(150);
  tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
  delay(150);
  tone(speakerPin, 10000, 400); //400 milisec @ 10000Hz
  delay(100);
}
//tone two ( .---)
void speakerToneTwo() {
  tone(speakerPin, 15000, 200); //200 milisec @ 15000Hz
  delay(150);
  tone(speakerPin, 3000, 1500); //1.5 sec @ 3000Hz
  delay(100);
}
//tone three ( .. ..)
void speakerToneThree() {
  tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
  delay(100);
  tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
  delay(750);
  tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
  delay(100);
  tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
  delay(100);
}
