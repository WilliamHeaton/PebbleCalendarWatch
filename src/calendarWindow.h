
extern Window *calendar_window;
extern Layer *days_layer;
extern Layer *timeLayer;

void processEventDays(uint16_t dta,uint8_t *encoded,int which);
void updateTime(struct tm * t);

void calendar_window_unload(Window *window);
void calendar_window_load(Window *window);
void get_event_days();
