#include <Arduino.h>
#include <USB.h>
#include <USBCDC.h>
#include <WiFi.h>
#include <time.h>
#include <ESP32Encoder.h>

#include "common.h"
#include "display.h"
#include "credentials/credentials.h"


#define ARDUINO_USB_MODE 1
#define ARDUINO_USB_CDC_ON_BOOT 1

#define NTP "pool.ntp.org", "time.google.com", "time.nist.gov"
#define TZ "GMT0BST,M3.5.0/1,M10.5.0"

#define ENCODER_A 26
#define ENCODER_B 27
#define ENCODER_BUTTON 25


const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;


alarm_t week[7] = {};

// freeRTOS stuff
QueueHandle_t xEncoderQueue;

SemaphoreHandle_t xWeekLocker;

TimerHandle_t xAlarmTimer;

TaskHandle_t xSunriseTaskHandle;

ESP32Encoder encoder;

rbdimmer_channel_t* dimmer_channel;


void recalculateNextAlarm(){
  Serial.println("getting current time and recalculating alarm timer");
  struct tm now;
  getLocalTime(&now);

  int dayOfWeek = now.tm_wday; // sunday = 0
  int secondsUntilNextAlarm;

  xSemaphoreTake(xWeekLocker, portMAX_DELAY);

  bool foundActiveAlarm = false;
  int targetDay = dayOfWeek; // day in the week that we are searching, starting at today
  int daysAhead;  // days ahead from today, starting today


  for (daysAhead = 0; daysAhead < 7; daysAhead++) // check days in the week, starting today and go for 1 week forward
  {
    targetDay = (dayOfWeek + daysAhead) % 7; // wrap around to stay inside the week

    if (week[targetDay].isActive == false)
    {
      continue;
    }
    
    if (daysAhead == 0 &&   // for today, determine if the alarm is ahead or behind
       (now.tm_hour > week[targetDay].alarmTime.tm_hour || // hour is larger OR hour is equal but minutes are higher means alarm behind us
       (now.tm_hour == week[targetDay].alarmTime.tm_hour && now.tm_min >= week[targetDay].alarmTime.tm_min)))
    {
      continue;
    }
    foundActiveAlarm = true;
    break;
    
  }
  
  if (foundActiveAlarm == true)
  {
    getLocalTime(&now);
    int alarm_seconds = week[targetDay].alarmTime.tm_hour * 3600 + week[targetDay].alarmTime.tm_min * 60;
    int now_seconds = now.tm_hour * 3600 + now.tm_min * 60 + now.tm_sec;
    secondsUntilNextAlarm = (daysAhead * 86400) + (alarm_seconds - now_seconds);

    Serial.printf("recalculation complete, time until next alarm is %d seconds \n", secondsUntilNextAlarm);
    xTimerStop(xAlarmTimer, 500);
    xTimerChangePeriod(xAlarmTimer, pdMS_TO_TICKS((uint32_t)secondsUntilNextAlarm*1000), 500);
    xTimerStart(xAlarmTimer, 500);
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
      Serial.println("init=" + rbdimmer_init());
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("regz=" + rbdimmer_register_zero_cross(16, 0, 0));
      rbdimmer_config_t config = {
        .gpio_pin = 17,
        .phase = 0,
        .initial_level = 0,
        .curve_type = RBDIMMER_CURVE_RMS
        };
      Serial.println("crc=" + rbdimmer_create_channel(&config, &dimmer_channel));
      Serial.println("setl=" + rbdimmer_set_level(dimmer_channel, 100));
      Serial.println("freq=" + rbdimmer_get_frequency(0));
      Serial.println("initialised dimmer");

      firstSync = false;
    }
    
    recalculateNextAlarm();
    vTaskDelay(12*60*60*1000 / portTICK_PERIOD_MS); // real
    // vTaskDelay(pdMS_TO_TICKS(30000));  // testing
  }

}


void vDisplayTime(void* pvParameters){ // this will actually handle displaying time on the OLED
  while (1)
  {
    enum Actions received_event;
  
    if (xQueueReceive(xEncoderQueue, &received_event, 0) == pdPASS)
    {
      // Serial.println(received_event);

      switch (received_event)
      {
      case LEFT:
        ui_handle_encoder_decrement(week);
        break;

      case RIGHT:
        ui_handle_encoder_increment(week);
        break;

      case CLICK:
        if (ui_handle_click(week) == true)
        {
          recalculateNextAlarm();
        }
        
        break;
      
      case LONG_PRESS:
        ui_handle_long_press(week);
        break;
      
      default:
        break;
      }
          
    }
    
    // time_t currentTime;
    struct tm timeinfo;
    getLocalTime(&timeinfo);

    // char timeString[64];
    // strftime(timeString, sizeof(timeString), "%c", &timeinfo);

    // Serial.println(timeString);

    // ui_draw_test();

    if (ui_is_edit_mode() == true)
    {
      ui_draw_edit_mode(week);  
    }
    else
    {
      ui_draw_display_mode(&timeinfo);
    }

    Serial.println(xTimerGetExpiryTime(xAlarmTimer) - xTaskGetTickCount());

    vTaskDelay(pdMS_TO_TICKS(10));
  }
  
}


