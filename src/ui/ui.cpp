#include <Arduino.h>
#include "ui.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Gui::Gui(Adafruit_SSD1306* display)
    : cursorpos(0),
      _display(display),
      _state(ScreenOff),
      buttons()
{
    
}
void Gui::takeWeek(alarm_t week[7])
{
    for (int i = 0; i < 7; ++i) {
        buttons[i] = &week[i];   
    }
    buttons[7] = nullptr;      //cancel button
}
void Gui::updateTime(){
    struct tm now;
    getLocalTime(&now);
    //actually show it
}

void Gui::updateGui()
{
    //read input from queue
    
    switch (_state)
    {
    case ScreenOff:
        //turnoff
        return;
        break;
    case ShowTime:
        updateTime();
        break;
    case DaySelect:
    // change to selector scrren put cursor on weekdays
        break;
    case SetAlarm:
    //
        break;
    default:
        break;
    }
    
}