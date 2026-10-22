# Vacuum robot reference firmware

Newly written Arduino code for the described Uno/ultrasonic/Adafruit prototype. It is not recovered original code and has not been tested on the physical robot. Existing code screenshots are historical assets, not the wiring specification for this implementation.

## Assumed hardware

The implementation assumes **two independently driven DC wheels**, one forward-facing **HC-SR04**, and an optional DC suction/pump motor. Confirm the actual motor count, shield revision and pump rating before use. It supports either the Adafruit V1 or V2 motor shield; it does not automatically identify the version.

| Connection | Proposed assignment |
|---|---|
| Left wheel | Shield M1 |
| Right wheel | Shield M2 |
| Optional DC pump/suction motor | Shield M3 |
| HC-SR04 trigger | Uno A0 used as a digital pin |
| HC-SR04 echo | Uno A1 used as a digital pin |
| HC-SR04 supply | 5 V and common ground |

Use a motor supply compatible with the actual motors and shield. Check each motor's stall current and the pump's startup current against the shield's limits. If the pump exceeds them, it needs a separately rated driver and a corresponding code change; do not connect it directly to an Arduino GPIO. Follow the actual shield's power-jumper instructions and share the required grounds.

## Setup

1. Open `VacuumRobot/VacuumRobot.ino` with its adjacent `Config.h`.
2. Set `MOTOR_SHIELD_VERSION` to **1** for `AFMotor.h`, or **2** for `Adafruit_MotorShield.h`. Install the matching official library; V2 also needs Adafruit BusIO.
3. Select **Arduino Uno**. Check channel assignments and sensor pins. If the platform has four independently controlled drive motors, adapt the code before use.
4. Keep `ENABLE_PUMP = false` for initial movement tests. Verify wheel directions with wheels raised; use `INVERT_LEFT` / `INVERT_RIGHT` if necessary.
5. Set `WIRING_CONFIRMED = true` after checking the wiring. The distributed sketch stays locked until this flag is changed and uploaded.
6. Open Serial Monitor at **9600 baud**. Send `g` to start, `x` or `s` to stop, and `d` to print the last distance. The robot does not start moving at power-up.
7. Test with the pump disabled first. Enable it only after its electrical compatibility is confirmed and the movement behavior is validated.

## Behavior

Two readings of at least 30 cm allow forward travel. A front obstacle at 25 cm or closer stops the wheels and pump, followed by a short pivot and a new measurement. Turn direction alternates between encounters. After six unsuccessful pivots the robot stops in a fault state. A missing/invalid echo or distance below 6 cm immediately stops the outputs when that reading is processed. Send `g` to retry only after inspecting the cause.

The pump, when enabled, runs only during forward cruising and is off while stopped, turning or faulted. Thresholds, speeds, settling time and turn duration are editable in `Config.h`. While waiting to resume, readings between 25 and 30 cm keep the robot stopped; while already cruising it continues until the obstacle threshold is reached.

Range checks are scheduled about every 70 ms, with a maximum 25 ms echo wait. Motor commands are stopped in software, but actual stopping distance depends on inertia and the driver. There is **no cliff sensor, side/rear coverage, mapping, coverage planning or docking**. Use a level, enclosed test area away from stairs; a front ultrasonic sensor cannot detect a drop-off. Adjust turn timing to the actual chassis.

## Verification and sources

Compiled for `arduino:avr:uno` using Arduino AVR Boards 1.8.8 with both V1 and V2 library paths, including enabled movement/pump configurations. Physical operation, sensor placement, motor direction, cleaning performance and pump compatibility remain unverified.

- [Adafruit V1 DC motor API and hardware guidance](https://learn.adafruit.com/adafruit-motor-shield/using-dc-motors)
- [Adafruit V2 DC motor API and hardware guidance](https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/using-dc-motors)
- [V1 library](https://github.com/adafruit/Adafruit-Motor-Shield-library)
- [V2 library](https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library)
