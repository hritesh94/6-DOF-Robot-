# TW2KA 6-DOF Robot Arm — custom PCB, firmware and browser controller

A 3D-printed 6-axis robot arm that homes itself, moves in joint or Cartesian space using inverse kinematics, and **draws** from G-code, SVG files or photos.

> 📹 

https://github.com/user-attachments/assets/a5f0eb35-01ec-4ff1-84e9-ad130a259d0c

## Final Result of What the Robot drew
<img width="720" height="1280" alt="WhatsApp Image 2026-09-27 at 4 41 34 PM" src="https://github.com/user-attachments/assets/d0164ba7-8a4f-489b-9efd-769c0aeec7fa" />






| | |
|---|---|
| **Mechanics** | Open-source [TW2KA](https://youtube.com/@tw2ka644) arm, 3D-printed on my own printer, assembled and modified |
| **Electronics** | Custom PCB (KiCad): Teensy 4.1 + 6× TMC2209 stepper drivers + 5 endstops |
| **Firmware** | Arduino/Teensyduino, AccelStepper, coordinated moves, homing, EEPROM calibration |
| **Controller** | Single HTML file: live 3D model (three.js), jogging, Cartesian IK, G-code drawing, image → G-code |
| **Status** | Version 1: working and drawing |

---

## What it can do

- **Jog** each joint (press and hold) and **home** J1–J5 on their endstops
- **Cartesian control**: move the tool in X/Y/Z/roll/pitch/yaw, or position-only with the wrist held still
- **Live 3D model** showing the commanded pose and the real motor position side by side
- **Calibrate from the app**: motor direction, homing direction, zero offset, soft limits and steps/degree per joint, all saved on the Teensy
- **Draw**: run G-code (G0/G1/G2/G3, M3/M5), find the reachable drawing area, fit drawings to it, and convert SVG / PNG / JPG images into G-code (outline or hatch)
- **Safety**: E-stop, endstop protection during moves, per-joint speed caps, table-height limit, wrist-flip blocking

## Repository layout

```
## Repository layout

```
6-DOF-Robot-/
├── README.md                       ← you are here
├── LICENSE
├── app/
│   └── tw2k_controller.html        ← the whole controller (open in Chrome/Edge)
├── firmware/
│   └── tw2k_firmware/
│       └── tw2k_firmware.ino       ← Teensy 4.1 firmware
├── hardware/
│   ├── BOM.md                      ← motors, drives, electronics
│   ├── cad/
│   │   ├── README.md
│   │   ├── pen_holder.3mf          ← my pen holder (print-ready)
│   │   ├── pen_holder_preview.png
│   │   └── Arduino robot arm files + step/   ← original TW2KA design files (credit: TW2KA)
│   │       ├── README.md
│   │       ├── Arm1/  Arm2/  Base/  ← printable parts
│   │       ├── Assembly step.STEP  ← full 3D assembly
│   │       ├── Hardware.txt        ← original hardware list
│   │       └── coupler stl file for planetary geared nema motor.3mf
│   └── pcb/
│       ├── README.md
│       ├── tw2ka_pcb.kicad_pro     ← open this in KiCad
│       ├── tw2ka_pcb.kicad_sch     ← schematic
│       ├── tw2ka_pcb.kicad_pcb     ← 4-layer board layout
│       ├── fp-lib-table
│       ├── 3d/
│       │   └── tw2ka_pcb.step      ← 3D model of the board
│       └── fabrication/            ← drill files (+ Gerbers)
├── docs/
│   ├── 01-build-log.md             ← the story: what I tried, what failed, why each feature exists
│   ├── 02-hardware.md              ← PCB, pin map, drivers, motors
│   ├── 03-kinematics.md            ← DH parameters, workspace, how the IK works
│   ├── 04-firmware.md              ← serial protocol, config, EEPROM
│   ├── 05-controller-app.md        ← code map of the HTML app, how to add features
│   ├── 06-calibration.md           ← first-time setup, step by step
│   ├── 07-drawing.md               ← G-code and image drawing
│   ├── 08-troubleshooting.md       ← problems I hit and how I fixed them
│   └── roadmap.md                  ← ROS 2, MoveIt, camera, learning plans
└── media/                          ← photos and videos
```
```

## Quick start

1. **Flash the firmware**: open `firmware/tw2k_firmware/tw2k_firmware.ino` in the Arduino IDE with Teensyduino, board **Teensy 4.1**, USB type **Serial**, install **AccelStepper**, upload.
2. **Open the app**: double-click `app/tw2k_controller.html` in **Chrome or Edge** on a computer (Web Serial is needed; phones run it as a simulator only).
3. **Connect → Enable all → Home all.**
4. Follow [docs/06-calibration.md](docs/06-calibration.md) once, then try Cartesian mode and drawing.

⚠️ Keep a hand near STOP (or press **Esc**) and your motor power switch, especially the first time.

## The story in short

1. My first arm used servo motors; I wanted something **precise**.
2. Builders like **Aaed Musa** and **Michael Reeves** inspired me, but precise arms often use BLDC motors with FOC drivers, far over my ₹10–20k budget.
3. **[Skyentific](https://youtube.com/@skyentific)** showed me a precise arm can use **stepper motors**, but his belts aren't sold in India.
4. I found **[TW2KA](https://youtube.com/@tw2ka644)**, and **[Matt.xyz++](https://youtube.com/@matt-04)** showed the same arm writing with G-code (and mentioned TMC drivers). I bought a 3D printer and printed every TW2KA part.
5. TW2KA uses an Arduino Mega + RAMPS + A4988. I moved to a **Teensy 4.1 + TMC2209s**. On a perfboard I killed two drivers and a motor (lesson: never unplug a motor while powered), so I learned **KiCad** and designed my own PCB.
6. Software: the **TW2KA GUI** only jogged the joints, and **Chris Annin's AR4 app** didn't move my arm correctly in X even with my DH parameters. So I **made my own app**.
7. Debugging on the real arm: J3 turning the wrong way, J5's steps per degree, wrist flips, J2 wiring. Then it drew its first SVG.

Full version: [docs/01-build-log.md](docs/01-build-log.md).

## How this was built (honest credits)

- **Mechanical design**: the open-source **TW2KA** arm ([YouTube](https://youtube.com/@tw2ka644)). I printed all parts on my own 3D printer, assembled the arm and modified parts in Fusion 360 (2–3 iterations, plus my own pen holder).
- **PCB**: designed by me in KiCad, soldered and brought up by me (after an earlier perfboard version).
- **Kinematics measurements**: I measured the link geometry axis-to-axis in Fusion 360; the DH table came from those measurements.
- **Software**: the firmware, inverse kinematics and controller app were written with **Claude (Anthropic)** as an AI coding partner, from my requirements, and then tested, debugged and calibrated by me on the real robot.
- **Inspiration and learning**:
  - [Skyentific](https://youtube.com/@skyentific) (stepper-driven 6-axis arm)
  - [Matt.xyz++](https://youtube.com/@matt-04) (TW2KA arm drawing with G-code, TMC drivers)
  - Chris Annin's **AR4** robot and software
  - **Aaed Musa** and **Michael Reeves** (robot builders who got me started)
  - **DIY Engineers** and **Electronoobs** on YouTube (how TMC stepper drivers work)
  - [Inverse kinematics basics playlist](https://www.youtube.com/watch?v=MYJIEiz2DKg&list=PLY6RHB0yqJVasji1rwZAGYirD8zW1ipj-)
- **Libraries**: [AccelStepper](https://www.airspayce.com/mikem/arduino/AccelStepper/) by Mike McCauley, [three.js](https://threejs.org/).

## License

The firmware and app in this repository are released under the MIT License (see `LICENSE`). The TW2KA mechanical design belongs to its creator; check their license before redistributing any of their CAD files.
