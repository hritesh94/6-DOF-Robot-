# Original TW2KA arm files

These are the **original design files of the TW2KA 6-axis robot arm**, made by the **TW2KA** YouTube channel. I printed my arm from these files.

- 📺 Build video: https://youtu.be/wI4Jh-T0Tlo
- 📺 Channel: https://youtube.com/@tw2ka644

> **All credit for this design goes to TW2KA.** I've included the files here so everything needed for this build is in one place. They're not my work, and they're shared as he published them. If you use them, please credit TW2KA and support his channel.
<img width="1178" height="671" alt="Picture2" src="https://github.com/user-attachments/assets/ab567158-314a-485e-a387-ddc534957427" />
<img width="828" height="790" alt="Picture1" src="https://github.com/user-attachments/assets/3d66f4dd-ff08-4707-9ff2-52b5ec953eab" />
<img width="1021" height="701" alt="Picture 3" src="https://github.com/user-attachments/assets/b3f54908-58dc-497c-a845-183a86186295" />



| File / folder | What it is |
|---|---|
| `Arm1/` | Parts for the first arm section |
| `Arm2/` | Parts for the second arm section |
| `Base/` | Parts for the base |
| `Assembly step.STEP` | Full 3D assembly of the arm (opens in Fusion 360, FreeCAD, SolidWorks, Onshape) |
| `Hardware.txt` | Hardware list from the original design (screws, bearings, belts, etc.) |
| `coupler stl file for planetary geared nema motor.3mf` | Coupler for the planetary-geared NEMA 17 motor (used on J2) |
| `Robot parts v1.PNG` | Overview of all printed parts |
| `Picture1.PNG`, `Picture2.PNG`, `Picture 3.PNG` | Reference pictures of the arm |





## How I used these files

1. Printed every part from `Arm1/`, `Arm2/` and `Base/` on my own 3D printer.
2. Bought the hardware listed in `Hardware.txt`, and the motors and belts listed in [BOM.md](../../BOM.md).
3. Assembled the arm using `Assembly step.STEP` and the pictures as a guide.

## What I changed

I kept TW2KA's mechanical design and changed only a few things:

- **Electronics:** instead of the original Arduino Mega + RAMPS + A4988 setup, I use my own PCB with a **Teensy 4.1 and six TMC2209** drivers (see [hardware/pcb](../../pcb/)).
- **Pen holder:** my own part for drawing, in [pen_holder.3mf](../pen_holder.3mf).
- **Small CAD tweaks:** 2–3 iterations in Fusion 360 so the arm runs better. These will be uploaded separately in [hardware/cad](../), so the original files here stay untouched.

For the full build story, see [docs/01-build-log.md](../../../docs/01-build-log.md).
