# SumoRobot 🤖

[![Build Status](https://github.com/Badredenyx/SumoRobot/actions/workflows/ci.yml/badge.svg)](https://github.com/Badredenyx/SumoRobot/actions) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)  

A mini-sumo combat & line-following robot built on a PIC microcontroller, controllable wirelessly via Bluetooth. Switch between **Sumo Duel** and **Speed Race** modes from your PC—and watch it hunt opponents or zip along a black line at variable speeds!

---

## 📋 Table of Contents

- [Features](#-features)  
- [Hardware & Software Requirements](#-hardware--software-requirements)  
- [Installation](#-installation)  
- [Configuration](#-configuration)  
- [Usage](#-usage)  
  - [Sumo Combat Mode](#sumo-combat-mode)  
  - [Line-Follower Mode](#line-follower-mode)  
- [Project Structure & Architecture](#-project-structure--architecture)  
- [Contributing](#-contributing)  
- [License](#-license)

---

## 🔧 Features

- **Dual Modes**  
  - **Combat Sumo**: Detect and push opponents off the ring.  
  - **Line Follower**: Race along a black line at programmable speeds.  
- **Wireless Control**  
  - Bluetooth serial link—switch modes from your PC by sending `c` (combat) or `s` (suiveur).  
- **Status Reporting**  
  - On boot, robot sends `Robot PRET !\n\r` to confirm readiness.  
- **Configurable Speed Profiles**  
  - Tune motor PWM for different speed levels during line-following.  
- **Sensor Suite**  
  - IR reflectance sensors for edge detection (sumo border & line following).  
  - Ultrasonic or bump sensors (optional) for opponent detection.

*(Detailed feature list in the [Project PDF](./projet_robotSumo.pdf).)*

---

## 🛠️ Hardware & Software Requirements

- **Microcontroller**: PIC18Fxx (configured in MPLAB X)  
- **Compiler**: XC8 (v2.3+ recommended)  
- **Bluetooth Module**: HC-05 / HC-06 (configured for 9600 bps)  
- **Motor Driver**: L293D or equivalent H-bridge  
- **Sensors**: IR reflectance array, bump/ultrasonic sensor (optional)  
- **Development Machine**: Windows or Linux with MPLAB X IDE (v5.50+)  
- **Dependencies**:  
  - MPLAB X project files (included under `/MPLABX/`)  
  - C standard library (provided by XC8)

---

## ⚙️ Installation

1. **Clone the repository**  
   ```bash
   git clone https://github.com/Badredenyx/SumoRobot.git
   cd SumoRobot
