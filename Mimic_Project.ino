//libraries
#include <Servo.h>
#include <Wire.h>

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

//The structure for the final outputs of gyroscope sensor
struct gyroSensor {
  float roll;
  float pitch;
  float yaw;

};
//creating a structure for the gyroscope sensor
gyroSensor gyroValues;

//gyroscope sensor needed variables
const int MPU = 0x68; // MPU6050 I2C address
float AccX, AccY, AccZ;  //to record the accelaration values from sensor
float GyroX, GyroY, GyroZ;  //to record the gyroscope values from sensor
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ; //to record the calculated angles from the sensor
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;  //to record the IMU errors
float elapsedTime, currentTime, previousTime;  //to record times between happenings
int c = 0; //a variable that holds the number of test runs



//----------------------------------------------------------------------




void setup() {
  //Starting serial communications
  Serial.begin(19200);
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
  //----------------------------------------------------------------------
  Serial.begin(19200);
  Wire.begin();                      // Initialize comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission
  // Call this function if you need to get the IMU error values for your module
  calculate_IMU_error();
  delay(20);


  //----------------------------------------------------------------------

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

    gyroSensor();

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
 // === Read acceleromter data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  //For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
  // Calculating Roll and Pitch from the accelerometer data
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - 0.58; // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) + 1.58; // AccErrorY ~(-1.58)

  // === Read gyroscope data === //
  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; // Divide by 1000 to get seconds
  Wire.beginTransmission(MPU);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 4 registers total, each axis value is stored in 2 registers
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  // Correct the outputs with the calculated error values
  GyroX = GyroX + 2.58; // GyroErrorX ~(-0.56)
  GyroY = GyroY - 0.64; // GyroErrorY ~(2)
  GyroZ = GyroZ + 0.21; // GyroErrorZ ~ (-0.8)

  // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  gyroAngleX = gyroAngleX + GyroX * elapsedTime; // deg/s * s = deg
  gyroAngleY = gyroAngleY + GyroY * elapsedTime;
  gyroValues.yaw =  gyroValues.yaw + GyroZ * elapsedTime;

  // Complementary filter - combine acceleromter and gyro angle values
  gyroValues.roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  gyroValues.pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

  // Print the values on the serial monitor
  Serial.print(gyroValues.roll);
  Serial.print("/");
  Serial.print(gyroValues.pitch);
  Serial.print("/");
  Serial.println(gyroValues.yaw);

  Serial.print("roll = " ); Serial.print( map (gyroValues.roll, -100, 100, 0, 180));
  Serial.print(" | pitch = " ); Serial.print( map (gyroValues.pitch, -100, 100, 0, 180));
  Serial.print(" | yaw = " ); Serial.print( map (gyroValues.yaw, -100, 100, 0, 180));
  Serial.println("\n=======================================================\n");
  delay(100);
}

//set the values from the sensors into the servos
void setServo() {
 
}

//this method will calculate the error in the readings from the gyroscope sensor.
//The outputs from this function should be change by the error in the gyroSensor() method
void calculate_IMU_error() {
  // We can call this funtion in the setup section to calculate the accelerometer and gyro data error. From here we will get the error values used in the above equations printed on the Serial Monitor.
  // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
  // Read accelerometer values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    c++;
  }
  //Divide the sum by 200 to get the error value
  AccErrorX = AccErrorX / 200;
  AccErrorY = AccErrorY / 200;
  c = 0;
  // Read gyro values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    c++;
  }
  //Divide the sum by 200 to get the error value
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
  GyroErrorZ = GyroErrorZ / 200;
  // Print the error values on the Serial Monitor
  Serial.print("AccErrorX: ");
  Serial.println(AccErrorX);
  Serial.print("AccErrorY: ");
  Serial.println(AccErrorY);
  Serial.print("GyroErrorX: ");
  Serial.println(GyroErrorX);
  Serial.print("GyroErrorY: ");
  Serial.println(GyroErrorY);
  Serial.print("GyroErrorZ: ");
  Serial.println(GyroErrorZ);
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
