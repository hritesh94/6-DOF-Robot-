# Control PCB (KiCad)

My own control board for the TW2KA arm: a **Teensy 4.1** driving **six TMC2209** stepper drivers, with endstop inputs. Designed in **KiCad 10**.

| | |
|---|---|
| Size | 102 × 100 mm |
| Layers | 4 (F.Cu, In1.Cu, In2.Cu, B.Cu) |
| MCU | Teensy 4.1 (socketed) |
| Drivers | 6 × TMC2209 modules (socketed) |
| Per driver | STEP, DIR, EN to the Teensy; UART RX/TX lines routed (J1–J6_RX/TX); MS1, MS2 and SPREAD selectable |
| Endstops | 5 inputs (SW_1–SW_5) for J1–J5 |
| Power | Motor supply via terminal block and fuse, 100 µF capacitor per driver, separate logic supply (VIO) |
<img width="720" height="1280" alt="WhatsApp Image 2026-09-28 at 1 49 23 PM" src="https://github.com/user-attachments/assets/f4d70b1a-bb72-41bc-974b-7ce2a27cd135" />
<img width="806" height="763" alt="pcb" src="https://github.com/user-attachments/assets/22837841-0a95-4729-a996-dbfb7aa9e764" />


https://github.com/user-attachments/assets/b51a67fe-33f5-430a-8fab-c27dd11908fc


Pin map and endstop wiring: [docs/02-hardware.md](../../docs/02-hardware.md).

## Files

```
hardware/pcb/
├── tw2ka_pcb.kicad_pro     ← open this in KiCad
├── tw2ka_pcb.kicad_sch     ← schematic
├── tw2ka_pcb.kicad_pcb     ← board layout
├── fp-lib-table            ← project footprint library table
├── Tmc2209_pin_socket.pretty/  ← custom TMC2209 socket footprint (to add)
├── 3d/
│   └── tw2ka_pcb.step      ← 3D model of the assembled board
└── fabrication/
    ├── tw2ka_pcb-PTH.drl   ← plated drill file
    ├── tw2ka_pcb-NPTH.drl  ← non-plated drill file
    └── gerbers/            ← Gerber files for the board house (to add)
```

Opening the board works without the footprint libraries (footprints are stored inside the `.kicad_pcb`). To **edit** footprints you also need the `Tmc2209_pin_socket.pretty` folder and the Teensy footprint library (`teensy.pretty`, available on GitHub).

## Ordering the board

Upload the Gerbers and both drill files from `fabrication/` to a PCB manufacturer (JLCPCB, PCBWay, etc.) and choose **4 layers**.

## Lessons from the perfboard version

- Put a 100 µF capacitor rated **35 V** (not 25 V) on each driver's motor supply.
- **Never plug or unplug a motor while the driver is powered.** It can destroy a TMC2209, which is how I lost two drivers and a motor.

## Next revision

- [ ] [Describe the small tweak you want to make]
