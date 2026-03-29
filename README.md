# Unified V2X Framework for Emergency Lane Clearing and Contactless Toll Collection

This repository presents a V2X-based intelligent traffic management system that integrates emergency vehicle lane clearing and contactless toll collection within a single roadside unit (RSU). The system is implemented using SUMO, OMNeT++, and Veins, and leverages IEEE 802.11p (DSRC) communication.

---

## Overview

Urban traffic congestion delays emergency response due to the lack of coordinated lane clearance. At the same time, toll plazas introduce additional delays. This work addresses both challenges using a unified RSU-based approach that:

* Detects emergency vehicles using Basic Safety Messages (BSM)
* Issues real-time lane change commands via TraCI
* Processes toll transactions simultaneously
* Prevents spoofing through whitelist validation

---

## Key Contributions

* Unified RSU application handling both lane clearing and tolling
* Reduction of unnecessary lane changes by up to 96.7%
* Constant detection latency of 5 ms using V2X communication
* 100% toll accuracy with zero false exemptions under spoofing attacks
* Evaluation on a real-world Bengaluru OSM road network

---

## System Architecture

The system consists of three layers:

1. Vehicle Layer
   Emergency vehicles broadcast PSID = 1 BSM messages. Civilian vehicles receive and act on RSU instructions.

2. RSU Layer
   Performs emergency detection, lane management, toll computation, and security validation.

3. Simulation Layer

   * SUMO for traffic simulation
   * OMNeT++ and Veins for network simulation
   * TraCI for real-time interaction

---

## Results Summary

| Metric            | Baseline  | Proposed V2X |
| ----------------- | --------- | ------------ |
| Lane Changes      | Up to 30  | 1            |
| Reduction         | —         | 96.7%        |
| Detection Latency | 26–177 ms | 5 ms         |
| Toll Accuracy     | Variable  | 100%         |
| Packet Loss       | —         | 0%           |

---

## Repository Structure

```bash
veinss/
├── sumo/              # Road network and traffic configurations
├── omnet/             # OMNeT++ simulation files
├── veins/             # RSU logic and V2X modules
├── results/           # Output data and graphs
├── figures/           # Images used in the paper
├── paper/             # Final paper PDF
```

---

## Setup and Execution

1. Install the following dependencies:

   * SUMO
   * OMNeT++
   * Veins

2. Clone the repository:

```bash
git clone https://github.com/ruubhagat/V2X-Lane-Clearing-Toll-System.git
cd V2X-Lane-Clearing-Toll-System
```

3. Run the simulation:

```bash
./run.sh
```

---

## Security Mechanism

The system includes a whitelist-based validation mechanism that verifies emergency vehicle identity using vehicle class information, preventing false PSID-based attacks.

---

## Research Context

This work is submitted to the ICITS 2026 conference and focuses on integrating mobility control and tolling within a unified V2X framework.

---

## Authors

| Name                          | Title      | Research Area                          |
|-------------------------------|-----------|----------------------------------------|
| Rutuja Bhagat                | Student    | V2X Communications, Vehicular Networks |
| Abhimanyu Singh              | Student    | IEEE 802.11p, OMNeT++ Simulation       |
| Sahil Kalotra                | Student    | Network Security, VANET                |
| Bhoomika R P                 | Student    | Intelligent Transportation Systems     |
| Prafullata K. Auradkar       | Professor  | Wireless Networks, IoT, Edge Computing |

---

## Code Availability

The implementation and simulation setup are available at:
https://github.com/ruubhagat/V2X-Lane-Clearing-Toll-System
