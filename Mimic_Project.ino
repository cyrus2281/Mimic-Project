
/* ============================================
 ***  Last Modified: 24/12/2020 ***
   
 ***  Milad Mobini (Github: Milad200281)  ***
   

  I2Cdev device library code is placed under the MIT license
  Copyright (c) 2012 Jeff Rowberg

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  ===============================================
*/


//libraries
#include <Servo.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"




//Declaring the I/O pins
#define speakerPin  13     //the pin for the piezo speaker(PMW)
#define ledRedPin   4     //RGB led pins (red)
#define ledGreenPin 5     //RGB led pins (green)
#define ledBluePin  6     //RGB led pins (blue)
#define interruptButton 3 //this button will change the mode

#define flexPin A0   //Flux sensor pin (analog)

int flexValue;

//The arm's mode {Manual mode, Learning mode, auto mode}
volatile boolean modeMan = true;    //this switch mode between manual and learning
boolean modeAuto = false;           //this switch mode between learning and auto

//delays
const int learningPause = 50;  //this is the amount of delay between each learning set
const int autoPause = 40;      //this is the amount of delay between each auto set
const int ManPause = 50;       //this is the amount of delay between each manual set
//learning configuration
const int numberOfServos = 5 ;                  //this is the number of servos used in the arm
const int numberOfRecordings = 70;              //the number of learn values for each motor.
//numberOfRecordings*learningPause= total miliseconds of learning length
int learnt[numberOfServos][numberOfRecordings];  //an array that stores the moves for all the motors in the given tries
const int learntDefault = 45;                    //default value for learnt

//servo motor configurations
Servo servos[5];     //[0]:base servo (horizental move), [1]:first node (vertical move), [2]:second node (verrical move),
//[3]: claw tilt (horizental move) [4]:claw servo (horizental move)
int servoPins[] = {11, 10, 9, 8, 7};
Servo servo;
const double ratioAngles = 0.67;        //the min/max angle ratio between the servos 2 and 3 (should be customized)
//variables to hold values
int yaw = 0;
int pitch = 0;
int roll = 0;

//The structure for the final outputs of gyroscope sensor
struct gyroSensor {
  int roll;
  int pitchOne;
  int pitchTwo;
  int yaw;

};
//creating a structure for the gyroscope sensor
gyroSensor gyroValues;

//gyroscope sensor needed variables

MPU6050 mpu;// class default I2C address is 0x68
#define OUTPUT_READABLE_YAWPITCHROLL
// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };



// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================
void setup() {
  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
  //Starting serial communications
  Serial.begin(115200);
  while (!Serial); // wait for Leonardo enumeration, others continue immediately
  Serial.println(" Starting...");

  //setting the I/O pin mode
  pinMode(speakerPin,  OUTPUT);
  pinMode(ledRedPin,   OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin,  OUTPUT);
  pinMode(interruptButton, INPUT_PULLUP); //pull will attach the internal resitance

  //color coding the mode, starting white
  rgbLed('w');
  //Attaching servo pin
  servo.attach(12);
  for (int i = 0; i < sizeof(servoPins); i++) {
    servos[i].attach(servoPins[i]);
  }

  //intitial values for sensor values
  gyroValues.yaw = 90;       //base servo
  gyroValues.pitchOne = 90;  //node one
  gyroValues.pitchTwo = 90;  //node two
  gyroValues.roll = 90;      //claw tilt
  flexValue = 90;            //claw

  //setting default values for learnt
  for (int i = 0; i < numberOfServos; i++) {
    for (int j = 0; j < numberOfRecordings; j++) {
      learnt[i][j] = learntDefault;
    }
  }
  attachInterrupt(1, ISR1, RISING); //attaching an interrupt for changing mode


  //Setting up the gyroscope sensor
// initialize device
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
   /*
  // wait for ready
  Serial.println(F("\nSend any character to begin DMP programming and demo: "));
  while (Serial.available() && Serial.read()); // empty buffer
  while (!Serial.available());                 // wait for data
  while (Serial.available() && Serial.read()); // empty buffer again
   */
  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
    attachInterrupt(0, dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR! 
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }


  Serial.println("Ready!");
  speakerToneOne();                //indicating the start of program

}//end of void setup

