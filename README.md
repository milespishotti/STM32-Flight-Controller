# STM32 Flight Controller

A quadcopter flight controller built from scratch in C on an STM32F411RE, currently in
hardware bring-up. Firmware (sensor fusion, PID control, motor mixing, RC input, and a
custom DShot driver) is complete and bench-verified with a logic analyzer. First hover
is the next milestone.

> **Status:** Firmware complete, running on a dev-board bring-up rig. Custom PCB is the
> next step once flight is proven on the dev board.

## Why this project

Most "flight controller" projects wrap Betaflight or ArduPilot on off-the-shelf hardware.
This one is written register-up: the sensor driver, receiver protocol parser, motor
protocol driver, sensor fusion, and control loop are all implemented from scratch against
the STM32 HAL, with no flight-stack dependency. The goal is to understand, and be able to
defend, every layer between "IMU register read" and "motor spins."

This project sits at the intersection of controls and embedded systems, which is where I
want to build my career. Writing the PID loops and sensor fusion myself, instead of tuning
someone else's black box, was the point: I wanted the practice of taking a system from raw
registers and interrupts up to closed-loop control, not just flying a drone.

## Hardware

| Layer | Component |
|---|---|
| MCU | STM32F411RE (Nucleo dev board, custom PCB in progress) |
| IMU | MPU6050 |
| Receiver protocol | CRSF (ELRS) |
| ESC protocol | DShot150 |
| Current build | Dev board + PDB/ESC/motor stack, wired up on a bench rig |

The physical build is staged by layer so each subsystem can be validated independently
before integration:
1. PDB + ESCs + motors, assembled
2. MPU6050 + mounting plate
3. Receiver + battery
4. Nucleo + mounting plate

## Firmware architecture

All source in `src/`, headers in `inc/`. Everything is written directly against STM32
HAL, with no RTOS and no third-party flight stack.

| Module | Responsibility |
|---|---|
| `MPU6050` | Low-level register read/config for the IMU |
| `Mahony` | Sensor fusion (Mahony filter, chosen over a full Kalman filter to fit the F411's compute budget) |
| `CRSF` | Custom bit-level parser for CRSF receiver telemetry/channel data over UART |
| `RC Input` | Converts raw CRSF frames into normalized channel data for the control loop |
| `DShot` | Custom DShot150 driver, drives all four motors |
| `FlightController` | PID control loops (rate/attitude) |
| `Motor Mixer` | Converts PID correction outputs into per-motor throttle values |
| `Safety` | Arming logic and throttle-gating, decides whether it's safe to send output to motors |
| `Logging` | Streams system state over UART to a host PC (SD card logging planned post-first-hover) |

### Control flow

```
CRSF (UART) → RC Input → ┐
                          ├─→ FlightController (PID) → Motor Mixer → DShot → ESCs
MPU6050 → Mahony (fusion)─┘
                          ↳ Safety gates final output
                          ↳ Logging streams state to host
```

## Verified so far

- **Full firmware stack:** CRSF parsing, sensor fusion, PID control, motor mixing,
  and DShot output are all implemented and running end to end.
- **DShot driver:** logic-analyzer-captured on all four channels simultaneously,
  confirming correct frame timing and protocol compliance.
- **CRSF parser:** logic-analyzer-verified against raw receiver UART input.
- **Power distribution system:** all four motors and ESCs, plus the Nucleo, are
  soldered to the PDB and confirmed spinning on command.
- **Control loop timing:** frequency/jitter characterization in progress.

## Notable bugs solved

**DShot checksum failure above throttle value 128**
The checksum routine only validated correctly for values under 128 due to a bug in the
computing loop, traced from a "motors ignore high throttle" symptom all the way down to
the logic-level signal before finding the fix.

**CRSF parser silently failing after ~20 seconds**
The original parser would desync roughly 20 seconds into a session. Root cause was frame
misalignment after a dropped byte. Fixed by rewriting the receive pipeline around a
circular DMA buffer with frame-start resynchronization, so the parser can recover
mid-stream after a lost frame instead of drifting permanently out of sync.

## Roadmap

- [ ] Replace new MPU6050 (current one is fried, bring-up paused)
- [ ] Characterize control loop frequency/jitter with logic analyzer
- [ ] First tethered hover test
- [ ] SD card logging (replacing UART host logging)
- [ ] Post-hover: Python tooling to summarize flight logs and generate PID tuning
      suggestions via an LLM agent
- [ ] Custom PCB design (replacing dev-board bring-up rig)

## Repo structure

```
.
├── src/    # implementation files
├── inc/    # headers
└── README.md
```

~30 commits of build history tracking the project from first UART read to full
sensor-fusion + PID + DShot integration.
