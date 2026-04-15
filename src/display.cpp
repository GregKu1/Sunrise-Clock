#include "display.h"
#include <U8g2lib.h>
#include <Wire.h>

#define SDA 21
#define SCL 22

#define TOP_BAR_FONT u8g2_font_pressstart2p_8u
#define MAIN_FONT  u8g2_font_inr46_mf  


const uint8_t clock_font[1211] U8G2_FONT_SECTION("clock_font") = 
  "\200\0\5\5\5\6\5\5\6\27/\0\0/\0\0\0\3\42\3\342\4\236\0\6\0\200\20\7\1\6\0"
  "\200\20\7\2\6\0\200\20\7\3\6\0\200\20\7\4\6\0\200\20\7\5\6\0\200\20\7\6\6\0\200\20"
  "\7\7\6\0\200\20\7\10\6\0\200\20\7\11\6\0\200\20\7\12\6\0\200\20\7\13\6\0\200\20\7\14"
  "\6\0\200\20\7\15\6\0\200\20\7\16\6\0\200\20\7\17\6\0\200\20\7\20\6\0\200\20\7\21\6\0"
  "\200\20\7\22\6\0\200\20\7\23\6\0\200\20\7\24\6\0\200\20\7\25\6\0\200\20\7\26\6\0\200\20"
  "\7\27\6\0\200\20\7\30\6\0\200\20\7\31\6\0\200\20\7\32\6\0\200\20\7\33\6\0\200\20\7\34"
  "\6\0\200\20\7\35\6\0\200\20\7\36\6\0\200\20\7\37\6\0\200\20\7 \6\0\200\20\7!\6\0"
  "\200\20\7\42\6\0\200\20\7#\6\0\200\20\7$\6\0\200\20\7%\6\0\200\20\7&\6\0\200\20"
  "\7'\6\0\200\20\7(\6\0\200\20\7)\6\0\200\20\7*\6\0\200\20\7+\6\0\200\20\7,"
  "\6\0\200\20\7-\6\0\200\20\7.\6\0\200\20\7/\6\0\200\20\7\60A\326\215\21'\216\221\6"
  "%\250`\250\205%\16\375\265\204)kQLj\220s\214\200\30C \305\30\10\61\10\62\214\202\10\303"
  " \301\70\16j\22\243\226\245\260\244!\16\375\134\302\212@\11jH\306!\0\61)\326\215\21G\5\64"
  "\336\341\220\226\60e-\352\10D:\4\221\214A\244\202\20\211(D\32\14!\311\377\377\377K\37\370\200"
  "\6\62'\326\215\21'\216\221\6%\250`\250\205%\16q\204$/\26\320x\207;\377\277g\300\42\22"
  "\222<\207\346>\360\1\17\63/\326\215\21'\216\221\6%\250`\250\205%\16q\204$\377\213\5,\12"
  "d\236\363\34H\226$\371O\42\16q\11[\224\21(A\15\311\70\4\0\64A\326\215\21\7\204$\247"
  "\10B(\202\20\212 \204\42\10\241\10B(\202\20\212 \204\42\10\241\10B(\202\20\212 \204\42\10"
  "\241\10B(\202\20\212 \204\42\10\241\10\362\201\17h\216\220\344\377\377\10\0\65&\326\215\21\7\77\340"
  "\71t\222\374\237\224\10%\252PE#\26\222\374'\21\207\270\204-\312\10\224\240\206d\34\2\0\66+"
  "\326\215\21'\216\221\6%(Q\250\205%\16%\311\377I\211P\242\12U\60\324\302\22\207\376\317%l"
  "QF\240\4\65$\343\20\0\67\35\326\215\21\7\77\340\71\64GH\362/\26\320x\207;\177\317\200E"
  "$$\371\377-\0\70/\326\215\21'\216\221\6%\250`\250\205%\16\375\277V\4b\25C*\220\201"
  "\212\64\12U\204\202%\16\375\237K\330\242\214@\11jH\306!\0\71+\326\215\21'\216\221\6%\250"
  "`\250\205%\16\375\237K\330\242\214P\23\24\221$\371\77\211\70\304%lQF\240\4\65$\343\20\0"
  ":\25\10\303\34\17F\370@\24\214\17\42F\70\302\7\222`\4\0;\6\0\200\20\7<\6\0\200\20"
  "\7=\6\0\200\20\7>\6\0\200\20\7\77\6\0\200\20\7@\6\0\200\20\7A\6\0\200\20\7B"
  "\6\0\200\20\7C\6\0\200\20\7D\6\0\200\20\7E\6\0\200\20\7F\6\0\200\20\7G\6\0"
  "\200\20\7H\6\0\200\20\7I\6\0\200\20\7J\6\0\200\20\7K\6\0\200\20\7L\6\0\200\20"
  "\7M\6\0\200\20\7N\6\0\200\20\7O\6\0\200\20\7P\6\0\200\20\7Q\6\0\200\20\7R"
  "\6\0\200\20\7S\6\0\200\20\7T\6\0\200\20\7U\6\0\200\20\7V\6\0\200\20\7W\6\0"
  "\200\20\7X\6\0\200\20\7Y\6\0\200\20\7Z\6\0\200\20\7[\6\0\200\20\7\134\6\0\200\20"
  "\7]\6\0\200\20\7^\6\0\200\20\7_\6\0\200\20\7`\6\0\200\20\7a\6\0\200\20\7b"
  "\6\0\200\20\7c\6\0\200\20\7d\6\0\200\20\7e\6\0\200\20\7f\6\0\200\20\7g\6\0"
  "\200\20\7h\6\0\200\20\7i\6\0\200\20\7j\6\0\200\20\7k\6\0\200\20\7l\6\0\200\20"
  "\7m\6\0\200\20\7n\6\0\200\20\7o\6\0\200\20\7p\6\0\200\20\7q\6\0\200\20\7r"
  "\6\0\200\20\7s\6\0\200\20\7t\6\0\200\20\7u\6\0\200\20\7v\6\0\200\20\7w\6\0"
  "\200\20\7x\6\0\200\20\7y\6\0\200\20\7z\6\0\200\20\7{\6\0\200\20\7|\6\0\200\20"
  "\7}\6\0\200\20\7~\6\0\200\20\7\177\6\0\200\20\7\0\0\0\4\377\377\0";




