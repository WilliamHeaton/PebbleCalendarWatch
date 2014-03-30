extern Window *agenda_window;
extern MenuLayer *menu_layer;

void agenda_window_unload(Window *window);
void agenda_window_load(Window *window);
void launchAgenda();
void agenda_mark_dirty();
void processEventDetails(int key, char datel[MAX_AGENDA_TITLE], char title[MAX_AGENDA_TITLE], char timel[MAX_AGENDA_TITLE] );
