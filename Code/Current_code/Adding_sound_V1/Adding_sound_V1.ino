#include <Servo.h>  // include servo libary
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// Useing 3 states to have One pat = pat of servo. Good But slow.

static const uint8_t PIN_MP3_TX = 8;   // Connects to module's RX (DFPlayer Mini)
static const uint8_t PIN_MP3_RX = 10;  // Connects to module's TX
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Create the DFRobot Player mini object
DFRobotDFPlayerMini player;

const byte PinSensor = A0;

Servo patterServo;
unsigned servoDown = 270;  // number to write to servo for it to be down.
unsigned servoUp = 30;     // number to write to servo for it to be up.


int PatThresh = 90;  // when sensor is triggered
int PatHyst = 20;    // a sort of debouce
bool held;
//int track = random(1,5);  // picks a number from 1 to 5 (for when I get more tracks)
int track = 1;

enum State {
  IDLE,
  PATTED,
  RESET
};

State state = IDLE;
unsigned long lastChange = 0;
unsigned long lastChangeMusic = 0;
const unsigned long interval = 200;
const unsigned long track_length = 3000;


void setup() {

  Serial.begin(9600);
  // Init serial port for DFPlayer Mini
  softwareSerial.begin(9600);
  // delay(1000);

  // Start communication with DFPlayer Mini
  if (player.begin(softwareSerial)) {

    Serial.println("OK");

    // Set volume to maximum (0 to 30).
    player.volume(20);
  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }

  patterServo.attach(9);
  held = false;
}

void loop() {

  unsigned long now = millis();
  int sensorVal = analogRead(PinSensor);
  unsigned long now2 = millis();

  switch (state) {
    case IDLE:
      patterServo.write(servoUp);
      Serial.println("  IDLE");
      Serial.println(sensorVal);
      if (!held && ((PatThresh + PatHyst) < sensorVal)) {  // Move to patted state if pat is sensed (but not if someone is just holding sensor.)
        lastChange = now;
        lastChangeMusic = now2;
        state = PATTED;
      } else if (held && (PatThresh - PatHyst) > sensorVal) {  // this switches it to being un held if someone was just holding the sensor
        held = false;
        Serial.println("   release");
        Serial.println(sensorVal);
        state = IDLE;
      }
      break;

    case PATTED:
      patterServo.write(servoDown);  // in Patted state, move servo down to pat
      Serial.println("  PATTED");
      Serial.println(sensorVal);
      held = true;

      if (now2 - lastChangeMusic > track_length) {  //track_length is the duration of the audio(1)
        lastChangeMusic = now2;

        //(track) is the interger we named before - currently there is only one, but can be more.
        player.play(track);
      }


      if (now - lastChange >= interval) {  // wait for servo to get there and move to state reset
        state = RESET;
        lastChange = now;
      }

      break;
    case RESET:
      patterServo.write(servoUp);  // Move servo back to starting position
      Serial.println("  RESET");
      Serial.println(sensorVal);
      if (now - lastChange >= interval) {  // wait for servo to get there
        state = IDLE;
        lastChange = now;
      }
      break;
  }
}
