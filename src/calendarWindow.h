
extern Window *calendar_window;
extern Layer *days_layer;
extern TextLayer *timeLayer;

void setColors(GContext* ctx);
void setInvColors(GContext* ctx);
void days_layer_update_callback(Layer *me, GContext* ctx);
void updateTime(struct tm * t);

void get_event_days();
void processEncoded(uint8_t encoded[42]);
void clearCalEvents();
void monthChanged();
void processEventDays(uint16_t dta,uint8_t *encoded);
void calendar_window_unload(Window *window);
void calendar_window_load(Window *window);

