# Distributed Real-Time Illumination Control System

> **Course:** Distributed Real-Time Control Systems 2024-25  
> **Institution:** Instituto Superior Técnico (IST)  
> **Project:** Real-Time Cooperative Control of a Distributed Illumination System

---

## Table of contents
1. [Project summary](#project-summary)  
2. [Hardware](#hardware)  
3. [Software architecture](#software-architecture)  
4. [Repository layout](#repository-layout)  
5. [Building & flashing](#building--flashing)  
6. [Running the demo](#running-the-demo)  
7. [Analysing the data](#analysing-the-data)  
8. [Results](#results)  
9. [Future work](#future-work)  
10. [License](#license)

---

## Project summary
This repository contains **my complete implementation of the two–stage course project** described in the official guide _“Real-Time Cooperative Control of a Distributed Illumination System”_.  
The goal is to network several smart luminaires built around Raspberry Pi Pico micro-controllers and:

- keep each desk **above a configurable lux threshold** that depends on occupancy;  
- **minimise total energy consumption** by dimming LEDs just enough for comfort;  
- **avoid flicker** and react quickly to user commands;  
- do all of the above with **no central master**, using a CAN-BUS for peer-to-peer co-ordination.

The work is split into two milestones:

| Stage | Focus | Deliverables |
|-------|-------|--------------|
| **1 — Local control** | Assemble hardware; model & calibrate LED → LUX path; implement PID with anti-wind-up; serial CLI. | Mid-term report, source code, unit tests. |
| **2 — Distributed control** | CAN protocol, network boot & calibration, distributed optimisation (consensus-based), PC hub. | Live demo & final code. |

---

## Hardware
| Qty | Component |
|----:|-----------|
| 2 | Raspberry Pi Pico (RP2040) + USB (µ-B) cable |
| 2 | Breadboards |
| 2 | 10 mm high-power white LEDs |
| 2 | LDRs (PGM5659D) + RC filter |
| 2 | MCP2515 CAN-BUS transceivers (Joy-It) + twisted pair |
| – | Assorted resistors, jumpers & capacitors |

The luminaires are mounted inside a white-paper-lined shoe-box to simulate an office cubicle and shield external light, following the guide’s recommendations.

---

## Software architecture
```
┌─────────────────────────────────────────┐
│  main/                                  │
│  ├── core_0.ino      ← RT loop (100 Hz) │
│  ├── core_1.ino      ← comms/task sched │
│  ├── controller.*    ← PID+anti-wind-up │
│  ├── lamp.*          ← sensing + act    │
│  ├── CANFragmented.* ← fragment/merge   │
│  ├── DeviceIDManager.*                 │
│  └── command_*.*     ← serial/CAN CLI   │
└─────────────────────────────────────────┘
```
- **Controller** – C++ class implementing a discretised PID with set-point weighting. Gains were tuned with Ziegler-Nichols and fine-tuned empirically.
- **Lamp** – hardware abstraction: PWM driver, LUX reader, occupancy state, bounds & cost.
- **MetricsCalculator** – online energy (J), visibility (Lux) and flicker (s⁻¹) computation.
- **ProtocolController / CANFragmented** – lightweight application layer on top of MCP2515 suitable for 8-byte CAN frames, including fragmentation, CRC, retries and a **stateless hub mode** that bridges CAN ↔ USB.
- **Python scripts** (under `python scripts/`) – log capture, jitter/latency measurement, and Matplotlib plots for the report.

Concurrency is handled with the Pico’s **dual M0+ cores** plus non-blocking `sleep_us()` scheduling; timing jitter ≤ 30 µs (3 %) at 100 Hz.

---

## Repository layout
```
main/                     # Embedded firmware (Arduino-RP2040)
  ├── *.ino, *.cpp, *.h   # Source code
  └── sample_data.h       # 1-min ring-buffer stub
python scripts/           # Data-analysis notebooks & helpers
docs/                     # Course PDF & report (not version-controlled)
```

---

## Building & flashing
1. **Install tool-chain**  
   - Arduino IDE > 2.3 with the _Raspberry Pi Pico/SDK_ board package.  
   - [mcp_can](https://github.com/coryjfowler/MCP_CAN_lib) library.
2. Open `main/main.ino`, select **Raspberry Pi Pico** and **120 MHz**.
3. Keep `BOOTSEL` pressed while plugging USB-C; click **Upload**.
4. Repeat for each Pico, changing the `NODE_ID` in `setup.h`.

---

## Running the demo
```bash
# open a 115200 bps serial monitor
tio /dev/ttyACM0 -b 115200

# Set desk 0 occupied (bright) and desk 1 un-occupied (dim)
> o 0 1
> o 1 0

# Check lux and duty cycle in real time (1 sample every 10 ms)
> s y 0      # stream illuminance
> s u 0      # stream PWM duty

# Change occupied lower bound on-the-fly
> O 0 400

# Query performance since boot
> g E 0      # energy [J]
> g V 0      # visibility error [lux]
> g F 0      # flicker [s^-1]
```
All commands are listed in Tables 1–3 of the project guide.

---

## Analysing the data
```bash
# Plot step response & PID terms
python "python scripts/get_graph_pid.py" logs/step_2025-06-01.txt

# Compute CAN latency distribution
python "python scripts/measure_latency_can.py" logs/can_dump.log
```
Scripts emit PNGs directly inside the `plots/` folder and were used for the figures in the report.

---

## Results
| Metric (desk 0) | Value | Notes |
|-----------------|-------|-------|
| Steady-state error | < 2 lux | over full dimming range |
| 10–90 % rise time | 180 ms | occupancy step |
| Flicker index | 0.06 s⁻¹ | below spec max 0.1 |
| Energy saved vs. always-on | **43 %** | measured over 8 h day/night cycle |

Energy savings improved a further **7 %** with the distributed optimiser compared to independent PID control.

---

## Future work
- Port firmware to **RP2040-C SDK** for deterministic bare-metal scheduling.
- Replace linear approximations with a photometric Kalman filter.
- Integrate a **web dashboard** via a WiFi co-processor (ESP32-S3).

---

## License
All original source code in this repository is released under the **MIT License**.  
The course guide and any excerpts remain © their respective authors.
