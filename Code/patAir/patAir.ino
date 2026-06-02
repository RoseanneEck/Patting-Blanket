// this code will pat if you pat. If it senses another pat while the servo is patting 
// it will pat again - should be faster patting! 


#include <Servo.h>

const byte PinSensor = A0;

Servo patterServo;

unsigned servoDownPos = 270;
unsigned servoUpPos   = 30;

int PatThresh = 90;
int PatHyst   = 20;

bool held = false;

// Prevents rapid retriggering (independent of held state and servo state)
const unsigned long lockoutTime = 200;
unsigned long lastTriggerTime = 0;

// Servo movement timing
const unsigned long movementTime = 200;
unsigned long currentMoveTime = movementTime;

enum State {
  IDLE,
  PAT_DOWN,
  PAT_UP
};

State state = IDLE;

unsigned long lastChange = 0;

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

  // Sensor release detection
  if (held && sensorVal < (PatThresh - PatHyst)) {
    held = false;
    Serial.println("RELEASE");
  }

//Set lockoutExpired if time has triggerd
  bool lockoutExpired = (now - lastTriggerTime) >= lockoutTime;

  // Detect NEW trigger only if not already held and lockoutExpired
  if (!held &&
      lockoutExpired &&
      sensorVal > (PatThresh + PatHyst)) {

    held = true;
    triggerPat = true;

    lastTriggerTime = now;

    Serial.println("TRIGGER");
  }

  // ----------------------------------------
  // SERVO STATE MACHINE
  // ----------------------------------------

  switch (state) {

    case IDLE:

      patterServo.write(servoUpPos);

      if (triggerPat) {

//Regular trigger from idle uses full movement time to go down
        currentMoveTime = movementTime;

        state = PAT_DOWN;
        lastChange = now;
      }

      break;

    case PAT_DOWN:

      patterServo.write(servoDownPos);

      // Ignore retriggers during downward motion

      if (now - lastChange >= currentMoveTime) {

        state = PAT_UP;
        lastChange = now;

        // Reset upward movement duration
        currentMoveTime = movementTime;
      }

      break;

    case PAT_UP:

      patterServo.write(servoUpPos);

      // Retrigger during PAT_UP
      if (triggerPat) {

        // How long had we already been moving upward?
        unsigned long elapsedUpTime = now - lastChange;

        // IF WE RETRIGGER during PAT_UP new downward move needs less time
        currentMoveTime = min(elapsedUpTime, movementTime);

        state = PAT_DOWN;
        lastChange = now;

        Serial.print("RETRIGGER DOWN TIME: ");
        Serial.println(currentMoveTime);
      }

      else if (now - lastChange >= currentMoveTime) {

        state = IDLE;
        lastChange = now;
      }

      break;
  }

  // ----------------------------------------
  // DEBUG
  // ----------------------------------------

  Serial.print("Sensor: ");
  Serial.print(sensorVal);

  Serial.print(" Held: ");
  Serial.print(held);

  Serial.print(" State: ");

  switch (state) {

    case IDLE:
      Serial.print("IDLE");
      break;

    case PAT_DOWN:
      Serial.print("PAT_DOWN");
      break;

    case PAT_UP:
      Serial.print("PAT_UP");
      break;
  }

  Serial.print(" MoveTime: ");
  Serial.println(currentMoveTime);
}