void vTimerCallback(TimerHandle_t xTimer){
  xTaskNotifyGive(xSunriseTaskHandle);

}


void vSunrise(void* pvParameters){  
/* 
- Unblocked by notification from vTimerCallback
- Increases pwm duty cycle over a period
- Blocks itself
*/ 
  while (1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    Serial.println("sunrise started");
    if (dimmer_channel != NULL)
    {
      rbdimmer_set_level_transition(dimmer_channel, 100, 30*60*1000);
    }
    Serial.println("sunrise finished");
    recalculateNextAlarm();

    vTaskDelay(pdMS_TO_TICKS(1*60*60*1000));  // wait 1 hour
    if (dimmer_channel != NULL)
    {
      rbdimmer_set_level(dimmer_channel, 0);
    }
    
  }
  

}


void vReadEncoder(void* pvParameters){
  /*
  - reads encoder
  - converts counts into left/right (0/1) and click (2)
  - sends converted "events" to a queue
  */

  int encoder_pos = 0;
  int prev_encoder_pos = 0;
  int change;

  uint8_t button_state = 0;
  uint8_t prev_button_state = 0;
  const int long_press_delay = 200; //ms
  int t_down = 0;
  int t_up = 0;

  while (1)
  {
    encoder_pos = encoder.getCount() / 4;
    change = encoder_pos - prev_encoder_pos;
    prev_encoder_pos = encoder_pos;

    if (change > 0)
    {
      enum Actions sent_event = RIGHT;
      // Serial.println("increase");
      xQueueOverwrite(xEncoderQueue, &sent_event);
    }
    else if (change < 0)
    {
      enum Actions sent_event = LEFT;
      // Serial.println("decrease");
      xQueueOverwrite(xEncoderQueue, &sent_event);
    }
    

    button_state = !digitalRead(ENCODER_BUTTON);

    if ( (button_state == 1) && (button_state != prev_button_state) )
    {
      t_down = millis();
      prev_button_state = button_state;
      // Serial.println("down");
    }

    if ( (button_state == 0) && (button_state != prev_button_state) )
    {
      t_up = millis();
      prev_button_state = button_state;
      // Serial.println("up");

      if (t_up - t_down > long_press_delay)
      {
        enum Actions sent_event = LONG_PRESS;
        xQueueOverwrite(xEncoderQueue, &sent_event);
        // Serial.println("long press");
      }
      else
      {
        enum Actions sent_event = CLICK;
        xQueueOverwrite(xEncoderQueue, &sent_event);
        // Serial.println("click");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
  
} 


void setup() {
  Serial.begin(115200);

  ui_init();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ENCODER_BUTTON, INPUT_PULLUP);

  encoder.attachFullQuad(ENCODER_A, ENCODER_B);
  encoder.clearCount();

  vTaskDelay(pdMS_TO_TICKS(1000));

  xWeekLocker = xSemaphoreCreateMutex();

  xAlarmTimer = xTimerCreate("AlarmTimer", pdMS_TO_TICKS(1000), pdFALSE, 0, vTimerCallback);

  xEncoderQueue = xQueueCreate(1, sizeof(enum Actions));

  week[0].alarmTime.tm_hour = 6;
  week[0].alarmTime.tm_min = 30;
  week[0].alarmTime.tm_sec = 0;
  week[0].alarmTime.tm_wday = 0;
  week[0].isActive = true;

  week[1].alarmTime.tm_hour = 6;
  week[1].alarmTime.tm_min = 30;
  week[1].alarmTime.tm_sec = 0;
  week[1].alarmTime.tm_wday = 1;
  week[1].isActive = true;

  week[2].alarmTime.tm_hour = 6;
  week[2].alarmTime.tm_min = 30;
  week[2].alarmTime.tm_sec = 0;
  week[2].alarmTime.tm_wday = 2;
  week[2].isActive = true;

  week[3].alarmTime.tm_hour = 6;
  week[3].alarmTime.tm_min = 30;
  week[3].alarmTime.tm_sec = 0;
  week[3].alarmTime.tm_wday = 3;
  week[3].isActive = true;

  week[4].alarmTime.tm_hour = 6;
  week[4].alarmTime.tm_min = 30;
  week[4].alarmTime.tm_sec = 0;
  week[4].alarmTime.tm_wday = 4;
  week[4].isActive = true;

  week[5].alarmTime.tm_hour = 6;
  week[5].alarmTime.tm_min = 30;
  week[5].alarmTime.tm_sec = 0;
  week[5].alarmTime.tm_wday = 5;
  week[5].isActive = true;

  week[6].alarmTime.tm_hour = 6;
  week[6].alarmTime.tm_min = 30;
  week[6].alarmTime.tm_sec = 0;
  week[6].alarmTime.tm_wday = 6;
  week[6].isActive = true;

  xTaskCreate(
    vSyncNTP,
    "syncNTP",
    8192,
    NULL,
    1,
    NULL
  );

  xTaskCreate(
    vDisplayTime,
    "displayTime",
    8192,
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

    xTaskCreate(
    vReadEncoder,
    "Encoder",
    4096,
    NULL,
    1,
    NULL
  );
}



void loop() {
  // put your main code here, to run repeatedly:
}

