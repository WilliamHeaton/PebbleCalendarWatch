#include <pebble.h>
#include <math.h>
#include <settings.h>
#include <calendarUtils.h>
#include <calendarWindow.h>
#include <agendaWindow.h>

Window *calendar_window;
Layer *days_layer;
Layer *timeLayer;
struct tm * curTime;
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
void updateTheTime(){
        time_t now = time(NULL);
        struct tm *currentTime = localtime(&now);
        updateTime(currentTime);
}

void updateTime(struct tm * t){
    curTime = t;
    curHour=t->tm_hour;
    curMin=t->tm_min;
    curSec=t->tm_sec;
    layer_mark_dirty(timeLayer);
}

GFont calendar_agenda_time_font;
GFont calendar_agenda_title_font;
GFont calendar_agenda_date_font;
int calendar_date_height;
int calendar_title_height;
int calendar_time_height;
int calendar_agenda_height = 0;
GRect calendar_date_cell_rect;
GRect calendar_title_cell_rect;
GRect calendar_time_cell_rect;
void calendar_agenda_layout(){
    calendar_agenda_date_font  = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    calendar_agenda_time_font  = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    calendar_agenda_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
    
    calendar_date_height = AGENDA_DATE_H;
    calendar_date_cell_rect  = (GRect){ .origin = (GPoint){.x=AGENDA_PAD_L,.y=-AGENDA_DATE_OFF                  }, .size = (GSize){.h=20,.w=144-2*AGENDA_PAD_L} };
    
    calendar_title_height = graphics_text_layout_get_content_size("Hygjp",       calendar_agenda_title_font, GRect(0,0,144,144), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft).h*AGENDA_ROWS;
    calendar_title_cell_rect = (GRect){ .origin = (GPoint){.x=AGENDA_PAD_L,.y=calendar_date_height             }, .size = (GSize){.h=calendar_title_height,.w=144-2*AGENDA_PAD_L} };
//    calendar_title_height -= 5;
    calendar_time_height  = graphics_text_layout_get_content_size( agendaTime[0], calendar_agenda_time_font,  GRect(0,0,144,144), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft).h;
    calendar_time_cell_rect =  (GRect){ .origin = (GPoint){.x=AGENDA_PAD_L,.y=calendar_date_height+calendar_title_height-5}, .size = (GSize){.h=calendar_time_height, .w=144-2*AGENDA_PAD_L} };

    calendar_agenda_height = calendar_date_height + calendar_title_height + calendar_time_height+AGENDA_BOTTOM_PAD-5;
}

