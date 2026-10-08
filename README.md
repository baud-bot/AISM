# 🦈 AISM — Intelligent Motorcycle Safety Assistant

**🌐 Languages:** **English** · [Español](README.es.md)

An embedded safety system for motorcyclists built on the **Arduino Nano**. It detects nearby obstacles, dangerous lean angles, sudden movements, and possible crashes, then automatically sends an **SMS alert with a Google Maps link** to a predefined emergency contact.

> 📌 **Note:** All on-screen messages (LCD) and SMS alerts are displayed in Spanish, as the project was originally developed for a Spanish-speaking audience. A Spanish → English translation table is available at the end of this document.

---

## ✨ Features

- **Obstacle detection** on three sides (left, right, rear) using ultrasonic sensors.
  
- **Lean angle monitoring** (roll & pitch) with a 6-axis IMU.

- **Sudden movement and rotation detection** with configurable thresholds.
  
- **Crash detection** based on simultaneous acceleration and tilt thresholds.
  
- **5-second confirmation countdown** to avoid false positives.
  
- **Alarm cancellation** by straightening the bike or turning off the switch.
  
- **Automatic SMS alert** with GPS coordinates as a Google Maps link.
  
- **LCD status display** with anti-flicker caching.
  
- **Buzzer alert** with distinct tones for precaution, danger, and crash states.
  
- **RAM-optimized** for the ATmega328P (2 KB SRAM).

---

## 🔧 Hardware Requirements

| Component | Quantity | Notes |
|---|---|---|
| Arduino Nano (ATmega328P) | 1 | 16 MHz, 5 V |
| HC-SR04 ultrasonic sensor | 3 | Left, right, rear |
| MPU6050 accelerometer + gyroscope | 1 | I2C, configured at ±8 g |
| LCD 16x2 with I2C module | 1 | Address `0x27` or `0x3F` |
| Passive buzzer | 1 | 5 V |
| SPST switch | 1 | Power on/off |
| GPS NEO-6M | 1 | TTL, 9600 baud |
| SIM900 GSM module | 1 | Quad-band, needs external 5 V / 2 A |
| GSM antenna | 1 | SMA connector |
| 5 V / 2 A power supply | 1 | Dedicated to the SIM900 |
| 9 V battery or USB power | 1 | For the Arduino |
| Dupont wires and breadboard | — | For prototyping |

### ⚡ Power Notes

- The SIM900 draws current peaks of up to **2 A** during SMS transmission.
- **Never power the SIM900 from the Arduino's 5 V pin.**
- Use a **dedicated 5 V / 2 A supply** for the GSM module.
- **Share only the GND** between the Arduino, the SIM900, and the external supply.

---

## 📍 Pinout

| Arduino Nano Pin | Component |
|---|---|
| **D2** | HC-SR04 LEFT — TRIG |
| **D3** | HC-SR04 LEFT — ECHO |
| **D4** | HC-SR04 RIGHT — TRIG |
| **D5** | HC-SR04 RIGHT — ECHO |
| **D6** | HC-SR04 REAR — TRIG |
| **D7** | HC-SR04 REAR — ECHO |
| **D8** | GPS TX (AltSoftSerial RX) |
| **D9** | GPS RX (AltSoftSerial TX) |
| **D10** | SIM900 TXD (SoftwareSerial RX) |
| **D11** | SIM900 RXD (SoftwareSerial TX) |
| **A0** | Buzzer (+) |
| **A1** | Switch (INPUT_PULLUP, other leg → GND) |
| **A4 (SDA)** | LCD SDA + MPU6050 SDA |
| **A5 (SCL)** | LCD SCL + MPU6050 SCL |
| **5V** | LCD, MPU6050, GPS, ultrasonic sensors |
| **GND** | Common ground for all modules |
| **VIN** | 9 V battery (optional) or USB power |

