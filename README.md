# Robotic Vacuum Cleaner Prototype

A compact floor-cleaning robot developed around an **Arduino Uno**, **HC-SR04 ultrasonic sensing** and an **L298 motor driver**. The build combines mobile movement with a motor-driven suction/pump mechanism inside a circular enclosure.

![Vacuum robot prototype](media/images/img-3926.jpg)

## My contribution — Semester 6

I worked on motor integration, coding and the electrical connections for the vacuum robot.

## Hardware and operation

The Arduino Uno provides control for the robot. Three ultrasonic sensors supply distance information, while an L298 driver controls movement. A 7 V suction/pump motor provides the cleaning mechanism; its electrical control is outside the documented reference firmware. The project was programmed using Arduino.

| Item | Details |
| --- | --- |
| Controller | Arduino Uno |
| Distance sensing | Three HC-SR04 ultrasonic sensors |
| Motor control | L298; the supplied reference diagram shows an L298N module |
| Cleaning mechanism | 7 V suction/pump motor |
| Programming | Arduino |

## New reference firmware

A newly written implementation is available in [firmware/VacuumRobot](firmware/VacuumRobot). It now uses direct L298 control and three ultrasonic sensors. The firmware adopts the supplied diagram's **two-wheel differential-drive arrangement**, with independent left and right motor control. This is the selected reference design; the original drive-motor count was not independently verified.

The code stops for invalid sensor readings, checks clearance before moving, and uses short pivots toward the clearer side. The 7 V pump remains outside software control because its driver circuit and current rating are not documented.

This firmware is newly written and has not been tested on the physical robot. Follow the [setup guide](firmware/README.md) before uploading.

## Circuit reference

![Supplied vacuum robot circuit](media/images/vacuum-circuit-reference.jpg)

The reference shows an Uno, three HC-SR04 sensors, an L298N and two geared drive motors. It does not show the suction pump circuit. The drawing's generic supply labels are not confirmed project battery specifications.

## Assembly views

![Internal assembly view](media/images/img-3924.jpg)

![Additional assembly view](media/images/img-3925.jpg)

## Earlier code references

The [original code screenshots](docs/code-screenshots) are retained separately. They show a serial-command program using direct motor-control pins, so the new three-sensor L298 implementation is not presented as a copy of that program.
