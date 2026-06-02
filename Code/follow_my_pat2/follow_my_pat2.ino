
#include <Servo.h>

// listens to human patting speed and copies it. 

const byte PinSensor = A0;

Servo patterServo;

unsigned servoDownPos = 270;
unsigned servoUpPos   = 30;

int PatThresh = 90;
int PatHyst   = 30;

bool held = false;

// ----------------------------------------
// TEMPO SETTINGS
// ----------------------------------------

// Fastest allowed tempo (acts like trigger lockout)
const unsigned long maximumTempo = 200; // BPM

// Slowest allowed tempo
const unsigned long minimumTempo = 50; // BPM

// Continue autoplay after user stops touching
const unsigned long playOutTime = 5000;

// Number of taps used to calculate tempo
const int tapCountRequired = 4;

// ----------------------------------------
// SERVO MOVEMENT SETTINGS
// ----------------------------------------

const unsigned long movementTime = 250;
const unsigned long turnAroundTime = 50; 
unsigned long currentMoveTime = movementTime;

// ----------------------------------------
// STATE MACHINE
// ----------------------------------------

enum State {
  IDLE,
  PAT_DOWN,
  PAT_UP
};

State state = IDLE;

unsigned long lastChange = 0;

// ----------------------------------------
// TAP TEMPO VARIABLES
// ----------------------------------------

unsigned long tapTimes[tapCountRequired];
int tapIndex = 0;
bool tempoReady = false;

unsigned long beatInterval = 1000;
unsigned long lastBeatTime = 0;

unsigned long lastTouchTime = 0;

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

  bool triggerPat = false;

  // RELEASE SENSOR DETECTION
  if (held && sensorVal < (PatThresh - PatHyst)) {

    held = false;

    Serial.println("RELEASE");
  }

  // NEW TOUCH SENSOR DETECTION
  if (!held && sensorVal > (PatThresh + PatHyst)) {

    held = true;

    Serial.println("TOUCH");

    // ----------------------------------------
    // TAP TEMPO RECORDING
    // ----------------------------------------

    tapTimes[tapIndex] = now;

    tapIndex++;

    if (tapIndex >= tapCountRequired) {

      // Shift all taps left by one
      for (int i = 0; i < tapCountRequired - 1; i++) {
        tapTimes[i] = tapTimes[i + 1];
      }

      tapIndex = tapCountRequired - 1;
    }

    // Once we have enough taps calculate BPM
    if (tapIndex >= tapCountRequired - 1) {

      unsigned long totalInterval = 0;

      for (int i = 0; i < tapCountRequired - 2; i++) {

        totalInterval += tapTimes[i + 1] - tapTimes[i];
      }

      unsigned long averageInterval = totalInterval / (tapCountRequired - 2);

      // Convert BPM limits into interval limits
      unsigned long minInterval = 60000UL / maximumTempo;
      unsigned long maxInterval = 60000UL / minimumTempo;

      // Clamp interval between max and min tempo limits
      beatInterval = constrain(averageInterval, minInterval, maxInterval);

      tempoReady = true;

      Serial.print("BPM: ");
      Serial.println(60000UL / beatInterval);
    }

    // Immediate response while user is tapping
    triggerPat = true;

    lastTouchTime = now;
  }

  // ----------------------------------------
  // AUTO PLAYOUT
  // ----------------------------------------

  bool playOutActive = (now - lastTouchTime) < playOutTime;

  if (tempoReady && playOutActive) {

    if (now - lastBeatTime >= beatInterval) {

      triggerPat = true;
      lastBeatTime = now;

      Serial.println("AUTO PAT");
    }
  }

  // ----------------------------------------
  // SERVO STATE MACHINE
  // ----------------------------------------

  switch (state) {

    case IDLE:

      patterServo.write(servoUpPos);

      if (triggerPat) {

        currentMoveTime = movementTime;

        state = PAT_DOWN;
        lastChange = now;
      }

      break;

    case PAT_DOWN:

      patterServo.write(servoDownPos);

      // Ignore retriggers during downward movement

      if (now - lastChange >= currentMoveTime) {

        state = PAT_UP;
        lastChange = now;

        currentMoveTime = movementTime;
      }

      break;

    case PAT_UP:

      patterServo.write(servoUpPos);

      // Retrigger during upward movement
      if (triggerPat) {

        unsigned long elapsedUpTime = now - lastChange;

		// turnAroundTime is added to cope with inertia (still not more than orginal movement time)
        currentMoveTime = min(elapsedUpTime + turnAroundTime, movementTime);

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

  Serial.print(" TempoReady: ");
  Serial.print(tempoReady);

  Serial.print(" BPM: ");

  if (beatInterval > 0) {
    Serial.print(60000UL / beatInterval);
  }
  else {
    Serial.print(0);
  }

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

  Serial.println();
}