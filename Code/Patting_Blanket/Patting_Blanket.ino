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
unsigned servoDownPos = 70;  // number to write to servo for it to be down.
unsigned servoUpPos = 20;     // number to write to servo for it to be up.


int PatThresh = 90;  // when sensor is triggered
int PatHyst = 20;    // a sort of debouce
bool held = false;   // boolean based on weather a person if continuing to hold the sensor

int track = 1;


// ---------------
// Servo movement states
// ---------------


enum State {
  IDLE,
  PAT_DOWN,
  PAT_UP
};

State state = IDLE;
unsigned long lastChange = 0;
unsigned long lastChangeMusic = 0;
const unsigned long interval = 200;
const unsigned long track_length = 5000;


void setup() {

  Serial.begin(9600);
  softwareSerial.begin(9600);



  if (player.begin(softwareSerial)) {  // Start communication with DFPlayer Mini (this plays the MP3 tracks)

    Serial.println("OK");  // DFPlayer mini will print ok in the serial monitor if no issues

    // Set volume to maximum (0 to 30).
    player.volume(20);
  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }

  patterServo.attach(9);          // pin the servo is attached to.
  patterServo.write(servoUpPos);  // make sure servo starts in the starting up position.
}

void loop() {

  unsigned long now = millis();
  int sensorVal = analogRead(PinSensor);
  unsigned long now2 = millis();

  // ----------------
  // INPUT HANDLING
  // ----------

  bool triggerPat = false;  //Always set false in the loop so if it's set true at the next bit of logic it's only true for 1 frame.

  // Detect NEW trigger only if not already held
  if (!held && sensorVal > (PatThresh + PatHyst)) {
    held = true;
    triggerPat = true;

    Serial.println("TRIGGER!");
  }

  // Detect release
  if (held && sensorVal < (PatThresh - PatHyst)) {
    held = false;
    Serial.println("RELEASE HELD");
  }

  // -----------
  // Triggering MP3 when a Pat is sensed
  // --------------

  if ((triggerPat) && (now2 - lastChangeMusic > track_length)) {
    int track = random(1,4);  // picks a number from 1 to 4 - so it will play a ramdom track. 
    player.play(track);  //(track) is the interger we named before - currently there is only one, but can be more.
      // If pat is sensed (but its not just being held) and its not already playing then please play track.
lastChangeMusic = now2;
  }                  
  // ----------------------------------------
  // SERVO STATE MACHINE
  // ----------------------------------------

  switch (state) {
    case IDLE:
      patterServo.write(servoUpPos);

      if (triggerPat) {  // Move to PAT_DOWN state if pat is sensed (but not if someone is just holding sensor.)
        state = PAT_DOWN;
        lastChange = now;
      }
      break;

    case PAT_DOWN:
      patterServo.write(servoDownPos);  // in PAT_DOWN state, move servo down to pat

      if (now - lastChange >= interval) {  // wait for servo to get there and move to state PAT_UP
        state = PAT_UP;
        lastChange = now;
      }

      break;
    case PAT_UP:
      patterServo.write(servoUpPos);  // Move servo back to starting position (It resets the servo)


      if (now - lastChange >= interval) {  // wait for servo to get there
        state = IDLE;
        lastChange = now;
      }
      break;
  }

  // Debug output
  Serial.print("Sensor: ");
  Serial.print(sensorVal);

  Serial.print("  Held: ");
  Serial.print(held);

  Serial.print("  State: ");

  switch (state) {
    case IDLE:
      Serial.println("IDLE");
      break;

    case PAT_DOWN:
      Serial.println("PAT_DOWN");
      break;

    case PAT_UP:
      Serial.println("PAT_UP");
      break;
  }
}
