// Squirrel Guard
// Teensy 4.1 + HuskyLens 2 + 2x MG996R pan/tilt + 12V diaphragm pump
// Libraries: Servo (built in), HUSKYLENS (DFRobot).
// Confirm the HUSKYLENS API, baud rate and frame size against your HuskyLens 2 library version.

#include <Servo.h>
#include "HUSKYLENS.h"

// ---------- Pins ----------
const uint8_t PIN_PAN   = 2;   // pan servo signal
const uint8_t PIN_TILT  = 3;   // tilt servo signal
const uint8_t PIN_PUMP  = 4;   // MOSFET gate (through 100 ohm resistor)
const uint8_t PIN_LEVEL = 5;   // float switch: closes to GND when water is OK
const uint8_t PIN_LED   = 13;  // onboard LED shows spray state

// ---------- Vision ----------
const uint8_t SQUIRREL_ID      = 1;    // ID given to "squirrel" when learned on HuskyLens
const int     FRAME_W          = 640;  // HuskyLens 2 frame width, confirm on your unit
const int     FRAME_H          = 480;  // HuskyLens 2 frame height, confirm on your unit
const int     DEADBAND_PX      = 25;   // ignore offsets smaller than this
const int     ALIGN_PX         = 40;   // "on target" if within this many pixels of center
const float   KP               = 0.02; // degrees of servo move per pixel of error, tune this
const int     CONFIRM_FRAMES   = 4;    // consecutive aligned frames before spraying

// ---------- Servo limits (degrees) ----------
const float PAN_MIN = 20,  PAN_MAX = 160, PAN_HOME = 90;
const float TILT_MIN = 40, TILT_MAX = 130, TILT_HOME = 90;

// ---------- Spray timing (ms) ----------
const unsigned long SPRAY_MS     = 1500;  // max continuous spray per event
const unsigned long COOLDOWN_MS  = 5000;  // rest time between sprays
const unsigned long LOST_HOME_MS = 3000;  // return to home after this long without a target

HUSKYLENS huskylens;
Servo panServo, tiltServo;

float panAng = PAN_HOME, tiltAng = TILT_HOME;
int confirmCount = 0;
bool spraying = false;
unsigned long sprayStart = 0, lastSpray = 0, lastSeen = 0;

void setPump(bool on) {
  digitalWrite(PIN_PUMP, on ? HIGH : LOW);
  digitalWrite(PIN_LED, on ? HIGH : LOW);
}

bool waterOK() {
  return digitalRead(PIN_LEVEL) == LOW;  // LOW = float switch closed = water present
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_PUMP, OUTPUT);
  setPump(false);
  pinMode(PIN_LEVEL, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);

  panServo.attach(PIN_PAN);
  tiltServo.attach(PIN_TILT);
  panServo.write(PAN_HOME);
  tiltServo.write(TILT_HOME);

  Serial1.begin(9600);  // HuskyLens UART on Teensy pins 0 (RX1) and 1 (TX1)
  while (!huskylens.begin(Serial1)) {
    Serial.println("HuskyLens not found, check wiring and baud rate");
    delay(500);
  }
  huskylens.writeAlgorithm(ALGORITHM_OBJECT_RECOGNITION);
  Serial.println("Squirrel Guard ready");
}

void loop() {
  unsigned long now = millis();

  // Stop the pump when the spray window ends or the water runs out
  if (spraying && (now - sprayStart >= SPRAY_MS || !waterOK())) {
    setPump(false);
    spraying = false;
    lastSpray = now;
    confirmCount = 0;
    Serial.println("spray off");
  }

  if (!huskylens.request()) return;

  bool found = false;
  int tx = 0, ty = 0;
  while (huskylens.available()) {
    HUSKYLENSResult r = huskylens.read();
    if (r.ID == SQUIRREL_ID) {
      found = true;
      tx = r.xCenter;
      ty = r.yCenter;
      break;
    }
  }

  if (found) {
    lastSeen = now;

    int ex = tx - FRAME_W / 2;
    int ey = ty - FRAME_H / 2;

    // Move the pan/tilt toward the target. Flip a sign if an axis moves the wrong way.
    if (abs(ex) > DEADBAND_PX) panAng -= KP * ex;
    if (abs(ey) > DEADBAND_PX) tiltAng += KP * ey;
    panAng  = constrain(panAng,  PAN_MIN,  PAN_MAX);
    tiltAng = constrain(tiltAng, TILT_MIN, TILT_MAX);
    panServo.write(panAng);
    tiltServo.write(tiltAng);

    bool aligned = abs(ex) <= ALIGN_PX && abs(ey) <= ALIGN_PX;
    confirmCount = aligned ? confirmCount + 1 : 0;

    if (!spraying &&
        confirmCount >= CONFIRM_FRAMES &&
        now - lastSpray >= COOLDOWN_MS &&
        waterOK()) {
      spraying = true;
      sprayStart = now;
      setPump(true);
      Serial.println("SPRAY");
    }
  } else {
    confirmCount = 0;
    if (!spraying && now - lastSeen > LOST_HOME_MS) {
      panAng = PAN_HOME;
      tiltAng = TILT_HOME;
      panServo.write(panAng);
      tiltServo.write(tiltAng);
    }
  }
}
