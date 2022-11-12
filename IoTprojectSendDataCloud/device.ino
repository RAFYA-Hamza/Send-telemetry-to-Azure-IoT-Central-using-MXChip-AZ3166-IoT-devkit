// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define SERIAL_VERBOSE_LOGGING_ENABLED 1
#include "src/iotc/iotc.h"
#include "src/iotc/common/string_buffer.h"
#include "Sensor.h"
#include <ArduinoJson.h>
#include "utility.h"

#define MESSAGE_MAX_LEN 256

// Peripherals
static DevI2C *ext_i2c; // Initialise le protocole de communication
static HTS221Sensor *Sensor;
static IOTContext context = NULL;

static float temperature;
static float humidity;
static int xAxesData[3];
static int gAxesData[3];
static float pressure;
char messagePayload[MESSAGE_MAX_LEN];
char buff[16];
 
static unsigned prevMillis = 0;
int ButtCount=0;
bool previous_etat = 0;
bool Value;

StaticJsonDocument<MESSAGE_MAX_LEN> doc1;

// PRIMARY/SECONDARY KEY ?? (DPS)
// Uncomment below to Use DPS Symm Key (primary/secondary key..)
IOTConnectType connectType = IOTC_CONNECT_SYMM_KEY;
const char* scopeId = "0ne006A0226";
const char* deviceId = "1rkln1g2pbh";
const char* deviceKey = "HB6d+q3yIipDMPcpxst2sk6B9cE30HH8s1aZXBckcMo=";

static bool isConnected = false;


void onEvent(IOTContext ctx, IOTCallbackInfo *callbackInfo) {
    if (strcmp(callbackInfo->eventName, "ConnectionStatus") == 0) {
        LOG_VERBOSE("Is connected ? %s (%d)", callbackInfo->statusCode == IOTC_CONNECTION_OK ? "YES" : "NO", callbackInfo->statusCode);
        isConnected = callbackInfo->statusCode == IOTC_CONNECTION_OK;
    }

    AzureIOT::StringBuffer buffer;
    if (callbackInfo->payloadLength > 0) {
        buffer.initialize(callbackInfo->payload, callbackInfo->payloadLength);
    }
    LOG_VERBOSE("- [%s] event was received. Payload => %s", callbackInfo->eventName, buffer.getLength() ? *buffer : "EMPTY");

    if (strcmp(callbackInfo->eventName, "Command") == 0) {
        LOG_VERBOSE("- Command name was => %s\r\n", callbackInfo->tag);
    }
}


void setup()
{
    Serial.begin(115200);
    pinMode(LED_WIFI, OUTPUT);
    pinMode(LED_AZURE, OUTPUT);
    pinMode(LED_USER, OUTPUT);
    pinMode(USER_BUTTON_B, INPUT_PULLUP);

    Screen.print(0, "IoT Devkit");
    Screen.print(1, "Initializing...");
    Screen.print(2, "\tWi-Fi");

    initWiFi();

    LOG_VERBOSE("WiFi WL_CONNECTED");
    // Azure IOT Central setup
    int errorCode = iotc_init_context(&context);
    if (errorCode != 0) {
        LOG_ERROR("Error initializing IOTC. Code %d", errorCode);
        return;
    }

    iotc_set_logging(IOTC_LOGGING_API_ONLY);

    // for the simplicity of this sample, used same callback for all the events below
    iotc_on(context, "MessageSent", onEvent, NULL);
    iotc_on(context, "Command", onEvent, NULL);
    iotc_on(context, "ConnectionStatus", onEvent, NULL);
    iotc_on(context, "SettingsUpdated", onEvent, NULL);
    iotc_on(context, "Error", onEvent, NULL);

    errorCode = iotc_connect(context, scopeId, deviceKey, deviceId, connectType);
    if (errorCode != 0) {
        LOG_ERROR("Error @ iotc_connect. Code %d", errorCode);
        return;
    }
    LOG_VERBOSE("Done!");

    prevMillis = millis();
    sensorInit();

}

