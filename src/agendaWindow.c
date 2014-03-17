#include <pebble.h>
#include <settings.h>
#include <agendaWindow.h>

Window *agenda_window;
MenuLayer *menu_layer;

void get_event_details(){
        
    DictionaryIterator *iter;

    if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
        app_log(APP_LOG_LEVEL_DEBUG, "agendaWindow.c",364,"App MSG Not ok");
        return;
    }    
    if (dict_write_uint8(iter, GET_EVENT_DETAILS, ((uint8_t)MAX_AGENDA_LENGTH)) != DICT_OK) {
        app_log(APP_LOG_LEVEL_DEBUG, "agendaWindow.c",364,"Dict Not ok");
        return;
    }
    if (app_message_outbox_send() != APP_MSG_OK){
        app_log(APP_LOG_LEVEL_DEBUG, "agendaWindow.c",364,"Message Not Sent");
        return;
    }
    app_log(APP_LOG_LEVEL_DEBUG, "agendaWindow.c",364,"Message Sent");
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
      return 1;
}
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return agendaLength;
}
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
    
    menu_cell_basic_header_draw(ctx, cell_layer, agenda[section_index][0]);
}
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    if(strlen(agenda[cell_index->section][2])>1)
        menu_cell_basic_draw(ctx, cell_layer, agenda[cell_index->section][1], agenda[cell_index->section][2], NULL);
    else
        menu_cell_basic_draw(ctx, cell_layer, agenda[cell_index->section][1], NULL, NULL);

}
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {

}

void agenda_window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    menu_layer = menu_layer_create(bounds);

    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
        .get_num_sections = menu_get_num_sections_callback,
        .get_num_rows = menu_get_num_rows_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_header = menu_draw_header_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
    });

    menu_layer_set_click_config_onto_window(menu_layer, window);

    layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
    
}
void agenda_window_unload(Window *window) {
    menu_layer_destroy(menu_layer);
}
void launchAgenda(){
    window_stack_push(agenda_window, true);
    get_event_details();
}
