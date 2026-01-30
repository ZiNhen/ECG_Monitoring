# IoT Real-time ECG Monitoring System 🫀

![ESP32](https://img.shields.io/badge/SoC-ESP32--S3-red)
![Framework](https://img.shields.io/badge/Framework-ESP--IDF-green)
![RTOS](https://img.shields.io/badge/OS-FreeRTOS-blue)
![Backend](https://img.shields.io/badge/IoT-ThingsBoard-2496ED)

> **Final Project - CE224.Q11: Embedded System Design** > Faculty of Computer Engineering - University of Information Technology (UIT - VNUHCM)

## 📖 Overview
This project implements an end-to-end IoT solution for real-time Electrocardiogram (ECG) monitoring. It captures bio-signals using the **AD8232** analog front-end, processes them on the **ESP32** using a dual-core FreeRTOS architecture with advanced DSP algorithms, and securely transmits telemetry to a self-hosted **ThingsBoard** server via **Cloudflare Tunnel**.

The system is capable of detecting R-peaks, calculating Heart Rate (BPM), and analyzing Heart Rate Variability (HRV) metrics (SDNN, RMSSD) to diagnose stress levels and arrhythmias.

---

## 🏗 System Architecture

The system follows a Client-Server model designed for reliability and security:

![System Architecture](docs/system_architecture.png)
*(Note: Use Figure 7 from your report here)*

1.  [cite_start]**Sensing Layer:** AD8232 sensor with a custom **Faraday Cage** shielding to minimize electromagnetic interference (EMI)[cite: 1805].
2.  **Edge Processing (ESP32):**
    * **Dual-core SMP Architecture:**
        * [cite_start]**Core 1 (App Core):** Dedicated to high-priority Signal Processing (ADC reading, Filtering, Pan-Tompkins)[cite: 1897].
        * [cite_start]**Core 0 (Protocol Core):** Handles Wi-Fi stack and HTTP transmission to avoid blocking the sampling process[cite: 1898].
3.  **Cloud/Server Layer:**
    * [cite_start]**Cloudflare Tunnel:** Exposes the local ThingsBoard server securely (Zero Trust) without port forwarding, utilizing SSL/TLS for encrypted data transmission[cite: 2137].
    * **ThingsBoard CE:** Visualizes real-time waveforms and executes Rule Chains for health alerts.

---

## 🧠 Digital Signal Processing (DSP) Pipeline

[cite_start]To ensure medical-grade accuracy, the raw signal undergoes a rigorous processing chain before analysis[cite: 2064]:

1.  [cite_start]**Sampling:** High-frequency sampling at **500Hz** to capture signal details[cite: 2033].
2.  **Digital Filtering (IIR Butterworth 4th Order):**
    * [cite_start]**High-Pass Filter ($f_c = 0.5Hz$):** Removes baseline wander caused by respiration[cite: 2052].
    * **Low-Pass Filter ($f_c = 40Hz$):** Removes muscle artifacts (EMG)[cite: 2058].
    * **Band-Stop Filter ($40Hz - 50Hz$):** Specifically targets power line interference (50Hz noise).
3.  **Downsampling:** Reduces sampling rate to **200Hz** using linear interpolation to optimize performance for the Pan-Tompkins algorithm.
4.  **Algorithm:**
    * **Pan-Tompkins:** Detects QRS complexes and R-peaks in real-time.
    * **HRV Calculation:** Computes **SDNN** and **RMSSD** (Root Mean Square of Successive Differences) for stress assessment[cite: 2108].

---

## 🛠 Hardware & Wiring

### Component List
* [cite_start]**MCU:** ESP32-DevKitC-32U (ESP32-WROOM-32U)[cite: 1699].
* **Sensor:** SparkFun AD8232 Single Lead Heart Rate Monitor.
* [cite_start]**Shielding:** Custom DIY Faraday Cage (Aluminum foil connected to GND).

### Pin Configuration
| AD8232 Pin | ESP32 Pin | Function |
| :--- | :--- | :--- |
| **GND** | GND | Power Ground |
| **3.3V** | 3.3V | VCC |
| **OUTPUT** | GPIO 34 | Analog Signal Input (ADC1_CH6) |
| **LO+** | GPIO 35 | Leads-off Detection (+) |
| **LO-** | GPIO 32 | Leads-off Detection (-) |

---

## 💻 Software Implementation

### Firmware (ESP-IDF)
The firmware is built using **ESP-IDF** and structured as a component-based project. It utilizes **FreeRTOS** features for inter-task communication:

* **`ECG_Reading_Task` (Core 1):**
    * Triggered by a hardware timer (GPTimer) for precise sampling.
    * Executes the DSP pipeline and R-peak detection.
    * [cite_start]Pushes processed data to a Queue[cite: 1963, 1973].
* **`HTTP_Transmission_Task` (Core 0):**
    * Ideally decoupled from sampling.
    * Batches data into JSON payloads.
    * [cite_start]Sends telemetry via HTTP POST to ThingsBoard[cite: 1975].

### Server Side (ThingsBoard & Cloudflare)
* **Dockerized Deployment:** ThingsBoard Community Edition runs in a Docker container.
* [cite_start]**Rule Engine:** Custom logic to calculate heart health status (Normal, Tachycardia, Bradycardia) based on HR thresholds[cite: 2141].
* **Security:** Cloudflare daemon (`cloudflared`) creates an outbound-only tunnel, protecting the server from direct internet exposure[cite: 2129].

---

## 📊 Results & Validation

### Dashboard
The real-time dashboard visualizes the filtered ECG waveform, instantaneous Heart Rate, and HRV metrics.

![Dashboard](docs/dashboard_relax.png)
*(Note: Use Figure 20/21 from the report)*

### Accuracy Test
We validated the system against **Google Fit** (PPG method) under different scenarios (Relax vs. Stress):

| Scenario | System Measurement | Google Fit Reference | Error Rate |
| :--- | :--- | :--- | :--- |
| **Relax** | 76 BPM | 74 BPM | ~1.3% |

*The result demonstrates high reliability with minimal deviation from the reference device[cite: 2348].*

---

## 🤝 Team Members
This project was developed by the **CE224.Q11 Team**:
* **Tran Duy Nhan (ZiNhen)** - 23521088
* Tran Le Quoc Thinh - 23521513
* Than Thanh Khoi - 23520788
* Pham Duc Thang - 23521430

## 📜 References
* [1] Pan-Tompkins Algorithm for QRS Detection.
* [2] ESP-IDF Programming Guide.
* [3] AD8232 Technical Reference.
* [4] ThingsBoard IoT Platform Documentation.

---
*University of Information Technology - VNUHCM | 2025*
