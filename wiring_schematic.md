# Squirrel Guard: Wiring Schematic and Build Notes

## Parts
- Teensy 4.1 (3.3V logic)
- HuskyLens 2 (camera, set to Object Recognition)
- 2x MG996R servos and 2x servo mount brackets (pan and tilt)
- Aimela 85 PSI 12V diaphragm pump
- 3/8" ID braided vinyl tubing
- Clear outdoor case
- 12V power supply (10A recommended)
- Added parts: 2x buck converter (one set to 6.0V at 5A, one set to 5.0V at 3A), IRLZ44N logic-level MOSFET, 1N5822 flyback diode, 100 ohm and 10k ohm resistors, 10A inline fuse, float switch for the water reservoir, nozzle, reservoir, check valve

## Power

| Rail | Source | Feeds |
|---|---|---|
| +12V | 12V PSU through 10A fuse | Pump (+), Buck A input, Buck B input |
| +6V | Buck A (set to 6.0V) | Servo V+ (pan and tilt) only |
| +5V | Buck B (set to 5.0V) | Teensy VIN, HuskyLens VCC |
| GND | Common ground | Every GND in the system |

Keep servo power separate from logic power. MG996R servos can pull 2 to 2.5A each when stalled, which will brown out the Teensy if they share a rail.

## Teensy Pin Map

| Teensy pin | Connects to |
|---|---|
| 0 (RX1) | HuskyLens TX |
| 1 (TX1) | HuskyLens RX |
| 2 | Pan servo signal (orange or yellow wire) |
| 3 | Tilt servo signal |
| 4 | 100 ohm resistor, then MOSFET gate |
| 5 | Float switch to GND (uses internal pull-up) |
| 13 | Onboard LED, status only |
| GND | Common ground |

## Pump Driver (low-side MOSFET)

```
+12V ───────────────┬─────────────────────────┐
                    │                         │
                  [Pump +]                 [1N5822 flyback]
                  [Pump -]                    (stripe to +12V)
                    │                         │
                    └──────── Drain ──┐       │
                                      │       │
                              IRLZ44N         │
                              Gate ──[100Ω]── Teensy pin 4
                              Gate ──[10kΩ]── GND
                              Source ──────── GND (common)
```

The flyback diode across the pump protects the MOSFET from the inductive kick when the pump switches off.

## Overall Wiring

```
12V PSU (+) ──[10A fuse]──┬──> Buck A in  ──> Buck A out 6.0V ──> Servo V+ (pan, tilt)
                          ├──> Buck B in  ──> Buck B out 5.0V ──> Teensy VIN, HuskyLens VCC
                          └──> Pump (+) (through MOSFET low side)

Teensy pin 0 (RX1) <── HuskyLens TX
Teensy pin 1 (TX1) ──> HuskyLens RX
Teensy pin 2       ──> Pan servo signal
Teensy pin 3       ──> Tilt servo signal
Teensy pin 4       ──> MOSFET gate (100Ω, 10kΩ pulldown)
Teensy pin 5       <── Float switch (other side to GND)

GND common: 12V PSU (-), Buck A (-), Buck B (-), Teensy GND, HuskyLens GND,
            both servo GND, MOSFET Source, float switch return.
```

## Mechanical Layout
1. Mount the HuskyLens and the nozzle on the same pan/tilt bracket so the nozzle always points where the camera is looking.
2. Put the pump, reservoir and Teensy in the clear case, away from the spray. Route the tubing out through a sealed gland.
3. Aim the nozzle slightly low so the stream lands on the target instead of above it.
4. Use dielectric grease on the connectors and silicone-seal the case entry points.

## Setup Steps
1. Flash the Teensy with `squirrel_guard.ino`. Open the serial monitor at 115200.
2. Set the HuskyLens 2 to Object Recognition and learn the squirrel as ID 1. Use several photos from different angles.
3. Bench-test without water first. Check that the pan and tilt move in the right direction when a squirrel is moved left or right.
4. If pan or tilt moves the wrong way, flip the sign on the matching line in the loop (`panAng -=` or `tiltAng +=`).
5. Tune `KP` if the movement overshoots (lower it) or lags (raise it).
6. Set the buck converters with the servos and Teensy connected. Do not power the servos through the Teensy.
7. Test the pump with the MOSFET first, then fill the reservoir and test spray duration.

## Safety and Outdoor Notes
- Use a GFCI-protected outlet for the 12V PSU if it is mains-powered. Keep all mains connections inside an approved enclosure.
- Keep the Teensy and buck converters out of the spray path.
- The `SPRAY_MS` limit stops the pump after 1.5 seconds. Raise it only after you have tested the spray.
- The float switch cuts the pump when the reservoir is low, so the pump does not run dry.
- Confirm the HuskyLens 2 baud rate, frame size and library calls against the version you have, since the firmware and API may differ from the sketch.
