# IoT-based ECG Monitoring System 🫀

![ESP32](https://img.shields.io/badge/ESP32-Espressif-red)
![ThingsBoard](https://img.shields.io/badge/IoT-ThingsBoard-blue)
![Docker](https://img.shields.io/badge/Deploy-Docker-2496ED)
![Status](https://img.shields.io/badge/Status-Active-success)

> **An end-to-end IoT solution for real-time Electrocardiogram (ECG) acquisition, Digital Signal Processing (DSP), and Heart Rate Variability (HRV) analysis.**

---

## 📖 Table of Contents
- [Overview](#-overview)
- [System Architecture](#-system-architecture)
- [Key Features](#-key-features)
- [Hardware & Tech Stack](#-hardware--tech-stack)
- [Signal Processing Pipeline](#-signal-processing-pipeline)
- [Getting Started](#-getting-started)
- [Results](#-results)
- [License](#-license)

---

## 🔭 Overview
This project aims to build a low-cost, portable, and reliable ECG monitoring system. It captures bio-signals using the **AD8232 sensor**, processes them directly on the **ESP32** using advanced digital filters and the **Pan-Tompkins algorithm**, and transmits health metrics to a self-hosted **ThingsBoard** server securely via **Cloudflare Tunnel**.

The system provides real-time visualization of ECG waveforms and calculates critical HRV metrics (SDNN, RMSSD) for early health risk diagnosis.
## User interface
<img width="925" height="448" alt="image" src="https://github.com/user-attachments/assets/1c56474b-d560-419d-b62d-2283155e89e5" />

---

## Firmware Architecture
<img width="698" height="443" alt="image" src="https://github.com/user-attachments/assets/d34749f8-3d80-40cf-8c11-b7f7d826be42" />

## 🏗 System Architecture
1.  **Sensing:** AD8232 captures analog heart signals.
2.  **Edge Processing (ESP32):**
    * ADC Sampling.
    * Digital Filtering (Low-pass, High-pass, Notch).
    * R-Peak Detection (Pan-Tompkins Algorithm).
    * Lead-off Detection.
3.  **Transmission:** Processed data is sent via **HTTP** telemetry.
4.  **Cloud/Server:**
    * **Cloudflare Tunnel:** Exposes the local server securely to the internet.
    * **Docker:** Containers for ThingsBoard Community Edition.
    * **ThingsBoard:** Dashboard visualization and HRV analytics.

---

## ✨ Key Features

* **Real-time ECG Acquisition:** High-precision sampling of cardiac signals.
* **On-Device DSP:** Implementation of IIR/FIR filters to remove:
    * Baseline wander (High-pass filter).
    * Muscle noise (Low-pass filter).
    * Power line interference (50Hz/60Hz Band-stop filter).
* **Advanced Algorithms:**
    * **Pan-Tompkins:** Robust detection of QRS complexes and R-peaks.
    * **HRV Analysis:** Calculation of **SDNN** and **RMSSD** for stress and heart health assessment.
* **Lead-off Detection:** Automatic alert when electrodes are disconnected.
* **Secure IoT Dashboard:** Self-hosted ThingsBoard instance accessible anywhere via Cloudflare Zero Trust.

---

## 🛠 Hardware & Tech Stack

### Hardware
* **Microcontroller:** ESP32 Development Board (WROOM-32).
* **Sensor:** AD8232 Single Lead Heart Rate Monitor.
* **Electrodes:** 3-lead ECG cable with biomedical pads.

### Firmware & Software
* **Language:** C/C++ (ESP-IDF / Arduino Framework).
* **Backend:** ThingsBoard Community Edition (TB-CE).
* **Deployment:** Docker & Docker Compose.
* **Networking:** Cloudflare Tunnel (cloudflared).

---

## 🧠 Signal Processing Pipeline

The raw signal from AD8232 is noisy. The ESP32 implements the following pipeline before analysis:

1.  **Raw Input:** `ADC Reading`
2.  **Filtering:**
    * *Low-Pass Filter:* Removes high-frequency noise.
    * *High-Pass Filter:* Eliminates baseline wander caused by breathing.
    * *Notch Filter:* Rejects 50Hz power line noise.
3.  **Pan-Tompkins Algorithm:**
    * Differentiation $\rightarrow$ Squaring $\rightarrow$ Moving Window Integration $\rightarrow$ Adaptive Thresholding.
4.  **Output:** Accurate R-Peak timestamps for Heart Rate (BPM) calculation.

The R-Peak timestamps and ECG values then will be send to thingsboard server for further analysis and diagnosis.

Below is the flowchart of firmware algorithm. The algorithm is divided into 2 phases and executed on 2 different core of a CPU to enhance the performance.

<img width="530" height="931" alt="image" src="https://github.com/user-attachments/assets/427a78be-8d46-48e2-a59b-ef3290cb4a6f" />

---

### 🚀 Hardware Wiring
| AD8232 Pin | ESP32 Pin |
| :--- | :--- |
| **GND** | GND |
| **3.3V** | 3.3V |
| **OUTPUT** | GPIO 36 (ADC1_0) |
| **LO+** | GPIO 34 (Input) |
| **LO-** | GPIO 35 (Input) |
<img width="1610" height="573" alt="image" src="https://github.com/user-attachments/assets/a9f44718-1738-4381-8b37-a200f7343fdc" />







