# Roadmap

Version 1 works. Next, I want to understand and own every layer.

## Learning (next)

- [ ] Derive the closed-form IK for this arm on paper and code it myself in Python; compare with the app's numerical solver
- [ ] ROS 2: go beyond nodes and publishers/subscribers
- [ ] Write a **URDF** of the arm and view it in **RViz**
- [ ] Simulate it in **Gazebo**
- [ ] Plan motion with **MoveIt 2**
- [ ] A ROS 2 driver node that talks to this firmware over serial

## Robot improvements

- [ ] PCB revision (small fixes noted in `hardware/pcb/`)
- [ ] Continuous motion (look-ahead) instead of stop-and-go drawing steps
- [ ] Encoders or TMC2209 UART/StallGuard for stall detection
- [ ] Camera for vision-guided tasks (pick and place, drawing what it sees)
- [ ] Later: learning-based control / reinforcement learning experiments in simulation
