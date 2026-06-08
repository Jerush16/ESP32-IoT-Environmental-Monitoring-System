## Hardware Components

|     Component        |   Quantity  |         Purpose              |
|----------------------|-------------|------------------------------|
| ESP32 Dev Board      |  1          | Main microcontroller + Wi-Fi |
| DHT22 Sensor         |  1          | Temperature & humidity       |
| MQ-2 Gas Sensor      |  1          | Gas leak detection           |
| Soil Moisture Sensor |  1          | Soil wetness monitoring      |
| HC-SR04 Ultrasonic   |  1          | Water level / distance       |
| Breadboard           |  1          | Prototyping                  |
| Jumper Wires         | As required | Connections                  |
| USB Cable            |  1          | Programming and power        |
| Power Supply         |  1          | System power                 |

## Pin Connections

|    Sensor   |    Sensor Pin   | ESP32 Pin |
|-------------|-----------------|-----------|
| DHT22       | Data            | GPIO 4    |
| MQ-2        | Analog Out (AO) | GPIO 34   |
| Soil Sensor | Analog Out      | GPIO 35   |
| HC-SR04     | TRIG            | GPIO 5    | 
| HC-SR04     | ECHO            | GPIO 18   |

## Notes
- GPIO 34 and 35 are input-only pins on ESP32 — correct for analog sensors.
- MQ-2 requires 30 seconds warmup after power-on for stable gas readings.
- DHT22 data pin needs a 10kΩ pull-up resistor to 3.3V.
- HC-SR04 operates at 5V logic — use a voltage divider on ECHO pin to protect ESP32's 3.3V GPIO.
