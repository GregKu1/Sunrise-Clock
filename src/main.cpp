#include <Arduino.h>
#include <USB.h>
#include <USBCDC.h>
#include <WiFi.h>
#include "time.h"
#include <Wire.h>

#include "credentials.h"

#define ARDUINO_USB_MODE 1
#define ARDUINO_USB_CDC_ON_BOOT 1

#define NTP "pool.ntp.org"
#define TZ "GMT0BST,M3.5.0/1,M10.5.0"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;


void syncNTP(void* pvParameters){
  while (1)
  {
    
    Serial.print("connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      vTaskDelay(pdMS_TO_TICKS(500));
      Serial.println("waiting...");
    }

    Serial.println("\nWifi Connected");


    configTzTime(TZ, NTP);
    struct tm timeinfo;
    int tries = 0;
    while (!getLocalTime(&timeinfo, 5000) && tries < 3)
    {
      Serial.println("attempting to sync");
      tries++;
    }

    if (tries < 3)
    {
      Serial.println("time sync ok");
    }
    else
    {
      Serial.println("failed to sync after 3 attempts");
    }
    

    Serial.println("disconnecting from WiFi");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    //vTaskDelay(pdMS_TO_TICKS(12*60*60*1000)); // real
    vTaskDelay(pdMS_TO_TICKS(60000));  // testing
  }

}

void displayTime(void* pvParameters){
  while (1)
  {
    digitalWrite(LED_BUILTIN, HIGH);

    time_t currentTime;
    struct tm timeinfo;

    time(&currentTime); // get current time since 1970
    localtime_r(&currentTime, &timeinfo); // converts time in seconds to local time and stores in a struct with fields

    char timeString[64];
    strftime(timeString, sizeof(timeString), "%c", &timeinfo);

    Serial.println(timeString);

    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  
}

void sunrise(void* pvParameters){

}

void setup() {
  Serial.begin(115200);

  xTaskCreate(
    syncNTP,
    "syncNTP",
    4096,
    NULL,
    2,
    NULL
  );

  xTaskCreate(
    displayTime,
    "displayTime",
    4096,
    NULL,
    1,
    NULL
  );

}

void loop() {
  // put your main code here, to run repeatedly:
}

