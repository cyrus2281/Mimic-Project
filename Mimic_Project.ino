//Declaring the I/O pins
#define speakerPin 5 //the pin for the piezo speaker(PMW)
#define ledRedPin 6     //RGB led pins (red)
#define ledGreenPin 7   //RGB led pins (green)
#define ledBluePin 8   //RGB led pins (blue)



//The arm's mode {Manual mode, Learning mode, auto mode}
volatile boolean modeMan = true;    //this switch mode between manual and learning
boolean modeAuto = false;         //this switch mode between learning and auto






void setup() {
  Serial.begin(9600); //Starting serial communications
  Serial.println("Starting...");
  attachInterrupt(0, ISR0, RISING); //attaching an interrupt for changing mode
  speakerToneOne(); //indicating the start of program

}

void loop() {
  //Manual Mode
  if (modeMan) {
    speakerToneThree(); //indicating change in mode
    rgbLed('r'); //color coding the mode, Manual red


  }
  else {
    //Automatic Mode
    if (modeAuto) {
      speakerToneThree(); //indicating change in mode
      rgbLed('b'); //color coding the mode, auto blue

    }
    //Learning Mode
    else {
      speakerToneThree(); //indicating change in mode
      rgbLed('g'); //color coding the mode, learning green

      speakerToneTwo(); //speaker indicating the learning is over
      modeAuto = true;
    }
  }








}




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
//a method for the rgb led color settings
void rgbLed(char color) {
  digitalWrite(ledRedPin, LOW);
  digitalWrite(ledGreenPin, LOW);
  digitalWrite(ledBluePin, LOW);
  switch (color) {
    case 'r':
      Serial.println("RGB LED color set to red");
      digitalWrite(ledRedPin, HIGH);
      break;
    case 'g':
      Serial.println("RGB LED color set to green");
      digitalWrite(ledGreenPin, HIGH);
      break;
    case 'b':
      Serial.println("RGB LED color set to blue");
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
