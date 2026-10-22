/* Newly written reference firmware. Uno, L298, TWO drive motors, 3 HC-SR04.
 * Not original project code. Verify motor count and Config.h before use.
 * The 7 V pump is not controlled by this sketch: its driver is unconfirmed.
 */
#include <Arduino.h>
#include "Config.h"
enum class State : uint8_t { STOPPED, CHECKING, CRUISING, TURNING, FAULT };
State state = State::STOPPED;
bool ready = false;
int ranges[3] = {-1, -1, -1};
unsigned long measuredAt[3] = {0, 0, 0};
bool measured[3] = {false, false, false};
uint8_t nextSensor = 0;
uint8_t turnAttempts = 0;
unsigned long lastPing = 0;
unsigned long turnStarted = 0;

void motor(uint8_t enable, uint8_t in1, uint8_t in2, int demand) {
  demand = constrain(demand, -255, 255);
  analogWrite(enable, 0); // Disable bridge while changing direction.
  digitalWrite(in1, demand > 0 ? HIGH : LOW);
  digitalWrite(in2, demand < 0 ? HIGH : LOW);
  analogWrite(enable, demand < 0 ? -demand : demand);
}
void drive(int left, int right) {
  if (!ready) return;
  motor(LEFT_ENABLE, LEFT_IN1, LEFT_IN2, INVERT_LEFT ? -left : left);
  motor(RIGHT_ENABLE, RIGHT_IN1, RIGHT_IN2, INVERT_RIGHT ? -right : right);
}
void stopRobot(bool fault) {
  drive(0, 0);
  state = fault ? State::FAULT : State::STOPPED;
  Serial.println(fault ? F("FAULT: stopped; inspect before g retry.") : F("Stopped."));
}
void resetScan() {
  for (uint8_t i = 0; i < 3; ++i) { measured[i] = false; ranges[i] = -1; }
  nextSensor = 0;
  lastPing = millis();
}
int readDistance(uint8_t index) {
  digitalWrite(TRIGGER_PINS[index], LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PINS[index], HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PINS[index], LOW);
  unsigned long echo = pulseIn(ECHO_PINS[index], HIGH, ECHO_TIMEOUT_US);
  if (!echo) return -1;
  int cm = echo / 58UL;
  return cm >= 2 && cm <= 400 ? cm : -1;
}
bool allFresh() {
  for (uint8_t i = 0; i < 3; ++i)
    if (!measured[i] || millis() - measuredAt[i] > MAX_READING_AGE_MS) return false;
  return true;
}
void handleCommand(char c) {
  if (c == '\r' || c == '\n' || c == ' ') return;
  if (c == 'x' || c == 's') { stopRobot(false); return; }
  if (!ready) { Serial.println(F("Verify wiring and TWO drive motors in Config.h.")); return; }
  if (c == 'g') {
    drive(0, 0);
    resetScan();
    turnAttempts = 0;
    state = State::CHECKING;
  } else if (c == 'd') {
    Serial.print(F("Last cm L/F/R (-1 invalid): "));
    for (uint8_t i = 0; i < 3; ++i) { Serial.print(ranges[i]); Serial.print(' '); }
    Serial.println();
  }
}
void setup() {
  Serial.begin(9600);
  // Establish disabled L298 outputs even while configuration remains locked.
  const uint8_t pins[] = {LEFT_ENABLE, RIGHT_ENABLE, LEFT_IN1, LEFT_IN2, RIGHT_IN1, RIGHT_IN2};
  for (uint8_t p : pins) { digitalWrite(p, LOW); pinMode(p, OUTPUT); }
  for (uint8_t i = 0; i < 3; ++i) {
    pinMode(TRIGGER_PINS[i], OUTPUT);
    digitalWrite(TRIGGER_PINS[i], LOW);
    pinMode(ECHO_PINS[i], INPUT);
  }
  if (!WIRING_CONFIRMED) {
    Serial.println(F("Configuration locked. Confirm pin map and two-wheel drive."));
    return;
  }
  ready = true;
  stopRobot(false);
  Serial.println(F("g start, x stop, d last ranges. No pump control."));
}
void loop() {
  while (Serial.available()) handleCommand(static_cast<char>(Serial.read()));
  if (!ready || state == State::STOPPED || state == State::FAULT) return;
  // Sequential pings reduce cross-talk. One sensor every 70 ms, each about 210 ms.
  if (millis() - lastPing >= PING_INTERVAL_MS) {
    lastPing = millis();
    const uint8_t i = nextSensor;
    ranges[i] = readDistance(i);
    measuredAt[i] = millis();
    measured[i] = true;
    nextSensor = (i + 1) % 3;
    if (ranges[i] < TOO_CLOSE_CM) { stopRobot(true); return; } // Includes timeout.
  }
  if (state == State::TURNING) {
    if (!allFresh()) { stopRobot(true); return; }
    if (millis() - turnStarted >= TURN_MS) {
      drive(0, 0);
      resetScan();
      state = State::CHECKING;
    }
    return;
  }
  if (!allFresh()) {
    if (state == State::CRUISING) stopRobot(true);
    return;
  }
  const bool blocked = ranges[1] <= FRONT_BLOCKED_CM ||
                       ranges[0] <= SIDE_BLOCKED_CM || ranges[2] <= SIDE_BLOCKED_CM;
  if (state == State::CRUISING) {
    if (blocked) { drive(0, 0); resetScan(); state = State::CHECKING; }
    return;
  }
  if (ranges[1] >= FRONT_CLEAR_CM && ranges[0] >= SIDE_CLEAR_CM && ranges[2] >= SIDE_CLEAR_CM) {
    drive(DRIVE_PWM, DRIVE_PWM);
    turnAttempts = 0;
    state = State::CRUISING;
  } else if (blocked) {
    const bool leftClear = ranges[0] >= TURN_CLEAR_CM;
    const bool rightClear = ranges[2] >= TURN_CLEAR_CM;
    if ((!leftClear && !rightClear) || turnAttempts >= MAX_TURN_ATTEMPTS) {
      stopRobot(true); return;
    }
    ++turnAttempts;
    const bool turnLeft = leftClear && (!rightClear || ranges[0] >= ranges[2]);
    drive(turnLeft ? -TURN_PWM : TURN_PWM, turnLeft ? TURN_PWM : -TURN_PWM);
    turnStarted = millis();
    state = State::TURNING;
  }
  // Between clear/blocked thresholds, remain stopped while scanning.
}
