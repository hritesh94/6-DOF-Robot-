# Build log — how this robot came together

This is the honest story of version 1: what I tried, what failed, and why each feature in the software exists. Every feature was added because the real robot needed it.

> 📷 Suggested photos for this page: printed parts laid out, half-assembled arm, the PCB before and after soldering, first homing, first drawing.

## 1. Why this robot

My first robot arm used **servo motors**. I didn't like it: I thought I couldn't learn inverse kinematics on it (I was wrong about that), and I wanted a **precise** robot.

I'd followed builders like **Aaed Musa** and **Michael Reeves** for a long time. Precise arms like theirs often use **BLDC motors with FOC control** (ODrive-style drivers), but they're expensive and every motor needs its own driver. My budget was **₹10–20k**, not ₹1 lakh.

Then **[Skyentific](https://youtube.com/@skyentific)**'s 6-axis arm showed me a precise arm can be built with **stepper motors**. But its belts aren't available in India, and getting them made would cost about ₹2,000 each.

I kept searching and found **[TW2KA](https://youtube.com/@tw2ka644)**. Its videos didn't show much of what it could do, so I was stuck between TW2KA and Skyentific, until **[Matt.xyz++](https://youtube.com/@matt-04)** showed the TW2KA arm **interpreting G-code and writing**. That decided it. He also mentioned using **TMC drivers** for better control.

## 2. Printing and assembling

I **bought a 3D printer** and printed every part TW2KA open-sourced, then assembled the arm with the motors and belt/planetary drives listed in [the BOM](../hardware/BOM.md). Along the way I made **2–3 CAD iterations in Fusion 360**, and later designed my own **pen holder**.

## 3. Electronics: the hard way

TW2KA uses an **Arduino Mega 2560 + RAMPS + A4988** drivers. That's good enough, but I wanted more precise control, so I raised my budget for a **Teensy 4.1** and **TMC2209** drivers. I learned how TMC drivers work from **DIY Engineers** and **Electronoobs** on YouTube, and by asking AI.

I first built the circuit on a **perfboard**, learning why each part is there: 100 µF capacitors on the motor supply, and why they should be rated 35 V (I first used 25 V ones from the local market at ~20–21 V, then switched to 35 V).

Then I made a mistake I didn't know about: **I unplugged a motor while the driver was powered**. That can destroy a TMC driver. I lost **two drivers and a motor**, and waited 1–2 months for replacements. Going back to the RAMPS board didn't work either, because it couldn't take more than about 15–16 V.

So I decided to learn it properly. I learned **KiCad**, designed my own PCB from what I'd learned about schematics, and waited about a month for it to be manufactured. I soldered it, ordered six new TMC2209s, and **every motor moved**. Details: [02-hardware.md](02-hardware.md).

## 4. Software: two apps that didn't work

- **TW2KA's own GUI and code**: I remapped the pins for my board (with AI help), but it could only **jog** the joints.
- **Chris Annin's AR4 app**: I found through a Matt.xyz++ short that he uses it, and it also runs on a Teensy with steppers. I set my pins and DH parameters, and the forward kinematics even checked out (the tool sat exactly at X = 175.9 mm), but **moving along X didn't work**.

## 5. Building my own: measuring the kinematics

I knew roughly how IK works and that it needs the robot's geometry, so I measured every axis-to-axis distance and angle in **Fusion 360** (see [03-kinematics.md](03-kinematics.md)). From these measurements the **DH table** was built, and it showed the arm has a **spherical wrist** (J4, J5, J6 axes meet at one point), which is what makes solving the IK practical.

## 6. Building my own app with an AI partner

So I decided to **make my own app**. A full 6-DOF arm was honestly beyond my scope (most people start inverse kinematics with 2 links), and I only wanted one thing first: **move straight in X, Y and Z**. I don't write code at this level yet, so I built it with **Claude (Anthropic)** as a coding partner, asking for every step so the control stayed in my hands. My part was defining what the robot needed, giving it my measurements, pin maps and drive ratios, testing every version on the real robot, and reporting exactly what went wrong. Its part was writing the firmware, IK and app.

The software went through many versions. Each feature below exists because of a real problem:

| Problem on the real robot | What was added |
|---|---|
| Needed to see what the arm *should* do vs what it *does* | Live 3D model: solid = commanded, faint = real motor position |
| Motors didn't move, or moved only a few steps | Hardware **test** buttons, visible serial log, jog speeds in steps/s |
| Angles were 6× wrong, homing stopped halfway | Real **gear ratios** from my drive table (e.g. J2 = 5.18:1 planetary × 80T/16T belt) |
| J3 homed away from its switch | Per-joint **homing direction** button, saved on the Teensy |
| Moving +X made the tool dive down | J3's motor turned opposite to the model → per-joint **motor reverse** |
| Wrist spun 180° when moving down | The IK found a "flipped" wrist solution → **wrist-flip blocking** and **position-only** mode |
| J5 moved ¼ as far as shown | Its driver was on a different microstep setting → **steps/degree** editable and saved |
| Limits stopped the arm too early | **Zero here**, **min/max here**, **endstop only** limits |
| J2 grinding | Too fast for its small motor → per-joint **speed caps**; also found a wiring fault |
| Tool pushed into the table | **Table-height limit** for Cartesian and drawing moves |
| Wanted to draw | **G-code runner**, reachable-area finder, **image/SVG → G-code** |

## 7. First drawings

With calibration done (directions, zeros, steps/degree), the arm follows straight lines in Cartesian space and draws SVG outlines cleanly.

> 📷 Put the first drawing photo and a video link here.

## What I learned

- Making the **real robot match the math** is harder than the math: signs, zero offsets and ratios all have to agree.
- **Open-loop steppers** need careful speeds and good wiring; a stall silently loses position.
- **Test in small steps** and watch the serial log.

## What's next

See [roadmap.md](roadmap.md): deriving and coding the IK myself, and moving the arm to ROS 2 + MoveIt.
