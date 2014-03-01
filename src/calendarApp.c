#include <pebble.h>
#include <math.h>


#define WATCHMODE false


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


static bool black = true;
static bool grid = true;
static bool invert = true;
static bool showtime = true;
static bool hidelastprev = true;
static bool boldevents = true;

// First day of the week. Values can be between -6 and 6 
// 0 = weeks start on Sunday
// 1 =  weeks start on Monday
static int start_of_week = 0;
static int showweekno = 0;
static int weekstoshow = 0;

static char weekno_form[4][3] = {"","%V","%U","%W"};
const char daysOfWeek[7][3] = {"S","M","T","W","Th","F","S"};
const char months[12][12] = {"January","Feburary","March","April","May","June","July","August","September","October", "November", "December"};

static int offset = 0;

static Window *window;
static Layer *days_layer;

static TextLayer *timeLayer;
int curHour;
int curMin;
int curSec;

bool calEvents[32] = {  false,false,false,false,false,
                        false,false,false,false,false,
                        false,false,false,false,false,
                        false,false,false,false,false,
                        false,false,false,false,false,
                        false,false,false,false,false,false,false};

static int isleap (unsigned yr) {
  return yr % 400 == 0 || (yr % 4 == 0 && yr % 100 != 0);
}

static unsigned months_to_days (unsigned month) {
  return (month * 3057 - 3007) / 100;
}

static unsigned years_to_days (unsigned yr) {
  return yr * 365L + yr / 4 - yr / 100 + yr / 400;
}
static long ymd_to_scalar (unsigned yr, unsigned mo, unsigned day) {
  long scalar;

  scalar = day + months_to_days(mo);
  if (mo > 2) /* adjust if past February */
    scalar -= isleap(yr) ? 1 : 2;
  yr--;
  scalar += years_to_days(yr);
  return scalar;
}
time_t p_mktime (struct tm *timeptr) {
  time_t tt;

  if ((timeptr->tm_year < 70) || (timeptr->tm_year > 120)) {
    tt = (time_t)-1;
  } else {
    tt = ymd_to_scalar(timeptr->tm_year + 1900,
                       timeptr->tm_mon + 1,
                       timeptr->tm_mday)
      - ymd_to_scalar(1970, 1, 1);
    tt = tt * 24 + timeptr->tm_hour;
    tt = tt * 60 + timeptr->tm_min;
    tt = tt * 60 + timeptr->tm_sec;
  }
  return tt;
}

// Calculate what day of the week it was/will be X days from the first day of the month, if mday was a wday
int wdayOfFirst(int wday,int mday){
    int a = wday - ((mday-1)%7);
    if(a<0) a += 7;
    
    return a;
}

// How many days are/were in the month
int daysInMonth(int mon, int year){
    mon++;
    
    // April, June, September and November have 30 Days
    if(mon == 4 || mon == 6 || mon == 9 || mon == 11)
        return 30;
        
    // Deal with Feburary & Leap years
    else if( mon == 2 ){
        if(year%400==0)
            return 29;
        else if(year%100==0)
            return 28;
        else if(year%4==0)
            return 29;
        else 
            return 28;
    }
    // Most months have 31 days
    else
        return 31;
}
void setColors(GContext* ctx){
    if(black){
        window_set_background_color(window, GColorBlack);
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_context_set_text_color(ctx, GColorWhite);
    }else{
        window_set_background_color(window, GColorWhite);
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_context_set_text_color(ctx, GColorBlack);
    }
}
void setInvColors(GContext* ctx){
    if(black){
        window_set_background_color(window, GColorWhite);
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_context_set_text_color(ctx, GColorBlack);
    }else{
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_context_set_text_color(ctx, GColorWhite);
    }
}