void loop() {
  //------------------------------------------------------------------------
  //Manual Mode
  if (modeMan) {
    Serial.print("****************\n* Manual Mode  *\n****************");
    speakerToneThree(); //indicating change in mode
    rgbLed('r'); //color coding the mode, Manual red
    Serial.println("RGB LED color set to red");
    //starting the loop
    while (modeMan) {
      //Reading from sensors
      gyroSensor();
      flexValue = flexSensor();
      //print the outputs
      printData();
      //writing the values into servos
      setServos();
      delay(ManPause);
    }
  }
  else {
    //----------------------------------------------------------------------
    //Automatic Mode
    if (modeAuto) {
      Serial.print("****************\n*  Auto Mode   *\n****************");
      speakerToneThree(); //indicating change in mode
      rgbLed('b'); //color coding the mode, auto blue
      Serial.println("RGB LED color set to blue");
      //mimicing what was previously learnt
      for ( int counter = 0; counter < numberOfRecordings; counter++) {
        //writing the learnt values to motors
        autoPrint(counter);
        //writing the learnt values to the motors
        autoSetServos(counter);
        delay(autoPause);
      }
      delay(1000);
    }
    //-----------------------------------------------------------------------
    //Learning Mode
    else {
      Serial.print("****************\n* Learning Mode*\n****************");
      speakerToneThree(); //indicating change in mode
      rgbLed('g'); //color coding the mode, learning green
      Serial.println("RGB LED color set to green");
      //starting the learning
      for (int counter = 0; counter < numberOfRecordings; counter++) {
        //Reading from sensors
        gyroSensor();
        flexValue = flexSensor();
        //print the outputs
        printData();
        //save tha data
        learnValues(counter);
        //writing the values into servos
        setServos();
        delay(learningPause);
      }
      speakerToneTwo(); //speaker indicating the learning is over
      modeAuto = true;
      delay(3000);
    }
  }
}//end of void loop


// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

//mpu int intrupt function
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}
//intterupt One (ISR1)
void ISR1() {
  detachInterrupt(1); //turning off the interrupt
  if (modeMan) {
    modeMan = false;         //setting the mode from manual to learning
    modeAuto = false;     //set the mode auto off
    Serial.println("\n@@@@@@@@@@@@@@@@\nSet mode to learning\n@@@@@@@@@@@@@@@@");
  } else {
    modeMan = true;       //set the mode from learning to manual
    modeAuto = false;     //set the mode auto off
    Serial.println("\n################\nSet mode to manual\n################");
  }
  delay(300);
  attachInterrupt(1, ISR1, RISING);
}

// ================================================================
// ===                    PRINT  METHODS                        ===
// ================================================================

//print all the outputs from the sensors
void printData() {
  // Print the unmapped values on the serial monitor
  Serial.print(roll);
  Serial.print("/");
  Serial.print(pitch);
  Serial.print("/");
  Serial.println(yaw);
  //printing the resuts
  Serial.print("roll = " );        Serial.print(gyroValues.roll);
  Serial.print(" | pitchOne = " ); Serial.print(gyroValues.pitchOne);
  Serial.print(" | pitchTwo = " ); Serial.print(gyroValues.pitchTwo);
  Serial.print(" | yaw = " );      Serial.print(gyroValues.yaw);
  Serial.print(" | flex = " );     Serial.print(flexValue);
  Serial.println("\n\n");
}
//this method will print values from learnt values
void autoPrint(int number) {
  Serial.print("\nLearnt Values: ");
  Serial.print("roll = " );        Serial.print(learnt[3][number]);
  Serial.print(" | pitchOne = " ); Serial.print(learnt[1][number]);
  Serial.print(" | pitchTwo = " ); Serial.print(learnt[2][number]);
  Serial.print(" | yaw = " );      Serial.print(learnt[0][number]);
  Serial.print(" | flex = " );     Serial.print(learnt[4][number]);
  Serial.println("\n");
}

