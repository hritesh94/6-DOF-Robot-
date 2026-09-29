# First-time calibration

Do this once; everything is saved on the Teensy. Keep a hand near STOP (Esc) and the motor power switch.

## 1. Check the hardware

1. Connect, press **Enable all**.
2. Press each **test** button (J1–J6). Each motor should turn 1600 steps forward, then back.
   - Same direction twice → DIR wire problem.
   - Shuddering → a coil wire or connector problem.

## 2. Motor directions

Hold each joint's **+** button and compare the real arm with the 3D model:

- J2 + → upper arm tilts **forward**
- J3 + → forearm tilts **down**
- J4–J6 + → watch the coloured drum turn in the 3D view

If a real joint moves opposite to the model, tap its **Motor direction** button. (On my arm J3 and J5 needed reversing.)

## 3. Homing directions

Press **Home** on one joint at a time. If it drives away from its switch: **Stop homing**, jog to see which button moves it toward the switch, set **toward** to that button, and home again.

## 4. Steps per degree

Jog a joint about 90° in the app and measure the real movement. If they differ, press **fix from measurement** and enter both numbers, then re-home that joint. (My J5 was 2× off because its driver runs at 16 microsteps.)

## 5. Zeros

Home all, then for J3, J4 and J5: jog until the real arm matches the model's zero pose (forearm level, wrist square, flange straight and facing forward) and press **zero here**. Check with **Go to zero pose**.

## 6. Limits

For the switch side, press **endstop only**. For the other side, type a wider value, jog slowly to the physical end, back off a little and press **max here** / **min here**.

## 7. Table and tool

- Set **Lowest flange height above the table** (Cartesian tab) to your pen length plus a few mm.
- d6 (wrist centre → flange) is 48 mm in the DH table.
