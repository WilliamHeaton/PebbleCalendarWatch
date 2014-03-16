#include <pebble.h>
#include <settings.h>
#include <calendarUtils.h>
#include <calendarWindow.h>

//Send Keys
#define GET_EVENT_DAYS 1
#define GET_SETTINGS 2
//Recieve Keys
#define MONTHYEAR_KEY 1
#define EVENT_DAYS_DATA_KEY 3
#define SETTINGS_KEY 4
#define EVENT_DETAILS_KEY 5
#define EVENT_DETAILS_LINE1_KEY 6
#define EVENT_DETAILS_LINE2_KEY 7
#define EVENT_DETAILS_CLEAR_KEY 2


//Storage Keys
#define BLACK_KEY 1
#define GRID_KEY 2
#define INVERT_KEY 3
#define SHOWTIME_KEY 4
#define START_OF_WEEK_KEY 5
#define SHOWWEEKNO_KEY 6
#define HIDELASTPREV_KEY 7
#define BOLDEVENTS_KEY 8
#define WEEKSTOSHOW_KEY 9


bool black = true;
bool grid = true;
bool invert = true;
bool showtime = true;
bool hidelastprev = true;
bool boldevents = true;

// First day of the week. Values can be between -6 and 6 
// 0 = weeks start on Sunday
// 1 =  weeks start on Monday
int start_of_week = 0;
int showweekno = 0;
int weekstoshow = 0;

char weekno_form[4][3] = {"","%V","%U","%W"};
char daysOfWeek[7][3] = {"S","M","T","W","Th","F","S"};
char months[12][12] = {"January","Feburary","March","April","May","June","July","August","September","October", "November", "December"};

int offset = 0;


int curHour;
int curMin;
int curSec;

bool calEvents[32] = {  false,false,false,false,false,
                        false,false,false,false,false,
                        false,false,false,false,false,
                        false,false,false,false,false,
                        false,false,false,false,false,
                        false,false,false,false,false,false,false};



void get_settings(){
    DictionaryIterator *iter;
        
    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);
        
    int year = currentTime->tm_year;
    int month = currentTime->tm_mon+offset ;
    
    while(month>11){
        month -= 12;
        year++;
    }
    while(month<0){
        month += 12;
        year--;
    }
    
    if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
        app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"App MSG Not ok");
        return;
    }    
    if (dict_write_uint8(iter, GET_SETTINGS, ((uint16_t)0)) != DICT_OK) {
        app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"Dict Not ok");
        return;
    }
    if(year*100+month>0){
        if (dict_write_uint16(iter, GET_EVENT_DAYS, ((uint16_t)year*100+month)) != DICT_OK) {
            app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"Dict Not ok");
            return;
        }
    }
    if (app_message_outbox_send() != APP_MSG_OK){
        app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"Message Not Sent");
        return;
    }
    app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"Message Sent");
}

void processSettings(uint8_t encoded[2]){

    black           = (encoded[0] & (1 << 0)) != 0;
    grid            = (encoded[0] & (1 << 1)) != 0;
    invert          = (encoded[0] & (1 << 2)) != 0;
    showtime        = (encoded[0] & (1 << 3)) != 0;
    hidelastprev    = (encoded[0] & (1 << 4)) != 0;
    boldevents      = (encoded[0] & (1 << 5)) != 0;

    start_of_week   = (int) encoded[1];
    showweekno      = (int) encoded[2];
    weekstoshow      = (int) encoded[3];

    int changed = false;
    if( ( ! persist_exists(BLACK_KEY)         ) || persist_read_bool(BLACK_KEY)         != black        ){ persist_write_bool(BLACK_KEY,         black);         changed = true;}
    if( ( ! persist_exists(GRID_KEY)          ) || persist_read_bool(GRID_KEY)          != grid         ){ persist_write_bool(GRID_KEY,          grid);          changed = true;}
    if( ( ! persist_exists(INVERT_KEY)        ) || persist_read_bool(INVERT_KEY)        != invert       ){ persist_write_bool(INVERT_KEY,        invert);        changed = true;}
    if( ( ! persist_exists(SHOWTIME_KEY)      ) || persist_read_bool(SHOWTIME_KEY)      != showtime     ){ persist_write_bool(SHOWTIME_KEY,      showtime);      changed = true;}
    if( ( ! persist_exists(HIDELASTPREV_KEY)  ) || persist_read_bool(HIDELASTPREV_KEY)  != hidelastprev ){ persist_write_bool(HIDELASTPREV_KEY,  hidelastprev);  changed = true;}
    if( ( ! persist_exists(BOLDEVENTS_KEY)    ) || persist_read_bool(BOLDEVENTS_KEY)    != boldevents   ){ persist_write_bool(BOLDEVENTS_KEY,    boldevents);    changed = true;}
    if( ( ! persist_exists(START_OF_WEEK_KEY) ) || persist_read_int (START_OF_WEEK_KEY) != start_of_week){ persist_write_int(START_OF_WEEK_KEY,  start_of_week); changed = true;}
    if( ( ! persist_exists(SHOWWEEKNO_KEY)    ) || persist_read_int (SHOWWEEKNO_KEY)    != showweekno   ){ persist_write_int(SHOWWEEKNO_KEY,     showweekno);    changed = true;}
    if( ( ! persist_exists(WEEKSTOSHOW_KEY)   ) || persist_read_int (WEEKSTOSHOW_KEY)   != weekstoshow  ){ persist_write_int(WEEKSTOSHOW_KEY,    weekstoshow);   changed = true;}



    if(changed){
    
        if(black)
            window_set_background_color(calendar_window, GColorBlack);
        else
            window_set_background_color(calendar_window, GColorWhite);
        
        
        if(showtime){
        
            if(black)
                text_layer_set_text_color(timeLayer, GColorWhite);
            else
                text_layer_set_text_color(timeLayer, GColorBlack);
            time_t now = time(NULL);
            struct tm *currentTime = localtime(&now);
            updateTime(currentTime);
        }
        else{
            text_layer_set_text(timeLayer, "");
        }
        layer_mark_dirty(days_layer);
    }
}