// ================================================================
// ===                    SENSOR  METHODS                       ===
// ================================================================

//flex sensor input method
int flexSensor() {
  int flexInput = analogRead(flexPin);
  return map(flexInput, 850, 1023, 60, 90);
}
//Gyroscope Sensor input method
void gyroSensor() {


  if (!dmpReady) return;
  // wait for MPU interrupt or extra packet(s) available
  while (!mpuInterrupt && fifoCount < packetSize) {

  }

  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;

#ifdef OUTPUT_READABLE_YAWPITCHROLL
    // display Euler angles in degrees
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    Serial.print("ypr\t");
    Serial.print(ypr[0] * 180 / M_PI);
    Serial.print("\t");
    Serial.print(ypr[1] * 180 / M_PI);
    Serial.print("\t");
    Serial.println(ypr[2] * 180 / M_PI);
#endif
  }


  yaw = ypr[0] * 180 / M_PI;
  pitch = ypr[1] * 180 / M_PI;
  roll = ypr[2] * 180 / M_PI;
  //mapping the values for servo motors
  gyroValues.roll  =  map (roll,  -80, 80, 0, 180);
  gyroValues.yaw   =  map (yaw,   -180, 180, 0, 180);
 gyroValues.pitchOne =(180- map (pitch, -80, 80, 0, 180));
  /*
  if (pitch <= 40 && pitch >= -40) {
    gyroValues.pitchOne =(180- map (pitch, -40, 40, 0, 180));
  } else {
    int temp = pitch;
    if (temp > 40) {
      temp -= 40;
    } else {
      temp += 40;
    }
    //0 to 60 because the second angle doesn't need to open all the way
    int pitchTwoTemp = map (temp, -40, 40, 0, 60);
    gyroValues.pitchTwo = ratio(pitchTwoTemp, gyroValues.pitchOne);
  }

  */
  delay(50);
}

//this function will make sure of the ratio of angles between servo 2 and 3
int ratio(int inner, int outer) { //this function should be customized based on your structure
  return int((outer * ratioAngles) + inner);        //0.67 is the ratio of angles between the two motors
}

//this method will write the final output from the sensors to an array
void learnValues(int number) { //the input is the counter from the loop
  learnt[0][number] = gyroValues.yaw;       //base servo
  learnt[1][number] = gyroValues.pitchOne;  //node one
  learnt[2][number] = gyroValues.pitchTwo;  //node two
  learnt[3][number] = gyroValues.roll;      //claw tilt
  learnt[4][number] = flexValue;            //claw
}

// ================================================================
// ===                     MOTOR  METHODS                       ===
// ================================================================

//set the values from the sensors into the servos
void setServos() {
  servo.write(gyroValues.yaw);
  servos[5].write(gyroValues.yaw);       //base servo
  servos[1].write(gyroValues.pitchOne);  //node one
  servos[2].write(gyroValues.pitchTwo);  //node two
  servos[3].write(gyroValues.roll);      //claw tilt
  servos[4].write(flexValue);            //claw
}
//this method will write the learnt values to servo motors
void autoSetServos(int number) {
    servo.write(learnt[0][number]);
  servos[0].write(learnt[0][number]);       //base servo
  servos[1].write(learnt[1][number]);  //node one
  servos[2].write(learnt[2][number]);  //node two
  servos[3].write(learnt[3][number]);      //claw tilt
  servos[4].write(learnt[4][number]);            //claw
}

// ================================================================
// ===                    OTHER  METHODS                       ===
// ================================================================
 
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
    case 'w':
      digitalWrite(ledBluePin, HIGH);
      digitalWrite(ledGreenPin, HIGH);
      digitalWrite(ledRedPin, HIGH);
      break;
  }
  delay(50);
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
