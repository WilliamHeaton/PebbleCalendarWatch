#include <pebble.h>
#include <math.h>
#include <settings.h>
#include <calendarUtils.h>
#include <calendarWindow.h>

#define WATCHMODE false

void clearEventDetails(){
    for(int i=100;i<200;i++){
        if( persist_exists(i) )
            persist_delete(i);
    }
}

int main(void) {
    calendar_window = window_create();
    window_set_fullscreen(calendar_window, true);
    window_set_window_handlers(calendar_window, (WindowHandlers) {
        .load = calendar_window_load,
        .unload = calendar_window_unload,
    });
    window_stack_push(calendar_window, true);
    app_event_loop();
    window_destroy(calendar_window);
}
