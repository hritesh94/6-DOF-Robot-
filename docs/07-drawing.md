# Drawing

> 📷 Suggested: the pen holder, the reachable-area box in the 3D view, a generated path preview, the finished drawing.

## Setup

1. Home the arm and pick a tool angle (mine: flange facing +X, pen pointing down).
2. Enter the pen offset: **pen tip below the flange** and **ahead of the flange** (mm).
3. Jog the pen tip onto the paper where the drawing should be and press **Set drawing origin here**.
4. Press **Find drawing area**. It flood-fills a 5 mm grid on the paper plane from the origin and keeps points the pen can reach on the paper and lifted, within joint limits, without wrist flips and above the table. The largest reachable rectangle appears as a yellow box.

## G-code

Supported: G0/G1, G2/G3 (I/J arcs), G90/G91, G20/G21, M3/M5.

- Pen **down** when Z ≤ 0 or after M3; **up** otherwise. Files with no Z and no M3/M5: G0 = travel, G1/G2/G3 = draw.
- **Fit G-code to area** scales and offsets any file into the reachable box.
- **Check path** splits every move into ≤ 2 mm steps, solves IK for all of them and reports the exact G-code line that can't be reached.
- **Run** sends one step at a time and waits for the Teensy to report arrival, so lines stay straight.

## Image → G-code

- **SVG**: every path, line, rect, circle, ellipse and polygon is sampled along its length (best for line art, logos and text).
- **PNG/JPG**: **Outline** traces dark regions with marching squares; **Hatch fill** fills them with horizontal lines. Adjust the darkness threshold.
- Strokes are simplified (Ramer–Douglas–Peucker, 0.15 mm), ordered nearest-first to reduce travel, sized to the reachable area, and checked automatically.

## Tips

- Start small (30–50 mm) near the origin.
- Hatch at 1.5–2 mm so lines don't merge.
- Accuracy depends on calibration: directions, zeros and steps/degree.
