//libraries
#include <Servo.h>
#include <Wire.h>

//Declaring the I/O pins
#define speakerPin  3   //the pin for the piezo speaker(PMW)
#define ledRedPin   4   //RGB led pins (red)
#define ledGreenPin 7   //RGB led pins (green)
#define ledBluePin  8   //RGB led pins (blue)

#define flexPin A0   //Flux sensor pin (analog)

int flexValue;

//The arm's mode {Manual mode, Learning mode, auto mode}
volatile boolean modeMan = !true;    //this switch mode between manual and learning
boolean modeAuto = false;           //this switch mode between learning and auto

//delays
const int learningPause = 50;  //this is the amount of delay between each learning set
const int autoPause = 30;      //this is the amount of delay between each auto set
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
int servoPins[] = {11, 10, 9, 6, 5};

//The structure for the final outputs of gyroscope sensor
struct gyroSensor {
  float roll;
  float pitchOne;
  float pitchTwo;
  float yaw;

};
//creating a structure for the gyroscope sensor
gyroSensor gyroValues;

//gyroscope sensor needed variables
const int MPU = 0x68;                                           // MPU6050 I2C address
float AccX, AccY, AccZ;                                         //to record the accelaration values from sensor
float GyroX, GyroY, GyroZ;                                      //to record the gyroscope values from sensor
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ; //to record the calculated angles from the sensor
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ; //to record the IMU errors
float elapsedTime, currentTime, previousTime;                   //to record times between happenings
float yaw, roll, pitch;                                         //the final outputs before mapping for servos
int errorCounter = 0;                                           //a variable that holds the number of test runs



void setup() {
  //Starting serial communications
  Serial.begin(19200);
  Serial.println(" Starting...");

  //setting the I/O pin mode
  pinMode(speakerPin,  OUTPUT);
  pinMode(ledRedPin,   OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin,  OUTPUT);

  //Attaching servo pin
  for (int i = 0; i < sizeof(servoPins); i++) {
    servos[i].attach(servoPins[i]);
  }

  //Setting up the gyroscope sensor
  Wire.begin();                      // Initialize comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission
  //this function get the IMU error values for module
  calculate_IMU_error();
  delay(20);

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
  //attachInterrupt(0, ISR0, RISING); //attaching an interrupt for changing mode
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
  Serial.print("\nLearnt Values");
  Serial.print("roll = " );        Serial.print(learnt[3][number]);
  Serial.print(" | pitchOne = " ); Serial.print(learnt[1][number]);
  Serial.print(" | pitchTwo = " ); Serial.print(learnt[2][number]);
  Serial.print(" | yaw = " );      Serial.print(learnt[0][number]);
  Serial.print(" | flex = " );     Serial.print(learnt[4][number]);
  Serial.println("\n");
}

//flex sensor input method
int flexSensor() {
  int flexInput = analogRead(flexPin);
  return map(flexInput, 800, 1023, 0, 179);
}
//Gyroscope Sensor input method
void gyroSensor() {
  //Read acceleromter data
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

  //Read gyroscope data
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
  // Correcting the outputs with the calculated error values
  GyroX = GyroX - GyroErrorX; // GyroErrorX
  GyroY = GyroY - GyroErrorY; // GyroErrorY
  GyroZ = GyroZ - GyroErrorX; // GyroErrorZ

  // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  gyroAngleX = gyroAngleX + GyroX * elapsedTime; // deg/s * s = deg
  gyroAngleY = gyroAngleY + GyroY * elapsedTime;
  yaw =  yaw + GyroZ * elapsedTime;

  // Complementary filter - combine acceleromter and gyro angle values
  roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

  //mapping the values for servo motors
  gyroValues.roll  =  map (roll,  -100, 100, 0, 180);
  gyroValues.yaw   =  map (yaw,   -100, 100, 0, 180);
  if (pitch <= 50 && pitch >= -50) {
    gyroValues.pitchOne = map (pitch, -50, 50, 0, 180);
  } else {
    int temp = pitch;
    if (temp > 50) {
      temp -= 50;
    } else {
      temp += 50;
    }
    gyroValues.pitchTwo = map (temp, -50, 50, 0, 180);
  }
  delay(50);
}

//this method will write the final output from the sensors to an array
void learnValues(int number) { //the input is the counter from the loop
  learnt[0][number] = gyroValues.yaw;       //base servo
  learnt[1][number] = gyroValues.pitchOne;  //node one
  learnt[2][number] = gyroValues.pitchTwo;  //node two
  learnt[3][number] = gyroValues.roll;      //claw tilt
  learnt[4][number] = flexValue;            //claw
}

//set the values from the sensors into the servos
void setServos() {
  servos[0].write(gyroValues.yaw);       //base servo
  servos[1].write(gyroValues.pitchOne);  //node one
  servos[2].write(gyroValues.pitchTwo);  //node two
  servos[3].write(gyroValues.roll);      //claw tilt
  servos[4].write(flexValue);            //claw
}
//this method will write the learnt values to servo motors
void autoSetServos(int number) {
  servos[0].write(learnt[0][number]);       //base servo
  servos[1].write(learnt[1][number]);  //node one
  servos[2].write(learnt[2][number]);  //node two
  servos[3].write(learnt[3][number]);      //claw tilt
  servos[4].write(learnt[4][number]);            //claw
}
//this method will calculate the error in the readings from the gyroscope sensor.
void calculate_IMU_error() {
  // We can call this funtion in the setup section to calculate the accelerometer and gyro data error.
  // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values.

  // Read accelerometer values 300 times
  while (errorCounter < 300) {
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
    errorCounter++;
  }
  //Divide the sum by 300 to get the error value
  AccErrorX = AccErrorX / 300;
  AccErrorY = AccErrorY / 300;
  errorCounter = 0;
  // Read gyro values 300 times
  while (errorCounter < 300) {
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
    errorCounter++;
  }
  //Divide the sum by 200 to get the error value
  GyroErrorX = GyroErrorX / 300;
  GyroErrorY = GyroErrorY / 300;
  GyroErrorZ = GyroErrorZ / 300;
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