> ⚠️ The GPS uses **AltSoftSerial**, which is fixed to pins **D8 (RX)** and **D9 (TX)** on the Arduino Nano. This avoids timer conflicts with the `tone()` function used by the buzzer.

---

## 📚 Required Libraries

Install the following libraries through the Arduino Library Manager:

| Library | Author | Purpose |
|---|---|---|
| **AltSoftSerial** | Paul Stoffregen | Serial communication with the GPS (fixed to D8/D9) |
| **TinyGPSPlus** | Mikal Hart | NMEA parsing for the NEO-6M GPS |
| **LiquidCrystal_I2C** | Frank de Brabander | I2C LCD control |
| **Wire** | Arduino | I2C communication (built-in) |
| **SoftwareSerial** | Arduino | Serial communication with the SIM900 (built-in) |
| **math.h** | — | Mathematical functions (built-in) |

**Note:** `Wire`, `SoftwareSerial`, and `math.h` are included with the Arduino IDE and require no additional installation.

---

## ⚙️ Development Environment Setup

### Board configuration

| Setting | Value |
|---|---|
| Board | Arduino Nano |
| Processor | **ATmega328P (Old Bootloader)** |
| Serial Monitor | 9600 baud |
| GPS baud rate | 9600 |
| SIM900 baud rate | 9600 |

> ⚠️ **Most Arduino Nano clones use the "Old Bootloader" configuration.** If uploading fails with a *"programmer is not responding"* error, make sure this option is selected before trying again.

### Recommended workflow

1. Disconnect the SIM900 power supply before uploading code to the Arduino.
2. Upload the sketch through the Arduino IDE.
3. Reconnect the SIM900 power supply.
4. Press the **RESET** button on the Arduino Nano.
5. Open the Serial Monitor to verify operation.

This prevents power fluctuations from the GSM module from interfering with the upload process.

---

## 🚀 Installation

1. **Clone the repository:**

       git clone https://github.com/yourusername/AISM.git

       cd AISM

2. Install the required libraries using the Arduino Library Manager.

3. Configure your emergency phone number in the source code:

       const char TELEFONO[] = "+595xxxxxxxxx";

4. Adjust the detection thresholds if needed:

       const float ACC_CHOQUE = 2.5;   // Crash acceleration threshold (g)
   
       const float ANG_CHOQUE = 60.0;  // Crash tilt threshold (degrees)
   
       const float ACC_ALERTA = 1.5;   // Sudden movement alert (g)

6. Wire all components following the pinout table.

7. Upload the sketch to the Arduino Nano.

8. Open the Serial Monitor at 9600 baud to observe real-time activity.

---

  ## 🧠 How It Works
   
Main loop

Every 150 ms the Arduino:

1.    Reads the three ultrasonic sensors and the MPU6050.

2.    Evaluates distance, tilt, rotation, and movement thresholds.

3.    Updates the buzzer tone and the LCD display.

4.    Reads GPS data continuously through AltSoftSerial.

5.    Checks for crash conditions.

# Crash detection

A crash is detected when both conditions occur simultaneously:

    Filtered acceleration greater than 2.5 g

    Maximum tilt (roll or pitch) greater than 60°

When a crash is detected:

- The LCD displays !! CHOQUE !! Xs with a 5-second countdown.

- The buzzer emits a 1500 Hz tone.

# The user may cancel the alert by:

- Straightening the bike (tilt below 25°).

- Turning off the switch.

If the alert is not cancelled within 5 seconds, an SMS is sent with the last valid GPS position.

---

## Alert levels

---

# Condition	Buzzer =	LCD Display

- Object closer than 25 cm	800 Hz =	! PRECAUCION !
  
- Object closer than 12.5 cm	1000 Hz	= !! PELIGRO !!
  
- Tilt greater than 30°	800 Hz = ! PRECAUCION !
  
- Tilt greater than 60°	1000 Hz	= !! PELIGRO !!
  
- Crash detected	1500 Hz	= !! CHOQUE !! 5s
  
