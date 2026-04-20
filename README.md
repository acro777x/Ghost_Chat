# 👻 Ghost Chat: "The College Dark Web"

**Ghost Chat** is a fully decentralized, off-grid hardware and software mesh communication network designed for maximum anonymity. Originally conceived as an ESP32 captive portal chat server, it has evolved into a robust **React Native Android App** + **ESP32 LoRa Router Node** architecture that bypasses standard internet providers completely.

## 🌟 The Vision
Ghost Chat acts like Tor for local environments (e.g., college campuses). The Android Application handles all AES-256 encryption client-side, while the ESP32 hardware acts as a blind relay mesh node. Messages bounce from node to node via the **SX1276 LoRa** protocol, making it completely invisible to internet service providers, school WiFi admins, and cellular towers.

## 🚀 Key Features

### 💻 Deep Encrypted Node Backend (ESP32)
*   **Zero-Knowledge Relaying:** The ESP32 node blindly routes packets it does not possess the keys to read.
*   **LRU Partition Management:** Dynamic, stateless caching allows infinite ephemeral room generation without memory leaks.
*   **LoRa Over-the-Air Delivery:** Long-range (1-10km) message bouncing even without cellular data.
*   **Destruct & Wipe Protocols:** Panic functions physically obliterate SPIFFS and SD cache metadata when triggered.

### 📱 Premium "Glassmorphism" Android App (React Native)
*   **Bypassing Web Restrictions:** Moving out of the Android "Captive Portal" allows native file-picking and media rendering.
*   **Modern UI:** Deep iOS-style blurs, bottom-sheet active styling, and fully animated gradient bubbles.
*   **File Attachments:** Native handling for Image payloads and "Bomb Images" (Burn on View).
*   **Offline First:** The React Native app gracefully handles polling errors and automatically reconnects using Expo networking APIs.

## 🛠️ Tech Stack & Directory Structure

*   **/ghost_chat_v3_sx1276:** The core C++ ESP32 firmware. Open and flash this using the Arduino IDE. Contains the async webserver and SPIFFS SD storage mechanisms.
*   **/ghost_chat_app:** The React Native Mobile Application. Uses `expo-blur`, `@react-navigation/native-stack`, and custom base-64 cryptography.

## ⚙️ How to Deploy

### 1. Flash the Hardware
Upload `ghost_chat_v3_sx1276.ino` to any Heltec WiFi LoRa 32 V2 or standard ESP32 board. 
Connect to the `Ghost_Net` Wi-Fi hotspot broadcasted by the ESP32.

### 2. Launch the Application
To run the Android app locally:
```bash
cd ghost_chat_app
npx expo start
```
*Use the "Expo Go" app on your device to scan the QR code and instantly launch the Dark Web interface.*

To compile into a hard `.apk` file:
```bash
eas build -p android --profile preview
```

---
*Maintained and Developed by [acro777x](https://github.com/acro777x).*
