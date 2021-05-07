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

//threash holds
int increaseThree = 900;
int increaseTwo = 600;
int increaseOne = 300;
int middle = 100;
int decreaseOne = -400;
int decreaseTwo = -600;
int decreaseThree = -900;

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
    servos[1].attach(S1P);
    servos[2].attach(S2P);
    servos[3].attach(S3P);
    servos[4].attach(S4P);
    servos[5].attach(S5P);
    //giving intitial values
    for (int i = 0; i < 5; i++)
    {
        servoValues[i] = 90;
    }

    Serial.println("Ready!");
}

void loop()
{
    //print data
    printData();
    //getting and setting values
    readShockToServoValue(analogRead(LXP), servoValues[0]);
    readShockToServoValue(analogRead(LYP), servoValues[1]);
    readShockToServoValue(analogRead(RXP), servoValues[2]);
    readShockToServoValue(analogRead(RYP), servoValues[3]);
    // readShockToServoValue(analogRead(LXP),servoValues[4]);
    //writting to servos
    writeServos();
    delay(eachRoundPause);
}

/**
 * Write servoValues to servos
 */
void writeServos(){
    for (int i = 0; i < 5; i++)
    {
        servos[i].write(servoValues[i], servoSpeed);
    }
}
    /**
 * Toggle hook switch value
 */
    boolean toggleHook(int shockValue, int servoValue)
{
    if (servoValue == 170)
    {
        servoValue = 30;
    }
    else
    {
        servoValue = 170;
    }
    return true;
}
/**
 * get the read value from shock pin and pass it through conditions and write result to servoValue
 * 
 */
boolean readShockToServoValue(int shockValue, int servoValue)
{
    if (shockValue >= increaseThree && servoValue < 177)
    {
        servoValue += 3;
    }
    else if (shockValue >= increaseTwo && servoValue < 178)
    {
        servoValue += 2;
    }
    else if (shockValue >= increaseOne && servoValue < 179)
    {
        servoValue += 1;
    }
    else if (shockValue >= middle)
    {
        //nothing
    }
    else if (shockValue >= decreaseThree && servoValue > 3)
    {
        servoValue -= 3;
    }
    else if (shockValue >= decreaseTwo && servoValue > 2)
    {
        servoValue -= 2;
    }
    else if (shockValue >= decreaseOne && servoValue > 1)
    {
        servoValue -= 1;
    }
    else
    {
        //nothing
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