void draw_first_event(GContext* ctx){

    calendar_agenda_layout();
    
    setColors(ctx);
    graphics_draw_line(ctx, GPoint(0, calendar_agenda_height),       GPoint(PEB_X,calendar_agenda_height));

    if(!notinvertagenda){
        setInvColors(ctx);
        graphics_fill_rect	(ctx,GRect(0,0,144,calendar_agenda_height),0,GCornerNone);
    }
    graphics_draw_text(
        ctx, 
        agendaDate[0],
        calendar_agenda_date_font, 
        calendar_date_cell_rect, 
        GTextOverflowModeTrailingEllipsis, 
        GTextAlignmentLeft, 
        NULL);
    
    graphics_draw_text(
        ctx, 
        agendaTitle[0],
        calendar_agenda_title_font, 
        calendar_title_cell_rect, 
        GTextOverflowModeTrailingEllipsis, 
        GTextAlignmentLeft, 
        NULL);
            
    if(strlen(agendaTime[0])>1){
        graphics_draw_text(
            ctx, 
            agendaTime[0],
            calendar_agenda_time_font, 
            calendar_time_cell_rect, 
            GTextOverflowModeTrailingEllipsis, 
            GTextAlignmentLeft,
            NULL);    
    }
    
}
void draw_date(GContext* ctx, struct tm *currentTime, int cal_y){

        char str[20];
        if(watchmode){
            strftime(str, sizeof(str), "%B %d, %Y", currentTime);
        }else{
            strftime(str, sizeof(str), "%B %Y",currentTime );
        }
        
        
        int date_h = graphics_text_layout_get_content_size(
                        str,
                        fonts_get_system_font(FONT_KEY_GOTHIC_24), 
                        GRect(0,0,PEB_X,PEB_Y),
                        GTextOverflowModeWordWrap, 
                        GTextAlignmentCenter).h;
        
    
        
        // Draw the MONTH/YEAR String
        graphics_draw_text(
            ctx, 
            str,  
            fonts_get_system_font(FONT_KEY_GOTHIC_24),
            GRect(0,cal_y-DATE_H-DATE_OFF,PEB_X,date_h),
            GTextOverflowModeWordWrap, 
            GTextAlignmentCenter, 
            NULL);
        
}
void draw_labels(GContext* ctx, int cal_x, int cal_y, int cell_x){
    int i,j;
    int o = showweekno>0?1:0;      // Offset if showing weeknumber;
    
    for(i=0;i<7;i++){
        
        // Adjust labels by specified offset
        j = i+start_of_week;
        if(j>6) j-=7;
        if(j<0) j+=7;
        graphics_draw_text(
            ctx, 
            daysOfWeek[j], 
            fonts_get_system_font(FONT_KEY_GOTHIC_14), 
            GRect(cal_x+1+(i+o)*cell_x, cal_y-16, cell_x-1, 15), 
            GTextOverflowModeWordWrap, 
            GTextAlignmentCenter, 
            NULL); 
    }
}
void draw_grid( GContext* ctx,
                int rows,   int cols,
                int cell_x, int cell_y,
                int cal_x,  int cal_y){
    int i;        
    // horizontal lines
    for(i=1;i<=rows;i++){
        graphics_draw_line(ctx, GPoint(0, PEB_Y-i*cell_y), GPoint(PEB_X, PEB_Y-i*cell_y));
    }
    // vertical lines
    for(i=1;i<cols;i++){
        graphics_draw_line(ctx, GPoint(cal_x+i*cell_x, cal_y), GPoint(cal_x+i*cell_x, PEB_Y));
    }
}
void days_layer_update_callback(Layer *me, GContext* ctx) {
    
    // Find the Target Month
    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);
    currentTime->tm_mon=currentTime->tm_mon+offset;
    if(offset!=0)
        currentTime->tm_mday=1;
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
    
    int rows,start;
    if(weekstoshow==0){
        // Day of the week for the first day in the target month
        int idow = wdayOfFirst(currentTime->tm_wday,currentTime->tm_mday) - start_of_week; if(idow<0) idow+=7;
        
        // Number of weeks that we will be showing
        rows = ceil(((float) (idow + dom))/7);
        
        // Day of month that we will start on
        // It should always be the beginning of the week
        start = 1-idow;
    
    }else{
        // Day of the week for the first day in the target month
        int idow = currentTime->tm_wday - start_of_week;if(idow<0) idow+=7;
        
        // Number of weeks that we will be showing
        rows = weekstoshow;
        
        // Day of month that we will start on
        // It should always be the beginning of the week
        start = today - idow - 7*floor(weekstoshow/3);
    }
    
    // Find the last day that will show on the calendar
    int end = start + rows*7;
    
    // Size & position of Day squares
    // First the constants that setup the screen size
    int label_h = LABEL_H;
    int time_h = TIME_H;
    int o = showweekno>0?1:0;      // Offset if showing weeknumber;
    
    int cols = showweekno>0?8:7;      // number of columns (days of the week)
    int cell_x = floor(PEB_X/cols);   // width of columns 
    
    
    int cal_x,cal_y;
    
    // Calculate size of calendar
    cal_y = PEB_Y;
    
    if(!hideagenda){
        draw_first_event(ctx);
        cal_y -= calendar_agenda_height;
    }else{
        calendar_agenda_height = 0;    
    }
    
    // If showing Month/Date, subtract height of month/date
    if(showdate){
        cal_y -= (!showtime && !hideagenda)?DATE_H_WO:DATE_H;
    }
    
    // If showing labels, subtract height of labels
    if(showlabels){
        cal_y -= LABEL_H;
    }else{
        label_h = 0;
    }
    
    // if showing time, subtract height of time
    if(showtime){
        cal_y -= time_h;
        updateTheTime();
    }else{
        time_h = 0;
    }
    
    // height of rows
    int cell_y = floor(cal_y/rows);
    
    // I don't like rows taller than 30
    if(cell_y>30)
        cell_y = 30;
    
    
    // Position of calendar
    cal_y = PEB_Y-rows*cell_y;
    cal_x = floor((PEB_X-1-(cell_x*cols))/2);
    
    
    // Setup font and background colors
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    int fh = 15;
    int fo = 9;
    setColors(ctx);
    
    
    // these are itterators/buffers
    int i,day;
    int dow = 0;
    int wknum = 0;
    char buff[8];
    
    // Draw the Gridlines
    if(grid){
        draw_grid(ctx,rows,cols,cell_x,cell_y,cal_x,cal_y);
    }
    
    // Draw days of week
    if(showlabels){
        draw_labels(ctx,cal_x,cal_y,cell_x);
        
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
                    cal_x+1, 
                    PEB_Y-(-0.5+rows-wknum)*cell_y-fo, 
                    cell_x-1, 
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
                    cal_x+(dow+o)*cell_x+1 + ((dow==0&&cols==7)?-2:0), 
                    PEB_Y-(rows-wknum)*cell_y+1, 
                    cell_x-1 + ((dow==0&&cols==7)?2:0) + ((dow==6)?3:0), 
                    cell_y-1)
                ,0
                ,GCornerNone);
        }
    
        // Is there an event today? If so prep special event style
