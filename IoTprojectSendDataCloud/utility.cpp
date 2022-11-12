#include "Sensor.h"
#include "AzureIotHub.h"
#include "Arduino.h"
#include <ArduinoJson.h>
#include "AZ3166WiFi.h"
#define MESSAGE_MAX_LEN 256


DevI2C *i2c;
HTS221Sensor *sensor;
static LSM6DSLSensor *accelgyroSensor;
static LPS22HBSensor *pressureSensor;
IPAddress ip;

static int xAxesData[3];
static int gAxesData[3];
static float pressure;


void sensorInit()
{
    i2c = new DevI2C(D14, D15);
    sensor = new HTS221Sensor(*i2c);
    sensor->init(NULL);


    accelgyroSensor = new LSM6DSLSensor(*i2c, D4, D5);
    accelgyroSensor->init(NULL);
    accelgyroSensor->enableAccelerator();
    accelgyroSensor->enableGyroscope();

    // Initialize the pressure sensor
    pressureSensor = new LPS22HBSensor(*i2c);
    pressureSensor->init(NULL);
}

void initWiFi(){
if(WiFi.begin() == WL_CONNECTED) {
        ip = WiFi.localIP(); // Obtient l’adresse IP de réseau WiFi
        Screen.print(1, "Wi-Fi Connected");
        Screen.print(2, WiFi.SSID());
        Screen.print(3, ip.get_address());
        digitalWrite(LED_WIFI, 1);
    } else {
        Screen.print("WiFi\r\nNot Connected\r\nWIFI_SSID?\r\n");
        return;
    }
}

float readTemperature()
{
    float temperature = 0;

    sensor->enable();
    sensor->getTemperature(&temperature);
    sensor->disable();
    sensor->reset();

    return temperature;
}

float readHumidity()
{
    float humidity = 0;

    sensor->enable();
    sensor->getHumidity(&humidity);
    sensor->disable();
    sensor->reset();

    return humidity;
}

int readAccelerometer()
{
    // Read accelerometer sensor
    accelgyroSensor->getXAxes(xAxesData);
    return xAxesData[3];
}

int readGyroscope()
{
     // Read gyroscope sensor
    accelgyroSensor->getGAxes(gAxesData);
    return gAxesData[3];
}

float readPressure()
{
    // Read gyroscope sensor
    pressureSensor->getPressure(&pressure);
    return pressure;

}
void readMessage(char *payload)
{
    float temperature = readTemperature();
    float humidity = readHumidity();
    float pressure = readPressure();
    xAxesData[3] = readAccelerometer();
    gAxesData[3] = readGyroscope();

    StaticJsonDocument<MESSAGE_MAX_LEN> doc;
  
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["pressure"] = pressure;
    doc["accelerometerX"] = xAxesData[0];
    doc["accelerometerY"] = xAxesData[1];
    doc["accelerometerZ"] = xAxesData[2];
    doc["gyroscopeX"] = gAxesData[0];
    doc["gyroscopeY"] = gAxesData[1];
    doc["gyroscopeZ"] = gAxesData[2];


    serializeJson(doc, payload, MESSAGE_MAX_LEN);
}