void days_layer_update_callback(Layer *me, GContext* ctx) {
    
    // Find the Target Month
    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);
    currentTime->tm_mon=currentTime->tm_mon+offset;
    now = p_mktime(currentTime);
    currentTime = localtime(&now);
    
    // Extract the info we need to know about the month to build a calendar
    int year = currentTime->tm_year+1900;                         // Year
    int mon = currentTime->tm_mon;                                // Month
    int today = currentTime->tm_mday;                             // Day of Month (only relevent if this we are looking at the current month)
    int dom = daysInMonth(mon,year);                              // Days in the target month
    int dlm = daysInMonth(mon==1?12:mon-1,mon==1?year-1:year);    // Days in Last Month

    
    
    // Find the number of weeks that we will be showing,
    // and the day of month that we will start on
        
    // We will always start on the first day of the week
    // if this is has us starting in the previous month,
    // we will just hide those days if, we do not want to show them
    
    int w,start;
    if(weekstoshow==0){
        // Day of the week for the first day in the target month
        int idow = wdayOfFirst(currentTime->tm_wday,currentTime->tm_mday) - start_of_week; if(idow<0) idow+=7;
        
        // Number of weeks that we will be showing
        w = ceil(((float) (idow + dom))/7);
        
        // Day of month that we will start on
        // It should always be the beginning of the week
        start = 1-idow;
    
    }else{
        // Day of the week for the first day in the target month
        int idow = currentTime->tm_wday - start_of_week;if(idow<0) idow+=7;
        
        // Number of weeks that we will be showing
        w = weekstoshow;
        
        // Day of month that we will start on
        // It should always be the beginning of the week
        start = today - idow - 7*floor(weekstoshow/3);
    }
    
    // Find the last day that will show on the calendar
    int end = start + w*7;
    



    // Size & position of Day squares
    // First the constants that setup the screen size
    int l = showweekno>0?0:1;      // position of left side of left column
    int b = 168;                   // position of bottom of bottom row
    int d = showweekno>0?8:7;      // number of columns (days of the week)
    int lw = showweekno>0?18:20;   // width of columns 
    int o = showweekno>0?1:0;      // Offset if showing weeknumber;
    
    // Then the Calculated values
    int bh = 21;                   // height of rows
    if(showtime){
        if(w == 1)      bh = 30;
        else if(w == 2) bh = 30;
        else if(w == 3) bh = 26;
        else if(w == 4) bh = 21;
        else if(w == 5) bh = 17;
        else            bh = 14;
    }else{
        if(w == 1)      bh = 30;
        else if(w == 2) bh = 30;
        else if(w == 3) bh = 30;
        else if(w == 4) bh = 30;
        else if(w == 5) bh = 24;
        else            bh = 20;
    }

    int r = l+d*lw; // position of right side of right column
    int t = b-w*bh; // position of top of top row
    int cw = lw-1;  // width of textarea
    int cl = l+1;
    int ch = bh-1;
    
    
    // Setup font and background colors
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    int fh = 15;
    int fo = 9;
    setColors(ctx);
    
    
    // these are itterators/buffers
    int j,i,day;
    int dow = 0;
    int wknum = 0;
    char buff[8];


    // Now it is time to start drawing
    
    // Draw the Gridlines
    if(grid){
        // horizontal lines
        for(i=1;i<=w;i++){
            graphics_draw_line(ctx, GPoint(0, b-i*bh), GPoint(144, b-i*bh));
        }
        // vertical lines
        for(i=1;i<d;i++){
            graphics_draw_line(ctx, GPoint(l+i*lw, t), GPoint(l+i*lw, b));
        }
    }

    // Draw days of week
    for(i=0;i<7;i++){
    
        // Adjust labels by specified offset
        j = i+start_of_week;
        if(j>6) j-=7;
        if(j<0) j+=7;
        graphics_draw_text(
            ctx, 
            daysOfWeek[j], 
            fonts_get_system_font(FONT_KEY_GOTHIC_14), 
            GRect(cl+(i+o)*lw, b-w*bh-16, cw, 15), 
            GTextOverflowModeWordWrap, 
            GTextAlignmentCenter, 
            NULL); 
    }
        
    // Fill in the cells with the month days
    for(i=start;i<=end;i++){
    
        // New Weeks begin on Sunday
        if(dow > 6){
            dow = 0;
            wknum ++;
        }        
        
        // If this is the first day of the week draw week numbers
        if(showweekno>0 && dow==0){
                        
            // Find the day that we will base the weeknumber on
            // If this is the week of Jan 1, use Jan 1
            // Otherwise, find the monday in this row
            currentTime->tm_year = year - 1900;
            currentTime->tm_mon = mon;
            currentTime->tm_mday = (mon==0 && i<1  && i < start_of_week )?1:(i - start_of_week + ((start_of_week>2)?8:1));

            now = p_mktime(currentTime);
            strftime(buff, sizeof(buff), weekno_form[showweekno], localtime(&now));

            graphics_draw_text(
                ctx, 
                buff,  
                font, 
                GRect(
                    cl, 
                    b-(-0.5+w-wknum)*bh-fo, 
                    cw, 
                    fh), 
                GTextOverflowModeWordWrap, 
                GTextAlignmentCenter, 
                NULL); 
        }


        // Is this today?  If so prep special today style
        if(invert && i==today && offset == 0){
            setInvColors(ctx);
            graphics_fill_rect(
                ctx,
                GRect(
                    l+(dow+o)*lw+1 + ((dow==0&&d==7)?-2:0), 
                    b-(w-wknum)*bh+1, 
                    cw + ((dow==0&&d==7)?2:0) + ((dow==6)?3:0), 
                    ch)
                ,0
                ,GCornerNone);
        }
    
        // Is there an event today? If so prep special event style
        if(boldevents && i>0 && i<=dom && calEvents[i-1]){
            font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
            fh = 19;
            fo = 12;        
        }
        
        // Get the number that we will show
                
        // Days from last month
        if(i<1){
            if(hidelastprev){
                dow++;
                continue;
            }else{
                day = dlm+i;
            
            }
            
        // Days from next month
        }else if(i>dom){
            if(hidelastprev){
                dow++;
                continue;
            }else{
                day = i-dom;
            }
            
        // Days from this month
        }else{
            day = i;
        
        }
        snprintf(buff,sizeof(buff),"%d", day);

        // Draw the day
        graphics_draw_text(
            ctx, 
            buff,  
            font, 
            GRect(
                cl+(dow+o)*lw, 
                b-(-0.5+w-wknum)*bh-fo, 
                cw, 
                fh), 
            GTextOverflowModeWordWrap, 
            GTextAlignmentCenter, 
            NULL); 
        

        // Reset Styles
        setColors(ctx);
        font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
        fh = 15;
        fo = 9;

        // and on to the next day
        dow++;   
    }
    
    
    // Build the MONTH YEAR string
    char str[20];
