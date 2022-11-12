# Send telemetry to Azure IoT Central using MXChip AZ3166 IoT devkit

Data acquisition (temperature, humidity, accelerometer, etc.) from connected objects (IoT) in the Microsoft Azure cloud using the MXChip AZ3166 IoT devkit which contains an STM32F412 type microcontroller and integrated sensors using the language C programming.

Features :
- Connect an MXCHIP AZ3166 devkit to IoT Central using functions developed in C programming language.
- Structure and sending the data provided by the sensors using JSON syntax.
- Sending an alert by Email to intervene and solve the problem using the IoT Central platform.
- An application (Web server) to remotely control the temperature and humidity in real time using the ESP32 board using the C programming language.

# Part 1 : Send telemetry to Azure IoT Central

The folder *IoTprojectSendDataCloud* contains the part that sends the data to the cloud

# Part 2 : Control real time temperature and humidity using the ESP32 board

The folder *DevProject* contains the code of application (Web server) to remotely control real time temperature and humidity using the ESP32 board
