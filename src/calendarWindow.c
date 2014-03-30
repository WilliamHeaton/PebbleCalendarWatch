#include <pebble.h>
#include <math.h>
#include <settings.h>
#include <calendarUtils.h>
#include <calendarWindow.h>
#include <agendaWindow.h>

Window *calendar_window;
Layer *days_layer;
TextLayer *timeLayer;

void setColors(GContext* ctx){
    if(black){
        window_set_background_color(calendar_window, GColorBlack);
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_context_set_text_color(ctx, GColorWhite);
    }else{
        window_set_background_color(calendar_window, GColorWhite);
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_context_set_text_color(ctx, GColorBlack);
    }
}
void setInvColors(GContext* ctx){
    if(black){
        window_set_background_color(calendar_window, GColorWhite);
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
    if(watchmode){
        currentTime->tm_year = year - 1900;
        currentTime->tm_mon = mon;
        currentTime->tm_mday = today;
        strftime(str, sizeof(str), "%B %d, %Y", currentTime);
    }else{
        currentTime->tm_year = year - 1900;
        currentTime->tm_mon = mon;
        currentTime->tm_mday = 1;
        strftime(str, sizeof(str), "%B %Y",currentTime );
    }

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


void get_event_days(){
        
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

void monthChanged(){

    clearCalEvents();
    get_event_days();
    
    layer_mark_dirty(days_layer);
}

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
        get_event_days();
    }
}


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
    }else{
        launchAgenda();
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

void calendar_window_unload(Window *window) {
    layer_destroy(days_layer);
    text_layer_destroy(timeLayer);
    tick_timer_service_unsubscribe();
}

void calendar_window_load(Window *window) {
    readSettings();
    
    clearCalEvents();
    
    if(!watchmode){
        window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);
    }


    if(black)
        window_set_background_color(window, GColorBlack);
    else
        window_set_background_color(window, GColorWhite);

    
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


    get_settings();
}