#if WATCHMODE
    strftime(str, sizeof(str), "%B %d, %Y", currentTime);
#else

    currentTime->tm_year = year - 1900;
    currentTime->tm_mon = mon;
    currentTime->tm_mday = 1;
    strftime(str, sizeof(str), "%B %Y",currentTime );

#endif

    GRect rec = GRect(0, 0, 144, 25);
    if(showtime)
        rec = GRect(0, 40, 144, 25);
    
    // Draw the MONTH/YEAR String
    graphics_draw_text(
        ctx, 
        str,  
        fonts_get_system_font(FONT_KEY_GOTHIC_24), 
        rec,
        GTextOverflowModeWordWrap, 
        GTextAlignmentCenter, 
        NULL);
}

void updateTime(struct tm * t){

    curHour=t->tm_hour;
    curMin=t->tm_min;
    curSec=t->tm_sec;
    
    static char timeText[] = "00:00";
    if(clock_is_24h_style()){
        strftime(timeText, sizeof(timeText), "%H:%M", t);
        if(curHour<10)memmove(timeText, timeText+1, strlen(timeText)); 
    }else{
        strftime(timeText, sizeof(timeText), "%I:%M", t);
        if( (curHour > 0 && curHour<10) || (curHour>12 && curHour<22))memmove(timeText, timeText+1, strlen(timeText)); 
    }
    text_layer_set_text(timeLayer, timeText);
    
}

static void get_settings(){
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
static void send_cmd(){
        
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
    if(year*100+month>0){
        DictionaryIterator *iter;

        if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
            app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"App MSG Not ok");
            return;
        }    
        if (dict_write_uint16(iter, GET_EVENT_DAYS, ((uint16_t)year*100+month)) != DICT_OK) {
            app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"Dict Not ok");
            return;
        }
        if (app_message_outbox_send() != APP_MSG_OK){
            app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"Message Not Sent");
            return;
        }
        app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"Message Sent");
    }
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
            window_set_background_color(window, GColorBlack);
        else
            window_set_background_color(window, GColorWhite);
        
        
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
void processEncoded(uint8_t encoded[42]){
    int index;
    for (int byteIndex = 0;  byteIndex < 4; byteIndex++){
        for (int bitIndex = 0;  bitIndex < 8; bitIndex++){
            index = byteIndex*8+bitIndex;
            calEvents[index] = (encoded[byteIndex] & (1 << bitIndex)) != 0;
        }
    }
}
void clearCalEvents(){

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
    
    for (int i = 0; i < 31; ++i){
        calEvents[i] = false;
    }
    
    if(year*100+month > 0 && persist_exists(year*100+month)){
        uint8_t encoded[42];
        persist_read_data(year*100+month, encoded, 8);
        processEncoded(encoded);
    }

}

#if !WATCHMODE
static void monthChanged(){

    clearCalEvents();
    send_cmd();
    
    layer_mark_dirty(days_layer);
}
#endif

void processEventDays(uint16_t dta,uint8_t *encoded){
    int m = dta%100;
    int y = (dta-m)/100;
    
    persist_write_data(dta, encoded, sizeof(encoded));
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
    if((m==month && y == year) ){
        processEncoded(encoded);
        layer_mark_dirty(days_layer);
    }else{
        send_cmd();
    }
}
void clearEventDetails(){
    for(int i=100;i<200;i++){
        if( persist_exists(i) )
            persist_delete(i);
    }
}