//        log_int(dlm+i-1)
        if(boldevents && (( i>0 && ((i<=dom && calEvents[i-1]) || (i>dom && nextEvents[i-dom+-1] ))) || (i<=0 && lastEvents[dlm+i-1] )) ){
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
                cal_x+1+(dow+o)*cell_x, 
                PEB_Y-(-0.5+rows-wknum)*cell_y-fo, 
                cell_x-1, 
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
    
    if(showdate){
        currentTime->tm_year = year - 1900;
        currentTime->tm_mon = mon;
        currentTime->tm_mday = today;
        draw_date(ctx,currentTime,cal_y-label_h);
    }  
}

void time_layer_update_callback(Layer *me, GContext* ctx) {

    static char timeText[] = "00:00";
    if(clock_is_24h_style()){
        strftime(timeText, sizeof(timeText), "%H:%M", curTime);
        if(curHour<10)memmove(timeText, timeText+1, strlen(timeText)); 
    }else{
        strftime(timeText, sizeof(timeText), "%I:%M", curTime);
        if( (curHour > 0 && curHour<10) || (curHour>12 && curHour<22))memmove(timeText, timeText+1, strlen(timeText)); 
    }
    int top = -TIME_OFFSET_WO;
    if(!hideagenda){
        top = calendar_agenda_height - TIME_OFFSET_W;
    }
    setColors(ctx);
    graphics_draw_text(
        ctx, 
        timeText,  
        fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS), 
        GRect(0,top,PEB_X,PEB_Y), 
        GTextOverflowModeWordWrap, 
        GTextAlignmentCenter, 
        NULL); 
}



void get_event_days(int o){
        
    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);
        
    int year = currentTime->tm_year;
    int month = currentTime->tm_mon+offset+o;
    
    factorDate(&month,&year);
    
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


bool processEncoded(uint8_t encoded[42],bool decoded[32]){
    int index;
    bool tmp;
    bool changed = false;
    for (int byteIndex = 0;  byteIndex < 4; byteIndex++){
        for (int bitIndex = 0;  bitIndex < 8; bitIndex++){
            index = byteIndex*8+bitIndex;
            tmp = (encoded[byteIndex] & (1 << bitIndex)) != 0;
            if( decoded[index] != tmp){
                decoded[index]  = tmp;
                changed = true;
            }
        }
    }
    return changed;
}

void clearCalEvents(){

    time_t now = time(NULL);
    struct tm *currentTime = localtime(&now);
        
    int year = currentTime->tm_year;
    int month = currentTime->tm_mon+offset ;
    
    factorDate(&month,&year);
    
    for (int i = 0; i < 31; ++i){
        calEvents[i] = false;
        nextEvents[i] = false;
        lastEvents[i] = false;
    }
    
    if(year*100+month > 0 && persist_exists(year*100+month)){
        uint8_t this_month[42];
        persist_read_data(year*100+month, this_month, 8);
        processEncoded(this_month,calEvents);
    }
    month +=1;
    factorDate(&month,&year);
    if(persist_exists(year*100+month)){
        uint8_t next_month[42];
        persist_read_data(year*100+month, next_month, 8);
        processEncoded(next_month,nextEvents);
    
    }
    month -=2;
    factorDate(&month,&year);
    if(persist_exists(year*100+month)){
        uint8_t last_month[42];
        persist_read_data(year*100+month, last_month, 8);
        processEncoded(last_month,lastEvents);
    }

}

void monthChanged(){

    clearCalEvents();
    get_event_days(0);
    
    layer_mark_dirty(days_layer);
}

void processEventDays(uint16_t dta,uint8_t *encoded,int which){
    int m = dta%100;
    int y = (dta-m)/100;
    m += which;
    factorDate(&m,&y);
     
    persist_write_data(y*100+m, encoded, sizeof(encoded));
    time_t now = time(NULL);
    
    struct tm *currentTime = localtime(&now);
    int year = currentTime->tm_year;
    int month = currentTime->tm_mon+offset ;
    factorDate(&month,&year);

    
    if((m==month && y == year) ){
        if(processEncoded(encoded,calEvents))
            layer_mark_dirty(days_layer);
    }else{
        if(which==0){
            get_event_days(0);        
        }
        
        if((m==month+1 && y == year) || (m==0 && y == year+1)){
        
            if(processEncoded(encoded,nextEvents))
                layer_mark_dirty(days_layer);
                
        }else if((m==month-1 && y == year) || (m==11 && y == year-1)){
        
            if(processEncoded(encoded,lastEvents))
                layer_mark_dirty(days_layer);
        }
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
    layer_destroy(timeLayer);
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


    calendar_agenda_layout();
    timeLayer = layer_create(layer_get_bounds(window_layer));
    layer_set_update_proc(timeLayer, time_layer_update_callback);
    layer_add_child(window_layer, timeLayer);
    
    if(showtime){
        updateTheTime();;
        layer_set_hidden(timeLayer,false);
    }else{
            layer_set_hidden(timeLayer,true);
    }


    get_settings();
}
