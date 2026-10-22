/*
 * Newly written reference firmware, October 2026; not original project code.
 * Uno + Adafruit shield + two DC drive motors + one front HC-SR04.
 * Optional suitably rated DC suction/pump motor on a third shield channel.
 * Pin assignments, motor count and shield revision require verification.
 */
#include <Arduino.h>
#include "Config.h"
#if MOTOR_SHIELD_VERSION == 1
#include <AFMotor.h>
AF_DCMotor leftMotor(LEFT_CHANNEL);
AF_DCMotor rightMotor(RIGHT_CHANNEL);
AF_DCMotor pumpMotor(PUMP_CHANNEL);
#elif MOTOR_SHIELD_VERSION == 2
#include <Wire.h>
#include <Adafruit_MotorShield.h>
Adafruit_MotorShield shield;
Adafruit_DCMotor *leftMotor = shield.getMotor(LEFT_CHANNEL);
Adafruit_DCMotor *rightMotor = shield.getMotor(RIGHT_CHANNEL);
Adafruit_DCMotor *pumpMotor = shield.getMotor(PUMP_CHANNEL);
#else
#error "MOTOR_SHIELD_VERSION must be 1 or 2"
#endif

enum class State : uint8_t { STOPPED, SETTLING, CRUISING, TURNING, FAULT };
State state = State::STOPPED;
bool ready = false;
bool turnRight = true;
uint8_t turnAttempts = 0;
uint8_t clearSamples = 0;
int distanceCm = -1;
unsigned long lastSample = 0;
unsigned long stateStarted = 0;

void setMotor(uint8_t index, int demand) {
  demand = constrain(demand, -255, 255);
  const uint8_t speed = static_cast<uint8_t>(demand < 0 ? -demand : demand);
  const uint8_t direction = demand > 0 ? FORWARD : demand < 0 ? BACKWARD : RELEASE;
#if MOTOR_SHIELD_VERSION == 1
  AF_DCMotor *motor = index == 0 ? &leftMotor : index == 1 ? &rightMotor : &pumpMotor;
#else
  Adafruit_DCMotor *motor = index == 0 ? leftMotor : index == 1 ? rightMotor : pumpMotor;
#endif
  motor->setSpeed(speed);
  motor->run(direction);
}

void drive(int left, int right) {
  setMotor(0, INVERT_LEFT ? -left : left);
  setMotor(1, INVERT_RIGHT ? -right : right);
}

void stopOutputs() {
  if (!ready) return;
  drive(0, 0);
  setMotor(2, 0);
}

void changeState(State next) {
  state = next;
  stateStarted = millis();
}

void stopRobot(bool fault) {
  stopOutputs();
  changeState(fault ? State::FAULT : State::STOPPED);
  clearSamples = 0;
  turnAttempts = 0;
  Serial.println(fault ? F("FAULT: outputs off. Inspect robot; g retries.") : F("Stopped."));
}

int readDistanceCm() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  const unsigned long echo = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (!echo) return -1;
  const int cm = static_cast<int>(echo / 58UL);
  return cm >= 2 && cm <= 400 ? cm : -1;
}

void handleCommand(char c) {
  if (c == '\n' || c == '\r' || c == ' ') return;
  if (c == 'x' || c == 's') { stopRobot(false); return; }
  if (!ready) { Serial.println(F("Verify Config.h before enabling motor outputs.")); return; }
  if (c == 'g') {
    stopOutputs();
    distanceCm = -1;
    clearSamples = 0;
    turnAttempts = 0;
    lastSample = millis() - SAMPLE_MS;
    changeState(State::SETTLING);
    Serial.println(F("Started; waiting for range checks."));
  } else if (c == 'd') {
    Serial.print(F("Last distance cm (-1 invalid): "));
    Serial.println(distanceCm);
  } else Serial.println(F("Commands: g start, x stop, d last distance."));
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIGGER_PIN, LOW);
  if (!WIRING_CONFIRMED) {
    Serial.println(F("Configuration lock active. Verify wiring and shield version."));
    return;
  }
#if MOTOR_SHIELD_VERSION == 2
  if (!shield.begin()) {
    Serial.println(F("Motor shield not detected; outputs disabled."));
    return;
  }
#endif
  ready = true;
  stopRobot(false);
  Serial.println(F("Ready. g=start, x=stop. Pump follows ENABLE_PUMP in Config.h."));
}

void loop() {
  while (Serial.available()) handleCommand(static_cast<char>(Serial.read()));
  if (!ready || state == State::STOPPED || state == State::FAULT) return;

  const unsigned long now = millis();
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;
    distanceCm = readDistanceCm();
    // Timeouts, invalid distances and dangerously close objects stop all outputs.
    if (distanceCm < TOO_CLOSE_CM) { stopRobot(true); return; }
    if (distanceCm >= CLEAR_CM) {
      if (clearSamples < 2) ++clearSamples;
    } else clearSamples = 0;
    if (state == State::CRUISING && distanceCm <= OBSTACLE_CM) {
      stopOutputs();
      turnRight = !turnRight;
      turnAttempts = 0;
      changeState(State::SETTLING);
      return;
    }
  }

  if (state == State::TURNING) {
    if (millis() - stateStarted >= TURN_MS) {
      stopOutputs();
      clearSamples = 0;
      changeState(State::SETTLING);
    }
    return;
  }
  if (state != State::SETTLING || millis() - stateStarted < SETTLE_MS) return;

  if (clearSamples >= 2) {
    drive(DRIVE_PWM, DRIVE_PWM);
    setMotor(2, ENABLE_PUMP ? PUMP_PWM : 0);
    turnAttempts = 0;
    changeState(State::CRUISING);
  } else if (distanceCm >= TOO_CLOSE_CM && distanceCm <= OBSTACLE_CM) {
    if (turnAttempts >= MAX_TURN_ATTEMPTS) { stopRobot(true); return; }
    ++turnAttempts;
    setMotor(2, 0);
    drive(turnRight ? TURN_PWM : -TURN_PWM, turnRight ? -TURN_PWM : TURN_PWM);
    changeState(State::TURNING);
  }
  // In the threshold gap, stay stopped until distance is clearly free or blocked.
}
