//Declaring the I/O pins
#define speakerPin 5 //the pin for the piezo speaker(PMW)



void setup() {

}

void loop() {

}

//tone one ( ..-)
void speakerToneOne(){
tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
delay(150);
tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
delay(150);
tone(speakerPin, 10000, 400); //400 milisec @ 10000Hz
delay(100);  
}
//tone two ( .---)
void speakerToneTwo(){
tone(speakerPin, 15000, 200); //200 milisec @ 15000Hz
delay(150);
tone(speakerPin, 3000, 1500); //1.5 sec @ 3000Hz
delay(100);  
}
//tone three ( .. ..)
void speakerToneOne(){
tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
delay(100);
tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
delay(750);
tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
delay(100);
tone(speakerPin, 5000, 200); //200 milisec @ 5000Hz
delay(100); 
}
