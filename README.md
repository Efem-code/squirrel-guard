# Squirrel Guard

A camera-aimed water sprayer that tracks a squirrel and sprays it when it stays in the target area. Built on a Teensy 4.1 with a HuskyLens 2 vision sensor, two MG996R servos for pan and tilt, and a 12V diaphragm pump.

## Contents

- `squirrel_guard.ino`: Teensy firmware
- `wiring_schematic.md`: power layout, pin map, pump driver circuit and build steps

## How it works

The HuskyLens 2 detects the squirrel and reports its position in the frame. The Teensy moves the pan and tilt servos to center it. Once the squirrel stays centered for a few frames, the pump sprays for 1.5 seconds, then waits 5 seconds before spraying again. A float switch in the reservoir cuts the pump when water is low.

## Before you power it up

The firmware was written against the DFRobot HUSKYLENS library from memory. Check the baud rate, frame size and function names against your HuskyLens 2 library version.

Test the pan and tilt directions on the bench with no water. If an axis moves the wrong way, flip the sign on that line in the loop.

Tune `KP` if the servos overshoot (lower it) or lag (raise it).

## Safety

Keep the Teensy and buck converters out of the spray path, and seal the case entry points. If the 12V supply runs from mains power, use a GFCI-protected outlet inside an approved enclosure.
