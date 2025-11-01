// *** Clapper Stepper motor control using RV8824 Driver and LDR ***
/*==========================================================================**
 What This Does
 Uses a Finite State Machine so behavior is structured and easy to extend.
 Uses AccelStepper for smooth accel/decel motion.
 
 Daylight only → waits interval, then moves half a turn in ~25 s.
 At night → goes into STATE_SLEEP, resets for next day.
 Manual override → typing GO in Serial Monitor forces an immediate half-turn.
 Max torque mode →
 full-step drive (MS1–3 LOW)
 low speed & accel in code
 disables motor between moves for power saving.
**============================================================================**
 Key Settings
  setMaxSpeed(100) → sets maximum speed (steps/sec). 
  setAcceleration(75) → sets acceleration (steps/sec²). Adjust for smoothness vs. torque.
  For new gear ratio 200 steps/rev, halfTurnSteps = stepsPerRevolution * 6.5 
  With current values, 0.5 rev finishes in about 25 s, smoothly ramped.
 ✅ Features:
   • 0.5 rev forward every 30 minutes (default) in daylight.
   • Serial "GO" = immediate half-turn.
   • "SET FREQ <minutes>" to change interval.
   • "SAVE FREQ" to store new interval in EEPROM (1–60 min range).
   • LED confirms EEPROM ops (1=load, 2=default, 3=save).
**=============================================================================*/

#include <AccelStepper.h>
#include <EEPROM.h>

// --- Pin Definitions ---
const int STEP_PIN = 9;
const int DIR_PIN  = 8;
const int EN_PIN   = 5;
const int LDR_PIN  = A0;
const int LED_PIN  = 13;

// Microstepping Pins
const int MS1 = 2;
const int MS2 = 3;
const int MS3 = 4;

// Stepper Setup
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
const int stepsPerRevolution = 200;
const int halfTurnSteps = stepsPerRevolution * 6.5;

// FSM States
enum State { STATE_IDLE, STATE_WAIT_INTERVAL, STATE_MOVE, STATE_SLEEP, STATE_OVERRIDE };
State currentState = STATE_IDLE;

// Timing
unsigned long lastMoveTime = 0;
unsigned long moveInterval = 1800000UL; // default 30 min (ms)

// Threshold + Hysteresis
const int daylightThreshold = 500;
const int hysteresis = 10;
bool lastDayState = false;

// EEPROM
const int EEPROM_SIGNATURE = 0xA5;
const int ADDR_SIGNATURE = 0;
const int ADDR_INTERVAL  = 1;

// LED Feedback
void flashLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    delay(delayMs);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH); // motor off
  pinMode(LED_PIN, OUTPUT);
  pinMode(MS1, OUTPUT); pinMode(MS2, OUTPUT); pinMode(MS3, OUTPUT);
  digitalWrite(MS1, LOW); digitalWrite(MS2, LOW); digitalWrite(MS3, LOW);
  stepper.setMaxSpeed(100);
  stepper.setAcceleration(50);
  stepper.setPinsInverted(true, false, false);

  // Load interval from EEPROM
  if (EEPROM.read(ADDR_SIGNATURE) == EEPROM_SIGNATURE) {
    unsigned int storedMins = EEPROM.read(ADDR_INTERVAL);
    if (storedMins >= 1 && storedMins <= 60) {
      moveInterval = (unsigned long)storedMins * 60000UL;
      Serial.print(F("Loaded interval from EEPROM: ")); Serial.print(storedMins); Serial.println(F(" min"));
      flashLED(1, 200);
    } else {
      flashLED(2, 200);
    }
  } else {
    Serial.println(F("No saved interval, using default 30 min."));
    flashLED(2, 200);
  }

  // File Info Header
  Serial.println(F("*======================================*"));
  Serial.print(F("* File: ")); Serial.println(F(__FILE__));
  Serial.print(F("* Compiled: ")); Serial.print(F(__DATE__));
  Serial.print(F(" Time: "));  Serial.println(F(__TIME__));
  Serial.println(F("* Type 'GO', 'SET FREQ <m>', 'SAVE FREQ' *"));
  Serial.println(F("*======================================*"));
}

void loop() {
  int ldrValue = analogRead(LDR_PIN);
  bool isDay = (ldrValue > daylightThreshold);
  if (ldrValue > daylightThreshold + hysteresis) isDay = true;
  else if (ldrValue < daylightThreshold - hysteresis) isDay = false;
  lastDayState = isDay;

  // Serial Commands (case-insensitive)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim(); cmd.toUpperCase();

    if (cmd == "GO") {
      currentState = STATE_OVERRIDE;
    } else if (cmd.startsWith("SET FREQ")) {
      int newFreqMin = cmd.substring(8).toInt();
      if (newFreqMin >= 1 && newFreqMin <= 60) {
        moveInterval = (unsigned long)newFreqMin * 60000UL;
        Serial.print(F("Interval set to ")); Serial.print(newFreqMin); Serial.println(F(" min."));
      } else {
        Serial.println(F("Invalid: use 1–60 min."));
      }
    } else if (cmd == "SAVE FREQ") {
      unsigned int saveMins = moveInterval / 60000UL;
      EEPROM.write(ADDR_SIGNATURE, EEPROM_SIGNATURE);
      EEPROM.write(ADDR_INTERVAL, saveMins);
      Serial.print(F("Interval saved: ")); Serial.print(saveMins); Serial.println(F(" min"));
      flashLED(3, 150);
    }
  }

  // FSM
  switch (currentState) {
    case STATE_IDLE:
      if (isDay) { lastMoveTime = millis(); currentState = STATE_WAIT_INTERVAL; }
      break;
    case STATE_WAIT_INTERVAL:
      if (!isDay) currentState = STATE_SLEEP;
      else if (millis() - lastMoveTime >= moveInterval) {
        digitalWrite(EN_PIN, LOW); stepper.move(halfTurnSteps); currentState = STATE_MOVE;
      }
      break;
    case STATE_MOVE:
      if (stepper.distanceToGo() != 0) stepper.run();
      else { digitalWrite(EN_PIN, HIGH); lastMoveTime = millis(); currentState = STATE_WAIT_INTERVAL; }
      break;
    case STATE_SLEEP:
      if (isDay) { lastMoveTime = millis(); currentState = STATE_WAIT_INTERVAL; }
      break;
    case STATE_OVERRIDE:
      digitalWrite(EN_PIN, LOW); stepper.move(halfTurnSteps); currentState = STATE_MOVE;
      break;
  }
}
