# TW2KA Arm Controller (web app)

A complete controller for the 6-axis arm in **one HTML file**. Open it in your browser, plug in the Teensy over USB, and you can jog, home, calibrate, move in X/Y/Z with inverse kinematics, and draw from G-code or images, while a live 3D model shows exactly what the arm is doing.

<img width="1407" height="830" alt="image" src="https://github.com/user-attachments/assets/326ef17f-c946-47f7-b909-463f7e00a337" />



## Requirements

- **Chrome or Edge on a computer.** The app talks to the Teensy with the Web Serial API, which these browsers support.
- The arm's firmware flashed on the Teensy 4.1: [`firmware/tw2k_firmware/tw2k_firmware.ino`](../firmware/tw2k_firmware/tw2k_firmware.ino)
- Nothing to install. On a phone, or without the Teensy, it runs as a **simulator**.

## Quick start

1. Download `tw2k_controller.html` and double-click it (it opens in your browser).
2. Click **Connect Teensy** and pick the Teensy's port.
3. Press **Enable all**, then **Home all**.
4. Jog with **−/+** or drag the sliders. The 3D model follows the real arm.

⚠️ Keep a hand near **STOP** (or press **Esc**) and the motor power switch, especially the first time.

## Features

### Live 3D view
- **Solid arm** = where you're telling it to go. **Faint arm** = where the motors really are.
- Drag to rotate, scroll or pinch to zoom.
- Built from the arm's real DH parameters, so what you see matches the geometry.

### Joint control
- A slider per joint to go to an exact angle.
- **Press and hold − / +** to jog a joint, like a classic robot GUI.
- **Current pose** readout: tool X/Y/Z, roll/pitch/yaw, and all six joint angles.

<img width="1407" height="830" alt="image" src="https://github.com/user-attachments/assets/d4522932-8af4-4c5b-9c73-3437dbb2c91b" />



### Cartesian control (inverse kinematics)
<img width="578" height="784" alt="image" src="https://github.com/user-attachments/assets/803b9b60-f02a-41e4-8f77-8dfbfb690e00" />

- Move the tool in **X, Y, Z** and set **roll, pitch, yaw**; the app solves all six joint angles.
- **Hold tool angle** mode (uses all 6 joints) or **Position only** mode (J1–J3 move, the wrist stays still).
- Blocks sudden **wrist flips**, respects joint limits, and has a **table-height limit** so the tool never goes into the table.

### Motors
- **Motor enable**: turn each joint's motor on or off.
- **Motor direction**: reverse a joint whose real motion is opposite to the 3D model.
- **Hardware test**: pulse a motor 1600 steps forward and back to check wiring.
<img width="1003" height="455" alt="image" src="https://github.com/user-attachments/assets/2e173fe7-34ff-48d0-b8c9-99d5d494e554" />


### Homing and calibration
<img width="681" height="377" alt="image" src="https://github.com/user-attachments/assets/c578adc9-97fd-4d55-b19a-5aa76283698a" />

- **Home all** or one joint at a time, with live endstop indicators.
- Set each joint's **homing direction**, **zero position** (*zero here*), **soft limits** (*min/max here*, *endstop only*) and **steps per degree** (*fix from measurement*).
- Everything is saved on the Teensy, so it's remembered after a power-cycle.

### Drawing
<img width="716" height="859" alt="image" src="https://github.com/user-attachments/assets/f834958f-0b94-424c-a948-0bd00d6c8d30" />

- Run **G-code** (G0/G1/G2/G3, M3/M5) with straight-line motion and pause / resume / stop.
- **Find drawing area**: shows the largest area the pen can reach (yellow box in 3D).
- **Fit G-code to area**, and **Image → G-code** for SVG, PNG and JPG (outline or hatch fill).
- **Check path** tests every point first and tells you the exact line that can't be reached.

## How it's built

- Plain HTML, CSS and JavaScript in a single file, with [three.js](https://threejs.org/) (r128) for 3D.
- Forward kinematics from the DH table; inverse kinematics with a damped-least-squares solver.
- Talks to the firmware with simple text commands like `M j1 … j6`, `H`, `J n +1`. The full list is in [docs/04-firmware.md](../docs/04-firmware.md).
- Code map and how to add features: [docs/05-controller-app.md](../docs/05-controller-app.md).

Built with **Claude (Anthropic)** as an AI coding partner, from my requirements, and tested and calibrated on the real arm.

## More docs

- First-time setup: [docs/06-calibration.md](../docs/06-calibration.md)
- Drawing guide: [docs/07-drawing.md](../docs/07-drawing.md)
- Troubleshooting: [docs/08-troubleshooting.md](../docs/08-troubleshooting.md)
