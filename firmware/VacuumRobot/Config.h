#pragma once
#include <Arduino.h>
// Proposed pins, not a verified transcription of the small reference drawing.
// Reference drawing has TWO drive motors; user's reported count needs resolving.
constexpr bool WIRING_CONFIRMED = false;
constexpr uint8_t LEFT_ENABLE = 5;  // L298 ENA, remove its enable jumper for PWM
constexpr uint8_t LEFT_IN1 = 7;
constexpr uint8_t LEFT_IN2 = 8;
constexpr uint8_t RIGHT_ENABLE = 6; // L298 ENB, remove its enable jumper for PWM
constexpr uint8_t RIGHT_IN1 = 9;
constexpr uint8_t RIGHT_IN2 = 10;
constexpr bool INVERT_LEFT = false;
constexpr bool INVERT_RIGHT = false;
// Sensor order: left, front, right. A0-A5 used as digital pins.
constexpr uint8_t TRIGGER_PINS[3] = {A0, A2, A4};
constexpr uint8_t ECHO_PINS[3] = {A1, A3, A5};
constexpr uint8_t DRIVE_PWM = 110;
constexpr uint8_t TURN_PWM = 100;
constexpr int FRONT_BLOCKED_CM = 25;
constexpr int FRONT_CLEAR_CM = 30;
constexpr int SIDE_BLOCKED_CM = 12;
constexpr int SIDE_CLEAR_CM = 15;
constexpr int TURN_CLEAR_CM = 25;
constexpr int TOO_CLOSE_CM = 6;
constexpr unsigned long PING_INTERVAL_MS = 70;
constexpr unsigned long MAX_READING_AGE_MS = 350;
constexpr unsigned long ECHO_TIMEOUT_US = 25000;
constexpr unsigned long TURN_MS = 250;
constexpr uint8_t MAX_TURN_ATTEMPTS = 6;
// No pump output: 7 V pump driver/current and wiring are not confirmed.
