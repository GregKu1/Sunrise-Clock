#ifndef GUI_H
#define GUI_H
#endif // GUI_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
typedef struct  // structure holds alarm time and relevant values
{
  
  struct tm alarmTime; // tm struct to store time data
  bool isActive; // alarm is active or not
} alarm_t;

class Gui {
public:
    alarm_t* buttons[8];
    enum State {
        ScreenOff,
        ShowTime,
        DaySelect,
        SetAlarm
    };

    Gui();
    Gui(Adafruit_SSD1306* display);

  

    void updateGui();
    
    void takeWeek(alarm_t[7]);
   
    State getState() const;

private:
    // Private members
    int cursorpos;
    void setState(State newState);
    Adafruit_SSD1306* _display;
    State _state;
    void updateTime();
};

