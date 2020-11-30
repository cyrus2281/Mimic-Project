//Declaring the I/O pins
#define speakerPin 5 //the pin for the piezo speaker(PMW)


//The arm's mode {Manual mode, Learning mode => auto mode}
volatile boolean mode = false;

void setup() {
  Serial.begin(9600); //Starting serial communications
  Serial.println("Starting...");
  attachInterrupt(0, ISR0, RISING); //attaching an interrupt for changing mode
}

void loop() {
//Manual Mode




//Learning Mode



//Automatic Mode





}




//intterupt Zero (ISR0)
void ISR0() {
  detachInterrupt(0); //turning off the interrupt
  if(!mode){
    mode=true;           //setting the mode from manual to learning
    Serial.println("Set mode to learning");
  }else{
    mode= false;        //set the mode from learning to manual
    Serial.println("Set mode to manual");  
  }
  delay(100);
  attachInterrupt(0, ISR0, RISING);
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
