Hardware Description

1. **ESP32 Microcontroller**
   The ESP32 is the main controller used in this project. It reads the sensor values, processes them, and sends the data for further analysis. It also has in-built Wi-Fi which makes it suitable for IoT-based applications.

2. **Vibration Sensor (SW-420)**
   This sensor is used to detect vibrations in the wire. It has two outputs: a digital output to check if vibration is present, and an analog output to measure how strong the vibration is.

3. **Current Sensor (ACS712)**
   The current sensor is connected to the wire to measure the flow of current. If the wire is broken, the current value drops, which helps in identifying faults.

4. **Voltage Sensor**
   This sensor measures the voltage across the wire. It helps in knowing whether the wire is in normal condition or if the supply is interrupted.

5. **Power Supply (3.7V Source)**
   A low voltage DC source is used to power the setup. It also acts as the input signal for testing current and voltage during normal and broken wire conditions.

6. **Jumper Wires and Breadboard/PCB**
   These are used to connect all the components together for prototyping and testing.

