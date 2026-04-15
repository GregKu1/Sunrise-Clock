#pragma once
#include <Arduino.h>
#include <rbdimmerESP32.h>

extern rbdimmer_channel_t* dimmer_channel;

extern SemaphoreHandle_t xWeekLocker;

typedef struct  // structure holds alarm time and relevant values
{
  
  struct tm alarmTime; // tm struct to store time data
  bool isActive; // alarm is active or not
} alarm_t;

  enum Actions {
    LEFT,
    RIGHT,
    CLICK,
    LONG_PRESS
  };
  
// EXAMPLE USAGE OF STRUCT

/*
alarm_t week[7];  // create array of alarms for the week.
week[0].status = 1; // alarm is active
week[0].alarm.tm_hour = 8;  // 8:30AM
week[0].alarm.tm_min = 30;  
*/