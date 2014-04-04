#include <pebble.h>
#include <settings.h>
#include <calendarWindow.h>
#include <agendaWindow.h>




static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
    if (showtime && units_changed & MINUTE_UNIT) {
        updateTime(tick_time);  
        if(!hideagenda && tick_time->tm_min%5==0){
            get_top_event();
        }
    }
    if (units_changed & HOUR_UNIT) {
        get_event_days(0);
    }
    if (units_changed & DAY_UNIT) {
        layer_mark_dirty(days_layer);
    }
}


void my_in_rcv_handler(DictionaryIterator *received, void *context) {
    
//    app_log(APP_LOG_LEVEL_DEBUG, "calendarApp.c",364,"Message Recieved");
    
    Tuple *settings_tuple = dict_find(received, SETTINGS_KEY);
    
    Tuple *event_days_tuple = dict_find(received, EVENT_DAYS_DATA_KEY);
    Tuple *event_days_next_tuple = dict_find(received, EVENT_DAYS_NEXT_DATA_KEY);
    Tuple *event_days_last_tuple = dict_find(received, EVENT_DAYS_LAST_DATA_KEY);
    Tuple *monthyear_tuple = dict_find(received, MONTHYEAR_KEY);
    
    Tuple *event_details_tuple = dict_find(received, EVENT_DETAILS_KEY);
    Tuple *event_details_line1_tuple = dict_find(received, EVENT_DETAILS_LINE1_KEY);
    Tuple *event_details_line2_tuple = dict_find(received, EVENT_DETAILS_LINE2_KEY);
    Tuple *event_details_datel_tuple = dict_find(received, EVENT_DETAILS_DATEL_KEY);
    Tuple *event_details_length_tuple = dict_find(received, EVENT_DETAILS_LENGTH_KEY);
    
    if(settings_tuple) {
        processSettings(settings_tuple->value->data);
    }
    if(event_days_tuple) {
        processEventDays(monthyear_tuple->value->uint16,event_days_tuple->value->data,0);
    }
    if(event_days_next_tuple) {
        processEventDays(monthyear_tuple->value->uint16,event_days_next_tuple->value->data,1);
    }
    if(event_days_next_tuple) {
        processEventDays(monthyear_tuple->value->uint16,event_days_last_tuple->value->data,-1);
    }
    if(event_details_length_tuple && event_details_length_tuple->value->uint16 < MAX_AGENDA_LENGTH) {
        persist_write_int(AGENDALENGTH_KEY,agendaLength = event_details_length_tuple->value->uint16);
        agenda_mark_dirty();
    }
    
    if(event_details_datel_tuple && event_details_tuple->value->uint16 < MAX_AGENDA_LENGTH ) {
        processEventDetails((int)event_details_tuple->value->uint16,
                            event_details_datel_tuple->value->cstring,
                            event_details_line1_tuple->value->cstring,
                            event_details_line2_tuple->value->cstring);
            
    }
}

int main(void) {
    
    getmode();
    
    app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    app_message_register_inbox_received(my_in_rcv_handler);
    
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
    
    calendar_window = window_create();
    window_set_fullscreen(calendar_window, true);
    window_set_window_handlers(calendar_window, (WindowHandlers) {
        .load = calendar_window_load,
        .unload = calendar_window_unload,
    });
    
    agenda_window = window_create();
    window_set_fullscreen(agenda_window, true);
    window_set_window_handlers(agenda_window, (WindowHandlers) {
        .load = agenda_window_load,
        .unload = agenda_window_unload,
    });
    
    window_stack_push(calendar_window, true);
    app_event_loop();
    window_destroy(agenda_window);
    window_destroy(calendar_window);
}
