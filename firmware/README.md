# Vacuum robot — L298 and three ultrasonic sensors

Newly written reference firmware for the reported Arduino Uno, L298 driver and three HC-SR04 sensors. This replaces the earlier Adafruit-based implementation. It is not original project code and has not been tested on the physical robot.

## Selected reference configuration

The implementation adopts **two geared drive motors**, one per wheel, connected to the two L298N bridges as shown in the [supplied circuit](../media/images/vacuum-circuit-reference.jpg). Independent wheel control enables the turning routine. This is the chosen reference design, not confirmation of the original motor count. A single-drive-motor chassis needs a different steering implementation. `WIRING_CONFIRMED` remains false until the physical build is checked against this configuration.

The reported pump is **7 V**. Its current, driver and wiring are unknown, and the circuit drawing does not show it. **No pump output is implemented.** The two L298 bridges in this design are already assigned to the wheels. A separate appropriately rated switching stage would be needed to control an additional pump; do not attach it to an Arduino GPIO.

## Proposed pin map

These are explicit new-code assignments, not a claimed transcription of every wire in the low-resolution drawing. Match the actual wiring to this table or edit `Config.h`.

| Signal | Uno pin |
|---|---|
| Left motor ENA (PWM) | D5 |
| Left motor IN1 / IN2 | D7 / D8 |
| Right motor ENB (PWM) | D6 |
| Right motor IN3 / IN4 | D9 / D10 |
| Left HC-SR04 trigger / echo | A0 / A1 |
| Front HC-SR04 trigger / echo | A2 / A3 |
| Right HC-SR04 trigger / echo | A4 / A5 |

A0–A5 are used as digital pins. Connect sensor supplies to regulated 5 V and common ground. Match left/right motor outputs to the intended wheel. Remove ENA/ENB jumpers before connecting PWM pins; do not confuse these with the module's regulator jumper. Follow the particular module's power instructions. Confirm motor voltage/stall current and account for driver voltage drop rather than treating the diagram's generic supply labels as the project's battery specification. Use a suitable motor supply with shared logic ground. Keep enable inputs pulled low during reset with external pull-downs if the module does not already provide them.

## Build and first checks

1. Open `VacuumRobot/VacuumRobot.ino` and select **Arduino Uno**. No Adafruit or other third-party library is needed.
2. Check that the build has independent left/right drive motors and matches the pin table, then set `WIRING_CONFIRMED = true` in `Config.h`.
3. Test wheel direction with wheels raised; adjust `INVERT_LEFT` and `INVERT_RIGHT` if needed. Keep the pump independently disconnected during drive tests.
4. Open Serial Monitor at **9600 baud**. `g` starts scanning and autonomous movement; `x` or `s` stops; `d` prints the last left/front/right readings. Startup is stopped.
5. Test each sensor, blocked paths and stop commands at low speed in a level enclosed area. Tune distances, speed and pivot duration for the chassis.

## Control behavior

Sensors are triggered sequentially, one about every 70 ms, so each is sampled about every 210 ms. A reading times out after 25 ms; invalid echoes or any distance below 6 cm cause a fault stop. Timeouts are never interpreted as an open path. Only fresh readings from all three sensors permit movement.

Forward travel starts when the front is at least 30 cm clear and both sides at least 15 cm clear. While moving, a front distance at or below 25 cm or side distance at or below 12 cm stops the wheels and starts a new scan. If blocked, the robot pivots toward the clearer side only when that side reports at least 25 cm. With neither side clear, it stops in a fault state. A pivot lasts up to the configured 250 ms before another stopped scan. Six unsuccessful pivots cause a fault. In the threshold gaps the stopped robot waits. Send `g` to retry after checking the cause.

Software stopping disables the motor bridges; it is not a guaranteed mechanical brake. Sensor sampling, echo waits and inertia affect stopping distance. Three ultrasonic sensors cannot prove that the entire turning area is clear, and there is no rear or drop-off detection. Use away from stairs and edges. Pump control, cleaning coverage, docking and cleaning performance are not implemented or validated.

## Verification

Compiled for `arduino:avr:uno` with Arduino AVR Boards 1.8.8, both with the default configuration lock and an enabled configuration. This checks compilation only; actual wiring, motor count, motion and obstacle response still need physical validation.

[L298 manufacturer datasheet](https://www.st.com/resource/en/datasheet/l298.pdf) documents the two bridges, direction inputs and independent enable inputs used here. The particular module's supply/regulator arrangement must also be checked.
