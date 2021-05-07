#include <Wire.h>
#include <VarSpeedServo.h>

//shock pins
#define LXP A0
#define LYP A1
#define LSP 3
#define RXP A2
#define RYP A3
#define RSP 4
//servo pins
#define S1P 5
#define S2P 6
#define S3P 7
#define S4P 8
#define S5P 9
//shock varialbes
int LX = 0;
int LY = 0;
int RX = 0;
int RY = 0;
//servo pins array
VarSpeedServo servos[5];
//servo values array
int servoValues[5];
//servo Speed
int servoSpeed = 60;
//reset servo value
int resetServo[5];


//threash holds
int increaseThree = 852;
int increaseTwo = 682;
int increaseOne = 550;
int middle = 512;
int decreaseOne = 470;
int decreaseTwo = 305;
int decreaseThree = 130;
//change values
int changeValueOne = 2;
int changeValueTwo = 5;
int changeValueThree = 10;
//pauses
int eachRoundPause = 150;
int shortPause = 50;
int longPause = 250;

void setup()
{
  //starting serial communication
  Serial.begin(9600);
  Serial.println(" Starting...");
  //attaching servo pins
  servos[0].attach(S1P);
  servos[1].attach(S2P);
  servos[2].attach(S3P);
  servos[3].attach(S4P);
  servos[4].attach(S5P);
  //rest values
  resetServo[0] = 90;
  resetServo[1] = 160;
  resetServo[2] = 12  0;
  resetServo[3] = 90 ;
  resetServo[4] = 170 ;
  //giving intitial values
  resetValues();
  //buttons/switches
  pinMode(LSP, INPUT_PULLUP);
  pinMode(RSP, INPUT_PULLUP);
  digitalWrite(LSP, HIGH);
  digitalWrite(RSP, HIGH);

  Serial.println("Ready!");
}

void loop()
{
  //print data
  printData();
  //getting and setting values
  LX = analogRead(LXP);
  LY = analogRead(LYP);
  RX = analogRead(RXP);
  RY = analogRead(RYP);

  readShockToServoValue(LX, 2);
  readShockToServoValue(LY, 3);
  readShockToServoValue(RX, 1);
  readShockToServoValue(RY, 0);

  if (digitalRead(LSP) == 0 ) {
    toggleHook(4);
  }
  if (digitalRead(RSP) == 0 ) {
    resetValues();
  }
  //writting to servos
  writeServos();
  delay(eachRoundPause);
}

/**
   Write servoValues to servos
*/
void writeServos() {
  for (int i = 0; i < 5; i++)
  {
    servos[i].write(servoValues[i], servoSpeed);
  }
}
/**
   reset all the servo values to 90
*/
void resetValues() {
  for (int i = 0; i < 5; i++)
  {
    servoValues[i] = resetServo[i];
  }
}
/**
  Toggle hook switch value
*/
boolean toggleHook(int number)
{
  if (servoValues[number] == 170)
  {
    servoValues[number] = 30;
  }
  else
  {
    servoValues[number] = 170;
  }
  return true;
}
/**
   get the read value from shock pin and pass it through conditions and write result to servoValue number

*/
boolean readShockToServoValue(int shockValue, int number)
{
  if (shockValue >= increaseThree && servoValues[number] < (180 - changeValueOne))
  {
    servoValues[number] += changeValueThree;
  }
  else if (shockValue >= increaseTwo && servoValues[number] < (180 - changeValueTwo))
  {
    servoValues[number] += changeValueTwo;
  }
  else if (shockValue >= increaseOne && servoValues[number] < (180 - changeValueThree))
  {
    servoValues[number] += changeValueOne;
  }
  else if (shockValue <= decreaseThree && servoValues[number] > changeValueThree)
  {
    servoValues[number] -= changeValueThree;
  } else if (shockValue <= decreaseTwo && servoValues[number] > changeValueTwo)
  {
    servoValues[number] -= changeValueTwo;
  }
  else if (shockValue <= decreaseOne && servoValues[number] > changeValueOne)
  {
    servoValues[number] -= changeValueOne;
  }

  return true;
}

void printData()
{
  Serial.print("LX");
  Serial.print(LX);
  Serial.print("  LY:");
  Serial.print(LY);
  Serial.print("  RX:");
  Serial.print(RX);
  Serial.print("  RY:");
  Serial.print(RY);
  Serial.print("  S1:");
  Serial.print(servoValues[0]);
  Serial.print("  S2:");
  Serial.print(servoValues[1]);
  Serial.print("  S3:");
  Serial.print(servoValues[2]);
  Serial.print("  S4:");
  Serial.print(servoValues[3]);
  Serial.print("  S5:");
  Serial.println(servoValues[4]);
}
