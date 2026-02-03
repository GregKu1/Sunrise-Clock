#include <Arduino.h>
#include <USB.h>
#include <USBCDC.h>
#include <WiFi.h>
#include "time.h"
#include <Wire.h>

#include "credentials.h"

#define ARDUINO_USB_MODE 1
#define ARDUINO_USB_CDC_ON_BOOT 1


#define NTP "pool.ntp.org", "time.google.com", "time.nist.gov"
#define TZ "GMT0BST,M3.5.0/1,M10.5.0"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

typedef struct  // structure holds alarm time and relevant values
{
  
  struct tm alarmTime; // tm struct to store time data
  bool isActive; // alarm is active or not
} alarm_t;

// EXAMPLE USAGE OF STRUCT

/*
alarm_t week[7];  // create array of alarms for the week.
week[0].status = 1; // alarm is active
week[0].alarm.tm_hour = 8;  // 8:30AM
week[0].alarm.tm_min = 30;  
*/


alarm_t week[7];


// freeRTOS stuff
SemaphoreHandle_t xWeekLocker;

TimerHandle_t xAlarmTimer;

TaskHandle_t xSunriseTaskHandle;

TaskHandle_t xRecalculationTaskHandle;



void getTimeDifference(tm time_alarm, tm time_now, int* difference_seconds){  // gets the time difference in seconds
  int alarm_seconds = time_alarm.tm_hour*3600 + time_alarm.tm_min*60 + time_alarm.tm_sec;
  int now_seconds = time_now.tm_hour*3600 + time_now.tm_min*60 + time_now.tm_sec;
  *difference_seconds = alarm_seconds - now_seconds;
}

void recalculateNextAlarm(){
  Serial.println("getting current time and recalculating alarm timer");
  struct tm now;
  getLocalTime(&now);

  int dayOfWeek = now.tm_wday; // sunday = 0, we are indexing week from monday = 0
  int dayOfWeekCorrected;

  dayOfWeekCorrected = (dayOfWeek + 6) % 7 ;  // convert to monday 0 indexed week

  int secondsUntilNextAlarm;

  xSemaphoreTake(xWeekLocker, 100);

  bool foundActiveAlarm = false;
  int targetDay = dayOfWeekCorrected; // day in the week that we are searching, starting at today
  int daysAhead;  // days ahead from today, starting today


  for (daysAhead = 0; daysAhead < 7; daysAhead++) // check days in the week, starting today and go for 1 week forward
  {
    targetDay = (dayOfWeekCorrected + daysAhead) % 7; // wrap around to stay inside the week

    if (week[targetDay].isActive == false)
    {
      continue;
    }
    
    if (daysAhead == 0 &&   // for today, determine if the alarm is ahead or behind
       ((now.tm_hour > week[targetDay].alarmTime.tm_hour || // hour is larger OR hour is equal but minutes are higher means alarm behind us
       (now.tm_hour == week[targetDay].alarmTime.tm_hour && now.tm_min > week[targetDay].alarmTime.tm_min))))
    {
      continue;
    }
    foundActiveAlarm = true;
    break;
    
  }
  
  if (foundActiveAlarm == true)
  {
    getTimeDifference(week[targetDay].alarmTime, now, &secondsUntilNextAlarm);
    secondsUntilNextAlarm += (daysAhead*86400);

    Serial.printf("recalculation complete, time until next alarm is %d seconds \n", secondsUntilNextAlarm);
    xTimerChangePeriod(xAlarmTimer, pdMS_TO_TICKS(secondsUntilNextAlarm*1000), 100);
  }
  else
  {
    xTimerStop(xAlarmTimer, 100);
  }
  
  xSemaphoreGive(xWeekLocker);
  
}


void vSyncNTP(void* pvParameters){
  bool firstSync = true;
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

    if (firstSync == true)
    {
      recalculateNextAlarm();
      firstSync = false;
    }
    

    vTaskDelay(12*60*60*1000 / portTICK_PERIOD_MS); // real
    // vTaskDelay(pdMS_TO_TICKS(30000));  // testing
  }

}

void vDisplayTime(void* pvParameters){ // this will actually handle displaying time on the OLED
  while (1)
  {

    // time_t currentTime;
    struct tm timeinfo;
    getLocalTime(&timeinfo);

    char timeString[64];
    strftime(timeString, sizeof(timeString), "%c", &timeinfo);

    Serial.println(timeString);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  
}


void vTimerCallback(TimerHandle_t xTimer){
  xTaskNotifyGive(xSunriseTaskHandle);

}

void vRecalculation(void* pvParameters){
  while (1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    Serial.println("running recalculation");
    recalculateNextAlarm();
  }
  
}


void vSunrise(void* pvParameters){  // not thought through yet, ignore these
  /* 
- Unblocked by notification from vTimerCallback
- Increases pwm duty cycle over a period
- Blocks itself
*/ 
  while (1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    Serial.println("sunrise started");
    for (int i = 0; i < 30; i++)
    {
      Serial.printf("current minute: %d \n", i);
      //pwm increase by 1/30th or smth
      // vTaskDelay(pdMS_TO_TICKS(60*1000)); // wait 1 minute

      digitalWrite(LED_BUILTIN, HIGH);
      vTaskDelay(pdMS_TO_TICKS(500));
      digitalWrite(LED_BUILTIN, LOW);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    recalculateNextAlarm();
    Serial.println("sunrise finished");
    vTaskDelay(pdMS_TO_TICKS(1*60*60*1000));  // wait 1 hour
    // pwm OFF
    
  }
  

}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  xWeekLocker = xSemaphoreCreateMutex();

  xAlarmTimer = xTimerCreate("AlarmTimer", pdMS_TO_TICKS(1000), pdFALSE, 0, vTimerCallback);

  week[0].alarmTime.tm_hour = 18;
  week[0].alarmTime.tm_min = 34;
  week[0].isActive = true;

  week[1].alarmTime.tm_hour = 18;
  week[1].alarmTime.tm_min = 49;
  week[1].isActive = true;

  week[2].alarmTime.tm_hour = 7;
  week[2].alarmTime.tm_min = 0;
  week[2].isActive = true;

  week[3].alarmTime.tm_hour = 7;
  week[3].alarmTime.tm_min = 0;
  week[3].isActive = true;

  week[4].alarmTime.tm_hour = 7;
  week[4].alarmTime.tm_min = 0;
  week[4].isActive = true;

  week[5].alarmTime.tm_hour = 7;
  week[5].alarmTime.tm_min = 0;
  week[5].isActive = true;

  week[6].alarmTime.tm_hour = 7;
  week[6].alarmTime.tm_min = 0;
  week[6].isActive = true;


  Serial.begin(115200);
  vTaskDelay(5000); //  wait up bro i gotta open serial monitor


  xTaskCreate(
    vSyncNTP,
    "syncNTP",
    4096,
    NULL,
    1,
    NULL
  );

  xTaskCreate(
    vDisplayTime,
    "displayTime",
    4096,
    NULL,
    3,
    NULL
  );

  xTaskCreate(
    vSunrise,
    "Sunrise",
    4096,
    NULL,
    1,
    &xSunriseTaskHandle
  );

}

void loop() {
  // put your main code here, to run repeatedly:
}