- SMS sent	Silent	= SMS ENVIADO / Correcto!
  
- SMS failed	Silent	= SMS FALLO / Revisar senal

---

# 📱 SMS Format

When a crash is confirmed, the system sends an SMS in the following format:

    AUXILIO! https://maps.google.com/?q=-xx.xxxxxx,-xx.xxxxxx

Tapping the link opens Google Maps at the location of the accident. If the GPS has no fix at the moment of sending, the SMS will read:

    AUXILIO! GPS sin fix

---

## 🛠 Troubleshooting

- Upload fails with "programmer is not responding"	SIM900 power fluctuation;	Disconnect the SIM900 supply during upload

- LCD shows nothing	Wrong I2C address or contrast	Try 0x3F; adjust the contrast potentiometer

- Buzzer sounds distorted	Timer conflict with SoftwareSerial;	Use AltSoftSerial for the GPS (D8/D9)

- GSM never registers	Weak 2G signal or invalid band mode	Reset the module; test in an area with strong 2G coverage

- CSQ = 0	No signal	Move to an open area; check the antenna

- AT+CBAND? → INVALID_BAND_MODE	Firmware issue on some SIM900 units	Power-cycle the module; may require firmware update

- GPS Chars = 0	Incorrect wiring or baud rate;	Verify TX → D8 and RX → D9; try 38400 baud

- GPS receives data but no fix	No sky visibility	Move outdoors; wait 1–5 minutes

- SMS not sent (+CMS ERROR: 500)	Weak signal;	Move to a location with CSQ ≥ 5

---

# ⚠️ Known Limitations

- 2G dependency: The SIM900 only operates on 2G networks, which are being phased out in many regions. For long-term reliability, consider migrating to a 4G LTE module such as the SIM7600.

- GPS fix time: The NEO-6M requires a clear view of the sky. The first fix may take between 1 and 5 minutes.

- RAM usage: The system uses approximately 70% of the ATmega328P's SRAM. Additional features may require reducing buffers or moving to a board with more memory.

- Single recipient: The system sends SMS alerts to one phone number, defined at compile time.

- No event logging: Each crash triggers a single SMS; events are not stored locally.

---

## 🔮 Future Improvements

- Migrate to 4G LTE for wider network coverage.

- Add a manual panic button.

- Support multiple SMS recipients.

- Log events to an SD card.

- Publish data to a cloud dashboard (MQTT, HTTP).

- Integrate with local emergency services.

- Design a custom PCB for a compact enclosure.

-  Add a rechargeable Li-ion battery integrated into the motorcycle.

## 🙏 Acknowledgments

  - Paul Stoffregen for AltSoftSerial

  - Mikal Hart for TinyGPSPlus

  - Frank de Brabander for LiquidCrystal_I2C

  - The Arduino community for its extensive documentation and examples

Developed for the **CCTECH 3rd Edition 2026 Technology Fair**.

---

## 🌐 Display Messages (Spanish → English)

The LCD and SMS alerts use Spanish messages. Here is a quick reference:

| Spanish | English |
|---|---|
| `SEGURO` | Safe |
| `! PRECAUCION !` | ! CAUTION ! |
| `!! PELIGRO !!` | !! DANGER !! |
| `!! CHOQUE !!` | !! CRASH !! |
| `Enderezar moto` | Straighten the bike |
| `Enviando SMS de alerta...` | Sending alert SMS... |
| `SMS ENVIADO / Correcto!` | SMS SENT / Success! |
| `SMS FALLO / Revisar senal` | SMS FAILED / Check signal |
| `INC:XX°` | Tilt: XX° |
| `I:XX D:XX A:XX` | L:XX R:XX B:XX (Left/Right/Back) |
| `G` | Sudden turn |
| `M` | Sudden movement |
| `APAGADO` | OFF |

This project is released under the MIT License. See the "LICENSE" file for details.



<p align="center"> <b>🦈 AISM — Ride safe. Get help fast.</b> </p> 
