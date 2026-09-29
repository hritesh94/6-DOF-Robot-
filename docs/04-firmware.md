# Firmware (Teensy 4.1)

File: `firmware/tw2k_firmware/tw2k_firmware.ino` — needs **Teensyduino** and the **AccelStepper** library.

## What it does

- Drives 6 steppers with **coordinated moves**: all joints start and finish together, each with its own speed cap.
- **Homes** J1–J5: fast approach to the switch, back off, slow second touch, then sets that angle.
- **Stops everything** if an endstop is hit while moving toward it.
- **Saves calibration in EEPROM**: motor direction, homing direction, zero offsets, soft limits, steps/degree.

## Config at the top of the file

| Setting | Meaning |
|---|---|
| `STEP_PIN`, `DIR_PIN`, `EN_PIN`, `SW_PIN` | Pin map (matches my PCB) |
| `MICROSTEPS`, `GEAR` | Default steps/degree (can be overridden from the app) |
| `INVERT` | Default motor directions |
| `LIM_MIN`, `LIM_MAX` | Default soft limits (degrees) |
| `HOME_DIR`, `OLD_RETURN_STEPS`, `HOME_FAST`, `HOME_SLOW`, `SEARCH_MAX` | Homing |
| `MAX_DEG_S` | Top speed per joint (J2 capped at 12°/s) |
| `JOG_DEG`, `JOG_ACC_DEG` | Jog speed and ramp |

## Serial protocol

USB serial, one command per line.

| Command | Action |
|---|---|
| `M j1 … j6` | Move to joint angles (degrees, absolute, coordinated) |
| `V deg/s` | Set max joint speed |
| `J n ±1` / `J 0 0` | Start / stop hold-to-jog on joint n |
| `H` / `H n` | Home J1–J5 (then go to zero) / home joint n |
| `Z` | Make the current position zero on all joints (saved for homed joints) |
| `O n` | Make joint n's current position its zero (saved) |
| `K n min max` | Set joint n's soft limits (saved) |
| `G n spd` | Set joint n's steps/degree (saved) |
| `I n 0/1` | Normal / reversed motor direction (saved) |
| `D n ±1` | Direction joint n drives to find its switch (saved) |
| `N n 0/1` | Disable / enable joint n's driver |
| `S` / `E` / `R` | Smooth stop / E-stop (disable drivers) / re-enable |
| `T n` | Hardware test: raw 1600 steps forward and back |
| `X` | Forget all saved calibration (then power-cycle) |
| `?` | Report everything |

Replies: `P` positions, `L` endstops, `N` enables, `D` homing directions, `INV` motor directions, `SPD` steps/degree, `LIM` limits, plus `OK …`, `ERR …`, `HOMED`.

## EEPROM layout

| Address | Content |
|---|---|
| 0 | Magic byte (`0xB8`) |
| 1–5 | Homing direction per joint (+1 stored as 2, −1 as 0) |
| 6–11 | Motor reversed flags |
| 15 | Calibration magic (`0xC2`) |
| 16+ | `struct Cal { float lmin[6], lmax[6], off[6], spd[6]; }` |