void loop()
{

    if (isConnected) {
        unsigned long ms = millis();
        Value = digitalRead(USER_BUTTON_B);
        if (Value == 1){
            previous_etat = 1;
        }
        if (Value == 0 && previous_etat == 1) {
            previous_etat = 0;
            if (ButtCount > 4){
                ButtCount = 0;
            }
            ButtCount++;
            readMessage(messagePayload);
            DeserializationError error = deserializeJson(doc1, messagePayload);
            switch (ButtCount){
                case 1:
                    Screen.clean();
                    Screen.print(0, "Welcome...");
                    Screen.print(1, " ");
                    Screen.print(2, "\t\t\tPush Up");
                    Screen.print(3, "The Button B");
                break;
                case 2:
                    Screen.clean();
                    temperature = doc1["temperature"];
                    sprintf(buff, "Temp : %.2f C\r\n", temperature ); //Stocker la valeur lie de la température dans le Buffer
                    Screen.print(0, "Environment : ");
                    Screen.print(1, buff);  //Afficher la valeur lie dans l'afficheur LCD
                    humidity = doc1["humidity"];
                    Serial.printf("Humidity: %f\n", humidity);
                    sprintf(buff, "Humidity : %s\r\n", f2s(humidity, 1 )); //Stocker la valeur lie de l'humidity dans le Buffer
                    Screen.print(2, buff);  //Afficher la valeur lie dans l'afficheur LCD
        
                break;
                case 3:
                    xAxesData[0] = doc1["accelerometerX"];
                    xAxesData[1] = doc1["accelerometerY"];
                    xAxesData[2] = doc1["accelerometerZ"];

                    Screen.print(0, "Accelerometer :");
                    sprintf(buff, "\t\t\tx : %d\r\n", (int)xAxesData[0] ); //Stocker la valeur lie de l'Accelerometer d'axe x dans le Buffer
                    Screen.print(1, buff);  //Afficher la valeur lie dans l'afficheur LCD

                    sprintf(buff, "\t\t\ty : %d\r\n", (int)xAxesData[1] ); //Stocker la valeur lie de l'Accelerometer d'axe y dans le Buffer
                    Screen.print(2, buff);  //Afficher la valeur lie dans l'afficheur LCD

                    sprintf(buff, "\t\t\tz : %d\r\n", (int)xAxesData[2] ); //Stocker la valeur lie de l'Accelerometer d'axe z dans le Buffer
                    Screen.print(3, buff);  //Afficher la valeur lie dans l'afficheur LCD    
                break;
                case 4:
                    gAxesData[0] = doc1["gyroscopeX"];
                    gAxesData[1] = doc1["gyroscopeY"];
                    gAxesData[2] = doc1["gyroscopeZ"];

                    Screen.print(0, "Gyroscope :");
                    sprintf(buff, "\t\t\tx : %d\r\n", (int)gAxesData[0] ); //Stocker la valeur lie de Gyroscoped'axe x dans le Buffer
                    Screen.print(1, buff);  //Afficher la valeur lie dans l'afficheur LCD

                    sprintf(buff, "\t\t\ty : %d\r\n", (int)gAxesData[1] ); //Stocker la valeur lie de Gyroscope d'axe y dans le Buffer
                    Screen.print(2, buff);  //Afficher la valeur lie dans l'afficheur LCD

                    sprintf(buff, "\t\t\tz : %d\r\n", (int)gAxesData[2] ); //Stocker la valeur lie de Gyroscope d'axe z dans le Buffer
                    Screen.print(3, buff);  //Afficher la valeur lie dans l'afficheur LCD    
                break;
                case 5:
                    pressure = doc1["pressure"];
                    Screen.clean();
                    Screen.print(0, "Pressure :");
                    sprintf(buff, "Pressure : %.2f \r\n", (float)pressure);
                    Screen.print(1, buff);
                break;
            }
        }
        if (ms - prevMillis > 20000) { // send telemetry every 20 second
            char msg[64] = {0};
            int errorCode = 0;
            prevMillis = ms;
            readMessage(messagePayload);
            errorCode = iotc_send_telemetry(context, messagePayload, MESSAGE_MAX_LEN);
            digitalWrite(LED_AZURE, 1);
            if (errorCode != 0) {
                LOG_ERROR("Sending message has failed with error code %d", errorCode);
            }
        }

    }
    if (context)
        iotc_do_work(context); // do background work for iotc
}
