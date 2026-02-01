#include <Arduino.h>
#include <USB.h>
#include <USBCDC.h>
#include <WiFi.h>
#include "time.h"
#include <Wire.h>

#include "credentials.h"

#define ARDUINO_USB_MODE 1
#define ARDUINO_USB_CDC_ON_BOOT 1

#define LED_ONBOARD 15

#define NTP "pool.ntp.org", "time.google.com", "time.nist.gov"
#define TZ "GMT0BST,M3.5.0/1,M10.5.0"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;


void syncNTP(void* pvParameters){
  while (1)
  {    
    Serial.printf("connecting to %s \n", ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
      vTaskDelay(pdMS_TO_TICKS(500));
      Serial.println("connecting...");
    }

    Serial.println("\nWifi Connected");

    configTzTime(TZ, NTP);  // syncs local clock with NTP server

    vTaskDelay(10000);
 
    Serial.printf("disconnecting from %s \n", ssid);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    vTaskDelay(12*60*60*1000 / portTICK_PERIOD_MS); // real
    // vTaskDelay(pdMS_TO_TICKS(30000));  // testing
  }

}

void displayTime(void* pvParameters){ // this will actually handle displaying time on the OLED
  while (1)
  {

    // time_t currentTime;
    struct tm timeinfo;

    // time(&currentTime); // get current time since 1970
    // localtime_r(&currentTime, &timeinfo); // converts time in seconds to local time and stores in a struct with fields

    getLocalTime(&timeinfo);

    char timeString[64];
    strftime(timeString, sizeof(timeString), "%c", &timeinfo);

    digitalWrite(LED_BUILTIN, HIGH);

    Serial.println(timeString);
    vTaskDelay(200);

    digitalWrite(LED_BUILTIN, LOW);

    vTaskDelay(pdMS_TO_TICKS(800));
  }
  
}
void alarm(void* pvParameters){ // this one will 

}


void sunrise(void* pvParameters){ // this one will do the pwm control of dimmer.
  while (1)
  {
    
  }
  

}

void setup() {
  Serial.begin(115200);
  vTaskDelay(5000); //  wait up bro i gotta open serial monitor

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

