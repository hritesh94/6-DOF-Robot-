# Hardware

> 📷 Suggested: PCB 3D render from KiCad, soldered board, board installed in the base, wiring overview.

## Control board

- **MCU**: Teensy 4.1 (3.3 V logic, **not 5 V tolerant**)
- **Drivers**: 6× TMC2209 modules in standalone STEP/DIR mode
- **Endstops**: 5 sockets (J7–J11 on the board) for J1–J5; J6 has no switch
- **Design files**: `hardware/pcb/` (KiCad project, schematic PDF, Gerbers)

### Pin map (Teensy 4.1)

| Joint | STEP | DIR | EN (active LOW) | Endstop pin | Board socket |
|---|---|---|---|---|---|
| J1 | 3 | 2 | 4 | 32 | J7 |
| J2 | 6 | 5 | 9 | 31 | J8 |
| J3 | 12 | 11 | 26 | 30 | J9 |
| J4 | 23 | 22 | 19 | 29 | J10 |
| J5 | 15 | 14 | 13 | 28 | J11 |
| J6 | 37 | 36 | 38 | (27 reserved, no switch) | — |

### Endstop wiring

Each socket: pin 1 = signal, pin 2 = GND. On the Omron switch, the **NC** contact goes to pin 1 and **COM** to pin 2; **NO** is left unconnected. The Teensy uses its internal pull-up, so:

- not pressed → LOW
- pressed → HIGH
- **broken wire → HIGH**, so a loose wire stops the arm instead of letting it crash (fail-safe)

### TMC2209 notes

- **Microstepping** is set by MS1/MS2 (standalone): GND/GND = 8, MS1 high = 32, MS2 high = 64, both high = 16. Most joints run at 8; my J5 driver is at 16.
- Set each driver's current with its **Vref** potentiometer *before* connecting motors; never plug/unplug a motor while powered.
- J2's motor is small (0.4 A), so its current and speed matter most.

## Motors and drives

| Joint | Motor | Drive | Reduction | Steps/° |
|---|---|---|---|---|
| J1 | 17HS4401S | 100T / 16T belt | 6.25 | 27.778 |
| J2 | 17HS13-0404S-PG5 | 5.18:1 planetary × 80T / 16T belt | 25.9 | 115.111 |
| J3 | 17HS15-1504S1 | 100T / 16T belt | 6.25 | 27.778 |
| J4 | Casun 36SHD4001-11B | 60T / 16T belt | 3.75 | 16.667 |
| J5 | Casun 35SHD4310-12 | 32T / 16T belt | 2 | 17.778 (driver at 16 microsteps) |
| J6 | Casun 20SHD0003-20 | 32T / 16T belt | 2 | 8.889 |

Steps/° = 200 × microsteps × reduction ÷ 360.