U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL, SDA);

char* day_names[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"}; 
char* month_names[] = {"JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"};

static bool edit_mode = false;
static int edit_start_day;
static int edit_start_offset;

static int light_level = 0;


void ui_init(){
    Wire.begin(SDA, SCL);
    u8g2.begin();
}

void ui_draw_test(){
    u8g2.clearBuffer();
    u8g2.setFont(clock_font);
    u8g2.drawStr(4,64, "12:34");

    u8g2.setFont(TOP_BAR_FONT);
    u8g2.drawStr(0, 8, "HELLO WORLD!");
    u8g2.sendBuffer();
}

void ui_draw_display_mode(struct tm *time_now){
    char* day = day_names[time_now->tm_wday];
    char* month = month_names[time_now->tm_mon];
    int date = time_now->tm_mday;
    int hours = time_now->tm_hour;
    int minutes = time_now->tm_min;

    char buf[32];

    u8g2.clearBuffer();

    u8g2.setFont(TOP_BAR_FONT);
    sprintf(buf,"%s %d %s", day, date, month);
    u8g2.drawStr(0, 8, buf);

    u8g2.setFont(clock_font);
    sprintf(buf, "%02d:%02d", hours, minutes);
    u8g2.drawStr(4, 64, buf);

    u8g2.sendBuffer();
}

void ui_draw_edit_mode(alarm_t *week){
    char* day = day_names[week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_wday ];
    int hours = week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_hour;
    int minutes = week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_min;

    char buf[32];

    u8g2.clearBuffer();

    u8g2.setFont(TOP_BAR_FONT);
    sprintf(buf, "EDITING: %s", day);
    u8g2.drawStr(0, 8, buf);

    u8g2.setFont(clock_font);
    sprintf(buf, "%02d:%02d", hours, minutes);
    u8g2.drawStr(4, 64, buf);

    u8g2.sendBuffer();
}


bool ui_handle_click(alarm_t *week){
    if (ui_is_edit_mode() == true)
    {
        edit_start_offset++;
        Serial.println("NEXT DAY!");
        if (edit_start_offset > 6)
        {
            edit_start_offset = 0;
            edit_mode = false;
            return true;
        }
        return false;
           
    }
    return false;
}

void ui_handle_long_press(alarm_t *week){
    if (ui_is_edit_mode() == false)
    {
        edit_mode = true;
        struct tm time_now;
        getLocalTime(&time_now);

        edit_start_day = time_now.tm_wday;
        edit_start_offset = 0;
    }
    else if (ui_is_edit_mode() == true)
    {
        xSemaphoreTake(xWeekLocker, portMAX_DELAY);
        week[(edit_start_day + edit_start_offset) % 7].isActive = !week[(edit_start_day + edit_start_offset) % 7].isActive;
        xSemaphoreGive(xWeekLocker);
    }

}

void ui_handle_encoder_increment(alarm_t *week){
    if (ui_is_edit_mode() == true)
    {
        xSemaphoreTake(xWeekLocker, portMAX_DELAY);
        int edit_hours = week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_hour;
        int edit_mins = week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_min;

        edit_mins += 5;

        div_t overflow = div(edit_mins, 60);
        edit_hours += overflow.quot;
        edit_mins = overflow.rem;
        edit_hours = edit_hours % 24;

        week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_hour = edit_hours;
        week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_min = edit_mins;

        xSemaphoreGive(xWeekLocker);
    }
    else if (ui_is_edit_mode() == false)
    {
        light_level += 10;
        if (light_level >= 100)
        {
            light_level = 100;
        }
        if (dimmer_channel != NULL)
        {
            rbdimmer_set_level_transition(dimmer_channel, light_level, 100);
        }
        Serial.println(light_level);
        
    }
    
}

void ui_handle_encoder_decrement(alarm_t *week){
    if (ui_is_edit_mode() == true)
    {
        xSemaphoreTake(xWeekLocker, portMAX_DELAY);
        int edit_hours = week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_hour;
        int edit_mins = week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_min;

        edit_mins -= 5;

        if (edit_mins < 0)
        {
            edit_mins += 60;
            edit_hours -= 1;
        }
        
        if (edit_hours < 0)
        {
            edit_hours = 23;
        }
        
        week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_hour = edit_hours;
        week[(edit_start_day + edit_start_offset) % 7].alarmTime.tm_min = edit_mins;
        xSemaphoreGive(xWeekLocker);
    }
    else if (ui_is_edit_mode() == false)
    {
        light_level -= 10;
        if (light_level <= 0)
        {
            light_level = 0;
        }
        if (dimmer_channel != NULL)
        {
            rbdimmer_set_level_transition(dimmer_channel, light_level, 100);
        }
        
        Serial.println(light_level);
        
    }

}

bool ui_is_edit_mode(){
    // Serial.printf("edit mode: %d \n", edit_mode);
    return edit_mode;
}
