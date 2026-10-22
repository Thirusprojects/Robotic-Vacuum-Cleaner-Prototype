#pragma once
#include <Arduino.h>

// Choose the library for your ACTUAL shield: 1=AFMotor (V1), 2=MotorShield V2.
#ifndef MOTOR_SHIELD_VERSION
#define MOTOR_SHIELD_VERSION 1
#endif
// These pins/channels are proposed, not confirmed original wiring.
constexpr bool WIRING_CONFIRMED = false;
constexpr bool ENABLE_PUMP = false; // Enable only after checking voltage/current.
constexpr uint8_t LEFT_CHANNEL = 1;
constexpr uint8_t RIGHT_CHANNEL = 2;
constexpr uint8_t PUMP_CHANNEL = 3;
constexpr bool INVERT_LEFT = false;
constexpr bool INVERT_RIGHT = false;
constexpr uint8_t TRIGGER_PIN = A0;
constexpr uint8_t ECHO_PIN = A1;
constexpr uint8_t DRIVE_PWM = 110;
constexpr uint8_t TURN_PWM = 100;
constexpr uint8_t PUMP_PWM = 200;
constexpr uint16_t OBSTACLE_CM = 25;
constexpr uint16_t CLEAR_CM = 30;
constexpr uint16_t TOO_CLOSE_CM = 6;
constexpr unsigned long SAMPLE_MS = 70UL;
constexpr unsigned long SETTLE_MS = 180UL;
constexpr unsigned long TURN_MS = 350UL;
constexpr unsigned long ECHO_TIMEOUT_US = 25000UL;
constexpr uint8_t MAX_TURN_ATTEMPTS = 6;
static_assert(LEFT_CHANNEL >= 1 && LEFT_CHANNEL <= 4, "Invalid left channel");
static_assert(RIGHT_CHANNEL >= 1 && RIGHT_CHANNEL <= 4, "Invalid right channel");
static_assert(PUMP_CHANNEL >= 1 && PUMP_CHANNEL <= 4, "Invalid pump channel");
static_assert(LEFT_CHANNEL != RIGHT_CHANNEL && LEFT_CHANNEL != PUMP_CHANNEL &&
              RIGHT_CHANNEL != PUMP_CHANNEL, "Motor channels must be distinct");
