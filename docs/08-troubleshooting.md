# Troubleshooting (problems I actually hit)

| Symptom | Cause | Fix |
|---|---|---|
| App connects but nothing moves | Old sketch on the Teensy, or motors disabled | Flash the right firmware; press **Enable all** |
| Jog seems to do nothing | Gear ratio was 1, so a 1° nudge was 4 steps | Set real gear ratios / steps per degree |
| Angle jumps after releasing jog | Display froze while the joint was still decelerating | App follows the real position while coasting |
| Homing stops halfway | Search distance used wrong steps/degree | Correct steps/degree |
| Joint homes away from its switch | Wrong homing direction | **toward** button in the homing card |
| +X makes the tool dive | J3 motor turns opposite to the model | Reverse J3 in **Motor direction** |
| Wrist spins 180° going down | IK switched to the flipped wrist solution at J5's limit | Wrist-flip blocking, **Position only** mode |
| "J5 reached its limit" too early | Wrong steps/degree put the switch in the wrong place | Fix steps/degree, **endstop only** |
| Joint moves ~¼ or ½ of what the app shows | Driver on a different microstep setting | **fix from measurement** |
| Moves blocked after pressing STOP | STOP latch wasn't cleared by Enable | Fixed: enabling clears it; app now warns |
| J2 grinds | Too fast for a 0.4 A motor / tangled wiring | Speed cap (12°/s), check wiring and belt |
| Tool presses into the table | Cartesian moves could go below the surface | **Lowest flange height** limit |

General rule: after any grinding or stall, **re-home** (open-loop steppers lose position silently).