void my_in_rcv_handler(DictionaryIterator *received, void *context) {
    
    app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"Message Recieved");
    
    Tuple *settings_tuple = dict_find(received, SETTINGS_KEY);
    
    Tuple *event_days_tuple = dict_find(received, EVENT_DAYS_DATA_KEY);
    Tuple *monthyear_tuple = dict_find(received, MONTHYEAR_KEY);
    
    Tuple *event_details_tuple = dict_find(received, EVENT_DETAILS_KEY);
    Tuple *event_details_line1_tuple = dict_find(received, EVENT_DETAILS_LINE1_KEY);
    Tuple *event_details_line2_tuple = dict_find(received, EVENT_DETAILS_LINE2_KEY);
    Tuple *event_details_clear_tuple = dict_find(received, EVENT_DETAILS_CLEAR_KEY);
    
    
    if (settings_tuple) {
        processSettings(settings_tuple->value->data);
    }
    if (event_details_clear_tuple) {
        clearEventDetails();
    }
    if (event_days_tuple) {
        processEventDays(monthyear_tuple->value->uint16,event_days_tuple->value->data);
    }
    if (event_details_line1_tuple) {
        persist_write_string(100+(event_details_tuple->value->uint16), event_details_line1_tuple->value->cstring);
    }
    if (event_details_line2_tuple) {
        persist_write_string(200+(event_details_tuple->value->uint16), event_details_line2_tuple->value->cstring);
    }
}


#if !WATCHMODE
void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    offset--;
    monthChanged();
}
void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    offset++;
    monthChanged();
}
void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    if(offset != 0){
        offset = 0;
        monthChanged();
    }
}
static void click_config_provider(void* context){
    
    window_single_click_subscribe(          BUTTON_ID_SELECT,  select_single_click_handler);
// Up and Down should be repeating clicks, but due to bug in 2.0 firmware repeating clicks cause watch to crash
  window_single_repeating_click_subscribe(BUTTON_ID_UP  , 100,   up_single_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_single_click_handler);
//    window_single_click_subscribe(BUTTON_ID_UP  ,   up_single_click_handler);
//    window_single_click_subscribe(BUTTON_ID_DOWN,   down_single_click_handler);

   
}
#endif

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
    if (showtime && units_changed & MINUTE_UNIT) {
        updateTime(tick_time);  
    }
    if (units_changed & HOUR_UNIT) {
        send_cmd();
    }
    if (units_changed & DAY_UNIT) {
        layer_mark_dirty(days_layer);
    }
}

void init() {
    
    if( persist_exists(BLACK_KEY))          black =         persist_read_bool(BLACK_KEY);
    if( persist_exists(GRID_KEY))           grid =          persist_read_bool(GRID_KEY);
    if( persist_exists(INVERT_KEY))         invert =        persist_read_bool(INVERT_KEY);
    if( persist_exists(SHOWTIME_KEY))       showtime =      persist_read_bool(SHOWTIME_KEY);
    if( persist_exists(HIDELASTPREV_KEY))   hidelastprev =  persist_read_bool(HIDELASTPREV_KEY);
    if( persist_exists(BOLDEVENTS_KEY))     boldevents =    persist_read_bool(BOLDEVENTS_KEY);
    if( persist_exists(START_OF_WEEK_KEY))  start_of_week = persist_read_int(START_OF_WEEK_KEY);
    if( persist_exists(SHOWWEEKNO_KEY))     showweekno =    persist_read_int(SHOWWEEKNO_KEY);
    if( persist_exists(WEEKSTOSHOW_KEY))    weekstoshow =   persist_read_int(WEEKSTOSHOW_KEY);
    
    
    clearCalEvents();
    app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    app_message_register_inbox_received(my_in_rcv_handler);
    
    window = window_create();
    
    window_set_fullscreen(window, true);
#if !WATCHMODE
    window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);
#endif


    
    if(black)
        window_set_background_color(window, GColorBlack);
    else
        window_set_background_color(window, GColorWhite);

    window_stack_push(window, false);
    
    Layer *window_layer = window_get_root_layer(window);
    
    days_layer = layer_create(layer_get_bounds(window_layer));
    layer_set_update_proc(days_layer, days_layer_update_callback);
    layer_add_child(window_layer, days_layer);


    timeLayer = text_layer_create( GRect(0, -6, 144, 43));
    if(black)
        text_layer_set_text_color(timeLayer, GColorWhite);
    else
        text_layer_set_text_color(timeLayer, GColorBlack);

    text_layer_set_background_color(timeLayer, GColorClear);
    text_layer_set_font(timeLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
    text_layer_set_text_alignment(timeLayer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(timeLayer));
    
    if(showtime){
        time_t now = time(NULL);
        struct tm *currentTime = localtime(&now);
        updateTime(currentTime);
    }

    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);

    get_settings();
}

static void deinit(void) {

    layer_destroy(days_layer);
    text_layer_destroy(timeLayer);
    tick_timer_service_unsubscribe();
    window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
