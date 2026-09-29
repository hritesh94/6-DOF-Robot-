# Controller app

File: `app/tw2k_controller.html` — one self-contained HTML file (HTML + CSS + JavaScript). It loads **three.js r128** from a CDN and talks to the Teensy with the **Web Serial API**, so it needs **Chrome or Edge** on a computer. On a phone it runs as a simulator.

## Code map

The script is divided by banner comments (`// ===================== NAME =====================`). Search for these to jump around:

| Section | What's inside |
|---|---|
| **CONFIG + KINEMATICS** | `DH` table, `LIMITS`, `fkFrames()` forward kinematics, `ik()` damped-least-squares solver, `ikRobust()` |
| **APP STATE** | Current commanded joints `q`, reported joints `qActual`, mode, Cartesian target, rotation helpers, `poseOf()` |
| **UI BUILD** | Joint sliders, Cartesian sliders, nudge-step buttons, DH editor table |
| **IK in Cartesian mode** | `solveCart()`: solves from the current pose, blocks wrist flips, table limit, position-only mode |
| **3D VIEW** | three.js scene, arm drawing from DH frames (`drawArm()`), orbit/zoom controls |
| **REFRESH** | `refresh()`: redraws the arm and ghost, updates sliders and the pose readout |
| **SERIAL (Web Serial API)** | Connect/disconnect, `write()`, `readLoop()`, move streaming, STOP, speed |
| **PER-JOINT ENABLE + TEST** | Motor on/off buttons, direction reverse buttons, hardware test buttons |
| **HOMING / ENDSTOPS** | Homing buttons, homing direction, switch indicators, **Zero and limits** card, steps/degree fix, `handleLine()` parser for all replies |
| **HOLD-TO-JOG** | Press-and-hold jogging |
| **G-CODE DRAWING** | G-code parser, path builder, IK path check, 3D preview, runner, reachable-area finder, image → G-code (SVG tracing, outline, hatch) |

## Data flow

```
sliders / IK / G-code ──► q (commanded) ──► "M j1..j6" ──► Teensy
                                                              │
3D faint arm ◄── qActual ◄── "P j1..j6" (10 Hz while moving) ◄┘
```

## Adding a feature

1. **New firmware command**: add a branch in `handle()` in the firmware, and a reply line if the app needs state back.
2. **Parse the reply**: add a `line.startsWith('XYZ ')` case in `handleLine()`.
3. **UI**: add a `<section class="card">` in the HTML and wire its buttons in a new banner section.
4. **Kinematics changes**: only touch the **CONFIG + KINEMATICS** section; everything else uses `fkFrames()` / `ik()`.

## Notes

- The app keeps nothing in the browser between sessions; all calibration is saved on the Teensy.
- `LIMITS` in the app are overwritten by the Teensy's `LIM` report on connect.
