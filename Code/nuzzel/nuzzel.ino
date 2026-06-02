#include <Servo.h>

// If you pat it pats. If a pat is sensed before the pat is completed it 
// sends it back for another pat. 

const byte PinSensor = A0;

Servo patterServo;

unsigned servoDownPos = 270;   // servo position for pat down
unsigned servoUpPos   = 30;    // servo position for pat up

int PatThresh = 90;
int PatHyst   = 20;

bool held = false;

// ----------------------------
// Servo movement states
// ----------------------------
enum State {
  IDLE,
  PAT_DOWN,
  PAT_UP
};

State state = IDLE;

unsigned long lastChange = 0;
const unsigned long interval = 200; //Movement time (per phase)

void setup() {

  Serial.begin(9600);

  patterServo.attach(9);
  patterServo.write(servoUpPos);
}

void loop() {

  unsigned long now = millis();
  int sensorVal = analogRead(PinSensor);

  // ----------------------------------------
  // INPUT HANDLING
  // ----------------------------------------

  bool triggerPat = false; //Always set false in the loop so if it's set true at the next bit of logic it's only true for 1 frame.

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

  // ----------------------------------------
  // SERVO STATE MACHINE
  // ----------------------------------------

  switch (state) {

    case IDLE:

      patterServo.write(servoUpPos);

      // Accept trigger in IDLE
      if (triggerPat) {
        state = PAT_DOWN;
        lastChange = now;
      }

      break;

    case PAT_DOWN:

      patterServo.write(servoDownPos);

      // Ignore triggers while moving down, so no code for reacting on triggerPat here
 
 // When it reaches the
      if (now - lastChange >= interval) {
        state = PAT_UP;
        lastChange = now;
      }

      break;

    case PAT_UP:

      patterServo.write(servoUpPos);

      // During PAT_UP:
      // a new trigger immediately restarts PAT_DOWN
      if (triggerPat) {
        state = PAT_DOWN;
        lastChange = now;
      }

//ALSO if it gets to the start it returns to idle
      else if (now - lastChange >= interval) {
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