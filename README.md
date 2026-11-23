# Squirrel Guard

A camera-aimed water sprayer. The camera finds a squirrel, two servos aim a nozzle at it, and a small pump sprays it once it stays in the target area.

Built on a Teensy 4.1 with a HuskyLens 2 vision sensor, two MG996R servos for pan and tilt, and a 12V diaphragm pump.

## What is in this repo

| File | What it is |
|---|---|
| `squirrel_guard.ino` | Teensy firmware. Upload this to the Teensy. |
| `wiring_schematic.md` | Power layout, pin map, pump driver circuit and mechanical notes. |
| `dashboard/index.html` | Live status page for your computer. Open it in Chrome or Edge while the Teensy is plugged in. |

## How it works

1. The HuskyLens 2 spots the squirrel and reports where it is in the picture.
2. The Teensy moves the pan and tilt servos to keep the squirrel centered.
3. Once the squirrel stays centered for four frames in a row, the pump sprays for 1.5 seconds.
4. The system then waits 5 seconds before it can spray again.
5. A float switch in the reservoir stops the pump if the water runs low.

## Safety first

The firmware has a **BENCH_MODE** switch near the top of the file. While it is `true`, the pump is blocked in software and cannot turn on, so you can test aiming safely at your desk.

Keep `BENCH_MODE` set to `true` until every step in the checklist below has passed.

The Teensy and buck converters stay out of the spray path. Seal the case entries. If the 12V supply runs on mains power, use a GFCI-protected outlet and keep the connections inside an approved enclosure.

## Setup, step by step

### 1. Install the software

1. Install the Arduino IDE (version 2 or later).
2. Install the Teensyduino add-on from pjrc.com so the IDE knows the Teensy 4.1.
3. In the Library Manager, install **HUSKYLENS** by DFRobot.
4. Open `squirrel_guard.ino`.

### 2. Check the HuskyLens settings

The firmware was written from memory of the DFRobot library, so confirm these against your HuskyLens 2 documentation before uploading:

- The serial baud rate used in `setup()` (currently `9600` on `Serial1`).
- The frame size constants `FRAME_W` and `FRAME_H` (currently `640` x `480`).
- The library function names: `begin`, `writeAlgorithm`, `request`, `available`, `read`.

If the compiler complains about a name, the library version differs. Fix that line and keep the rest.

### 3. Teach the camera what a squirrel looks like

1. Set the HuskyLens 2 to **Object Recognition**.
2. Learn the squirrel as **ID 1**. Use several photos from different angles.

### 4. Wire it up

Follow `wiring_schematic.md`. Wire the logic side and the servo side before you connect any water.

### 5. Upload and open the dashboard

1. Select **Teensy 4.1** as the board and upload the sketch.
2. Open `dashboard/index.html` in **Chrome or Edge**. Firefox and Safari cannot use the USB connection the dashboard needs.
3. Click **Connect to Teensy** and pick the Teensy's serial port.
4. The Mode tile should read **BENCH**.

### 6. Bench checklist (no water)

Do these in order. Each step must pass before you move on.

1. **Camera sees the target.** Hold a photo of a squirrel in front of the camera. The Target tile should read **Yes**.
2. **Pan direction is right.** Move the photo left and right. The dot on the Aim panel should move the same way. If it moves backwards, change the sign on the `panAng -=` line and upload again.
3. **Tilt direction is right.** Move the photo up and down. The dot should follow. If not, change the sign on the `tiltAng +=` line.
4. **Centering works.** Center the photo. The Aligned tile should turn **Yes** within about a second.
5. **Pump stays off.** The Spraying tile should never turn on while in BENCH mode, even when Aligned is Yes.
6. **Lost target homes.** Remove the photo. After about 3 seconds the servos should return to the home position.

### 7. Tune the aiming (optional)

- If the servos overshoot the target and wobble, lower `KP`.
- If they are slow to follow, raise `KP` a little.

### 8. First live test

Only do this after the bench checklist passes.

1. Set `BENCH_MODE` to `false` in the code and upload again.
2. Fill the reservoir and check that the Water tile reads **OK**.
3. Keep people and pets out of the spray zone.
4. Stand back and watch the first few sprays on the dashboard.

## Troubleshooting

| What you see | What to check |
|---|---|
| Dashboard says "Web Serial is not available" | Use Chrome or Edge on a computer. |
| "Camera not found" in the log | HuskyLens wiring (RX and TX may be swapped), its baud rate, and its power. |
| Target never reads Yes | Re-learn the squirrel as ID 1 with more photos, and check lighting. |
| Dot moves the wrong way | Flip the sign on that axis in the code. |
| Servos jitter or shake | Power them from the 6V buck, not the Teensy. Lower `KP`. |
| Teensy resets when servos move | Shared power rail or weak supply. Check the buck converters and grounds. |
| Water shows LOW with a full reservoir | Check the float switch wiring and that it closes to GND when floating up. |
| Pump runs but nothing sprays | Check the tubing, the check valve direction and the nozzle. |

## Ideas for later

- Add a physical emergency stop button that cuts the pump power.
- Log spray counts to a file so you can see activity over days.
- Add a relay on the pump if you prefer it over the MOSFET.
