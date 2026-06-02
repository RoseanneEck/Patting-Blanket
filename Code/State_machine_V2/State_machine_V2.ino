#include <Servo.h>  // include servo libary

// Useing 3 states to have One pat = pat of servo. Good But slow. 

const byte PinSensor = A0;

Servo patterServo;
unsigned servoDown = 270;  // number to write to servo for it to be down.
unsigned servoUp = 30;     // number to write to servo for it to be up.


int PatThresh = 90;  // when sensor is triggered
int PatHyst = 20;     // a sort of debouce
bool held;

enum State {
  IDLE,
  PATTED,
  RESET
};

State state = IDLE;
unsigned long lastChange = 0;
const unsigned long interval = 200;


void setup() {

  Serial.begin(9600);
  patterServo.attach(9);
  held = false;
}

void loop() {

  unsigned long now = millis();
  int sensorVal = analogRead(PinSensor);

  switch (state) {
    case IDLE:
      patterServo.write(servoUp);
      Serial.println("  IDLE");
      Serial.println(sensorVal);
      if (!held && ((PatThresh + PatHyst) < sensorVal)) {  // Move to patted state if pat is sensed (but not if someone is just holding sensor.) 
        lastChange = now; 
        state = PATTED;

      }

      else if (held && (PatThresh - PatHyst) > sensorVal) {  // this switches it to being un held if someone was just holding the sensor
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
      if (now - lastChange >= interval) { // wait for servo to get there and move to state reset
        state = RESET;
        lastChange = now;
      }
      break;
    case RESET:
      patterServo.write(servoUp); // Move servo back to starting position
      Serial.println("  RESET");
      Serial.println(sensorVal);
      if (now - lastChange >= interval) { // wait for servo to get there
        state = IDLE;
        lastChange = now;
      }
      break;
  }
}
