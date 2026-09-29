# Kinematics

## Measurements (Fusion 360, axis-to-axis, mm)

| Pair | Distance | Angle | Notes |
|---|---|---|---|
| Base face → J2 | 135.30 | — | Perpendicular from the mounting face |
| J1 ↔ J2 | 37.50 | 90° | Shoulder offset |
| J2 ↔ J3 | 160.00 | 0° | Upper arm, parallel axes |
| J3 ↔ J4 | 15.00 | 90° | Elbow offset |
| J3 ↔ J5 | 139.21 | 0° | Elbow to wrist centre (diagonal) |
| J4 ↔ J5, J5 ↔ J6 | 0 | 90° | Intersecting → **spherical wrist** |

The 139.21 mm diagonal splits into a 15 mm offset and a 138.40 mm forearm: √(139.21² − 15²) = 138.40.

## DH table (standard convention)

| i | a (mm) | α | d (mm) | θ |
|---|---|---|---|---|
| 1 | 37.5 | −90° | 135.3 | θ1 |
| 2 | 160 | 0° | 0 | θ2 − 90° |
| 3 | 15 | −90° | 0 | θ3 |
| 4 | 0 | +90° | 138.4 | θ4 |
| 5 | 0 | −90° | 0 | θ5 |
| 6 | 0 | 0° | 48 | θ6 |

d6 = 48 mm is wrist centre to tool flange (measured 47–50 mm). A pen's own offset is set separately in the drawing card.

### Zero pose

All joints at 0° means: upper arm vertical, forearm horizontal pointing +X, wrist straight, flange facing +X. Calibration makes the real arm match this pose (see [06-calibration.md](06-calibration.md)).

## Forward kinematics

Each joint contributes one DH transform; multiplying them gives the flange pose:

```
T0_6 = A1(θ1) · A2(θ2) · A3(θ3) · A4(θ4) · A5(θ5) · A6(θ6)
```

## Inverse kinematics (how the app does it)

The app uses a **numerical** solver (damped least squares) rather than closed-form equations:

1. Start from the arm's current joint angles.
2. Compute the flange pose with FK and the error to the target (position in mm, orientation in radians).
3. Build the 6×6 Jacobian from the joint axes and positions.
4. Take a small damped step `Δq = Jᵀ (J Jᵀ + λ² I)⁻¹ e`, clamp to joint limits, repeat until the error is under 0.01 mm.

Why numerical: it works with **any** DH table, so it kept working while signs and offsets were being corrected, and starting from the current pose picks the solution closest to where the arm already is.

Extra rules added from real-world testing:
- **Wrist-flip blocking**: a step that would swing any joint more than 25° at once is refused. Near J5 = 0 the wrist has two equivalent solutions (J4±180°, −J5, J6±180°); the flip is fine on paper but dangerous on the real arm.
- **Position-only mode**: solves X/Y/Z with J1–J3 only and keeps J4–J6 fixed.
- **Locked joints**: disabled motors are excluded from the solve.
- **Table limit**: solutions that put the flange, elbow or wrist below the table are rejected.

### Closed-form alternative (to do myself)

Because the wrist is spherical, the IK can also be solved analytically: find the wrist centre, solve J1 with atan2, solve J2/J3 as a 2-link planar problem with the law of cosines, then solve J4–J6 from the remaining rotation. Coding this myself is on the [roadmap](roadmap.md).

## Workspace (approximate)

- Max horizontal reach from the J1 axis: 37.5 + 160 + 139.21 ≈ 337 mm (+ tool)
- Max height above the base face: 135.3 + 160 + 139.21 ≈ 435 mm (+ tool)
- The drawing card's **Find drawing area** computes the real usable area for a given tool angle.
