#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"


static const uint8_t PIN_MP3_TX = 8; // Connects to module's RX (DFPlayer Mini)
static const uint8_t PIN_MP3_RX = 10; // Connects to module's TX
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

const int pot = A0;
int potValue = 0;

// Create the DFRobot Player mini object
DFRobotDFPlayerMini player;

void setup() {

  pinMode(pot, INPUT);

  // Init USB serial port for debugging
  Serial.begin(9600);
  // Init serial port for DFPlayer Mini
  softwareSerial.begin(9600);

  // Start communication with DFPlayer Mini
  if (player.begin(softwareSerial)) {
    Serial.println("OK");

    // Set volume to maximum (0 to 30).
    player.volume(30);
  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }      
}

void loop() {

	potValue = analogRead(pot);

	if(potValue > 100 ){ 

	 static unsigned long timer = millis();
 	 if (millis() - timer > 2000) { //2000 is the duration of the audio(1)
  		timer = millis();

   		//(2) is the 2rd file in the sd card, the order = the order you copied the file to it
   		player.play(1);  
  }
  
	}else {
  	  static unsigned long timer = millis();
  
 	 if (millis() - timer > 3000) { //3000 is the duration of the audio(2)
  	  	timer = millis();
   		player.play(1); 
  		}
	}
}