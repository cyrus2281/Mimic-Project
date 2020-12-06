# Mimic-Project
A mechanical arm programed using Arduino which records the movements of a hand and mimics it.

Abstract:

The mechanical arm reads the inputs from the gyroscope sensor and the flex sensor located on the user's hand. The gyroscope returns the angular changes, and the flex sensor returns the amount of bending of the finger. Then these values are written to servo motors used to make the arm. The arm uses a RGB LED and piezo speaker to help the user understand what is happening. 

The arm has three modes: 
1. Manual Mode: Mimics the movements of the hand gestures at the real-time.
2. Learning Mode: Manual+Records the results of the sensors to a data set.
3. Auto Mode: This mode will mimics the values learnt (pre-setted) from the data set.



Requirments:

Arduino Mega/Uno

GY-521 6-axis gyroscope/accelometer sensor

Micro Servo motors

Flex Sensor

Bushbutton

Piezo Speaker

RGB LED

Breadboard

Jumper Wires