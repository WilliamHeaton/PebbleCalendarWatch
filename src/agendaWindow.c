#include <pebble.h>
#include <settings.h>
#include <agendaWindow.h>

Window *agenda_window;
MenuLayer *menu_layer;

void processEventDetails(int key, char datel[MAX_AGENDA_TITLE], char title[MAX_AGENDA_TITLE], char timel[MAX_AGENDA_TITLE]){

    bool changed = false;
    
    if(agendaLength < key){
        persist_write_int(AGENDA_KEY,agendaLength = key);
        changed = true;
    }
    
    if( strcmp(agenda[key][0],datel) !=0 ){
        strcpy(agenda[key][0],datel);
        changed = true;
    }
    if( strcmp(agenda[key][1],title) !=0 ){
        strcpy(agenda[key][1],title);
        changed = true;
    }
    if( strcmp(agenda[key][2],timel) !=0 ){
        strcpy(agenda[key][2],timel);
        changed = true;
    }
    if(changed){
        persist_write_data(AGENDA_KEY+key+1,agenda[key],sizeof(agenda[key]));
        agenda_mark_dirty();
    }

}
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

int agenda_left_padding, agenda_bottom_padding, agenda_title_font_height,agenda_time_padding;
GFont agenda_title_font;
GRect agenda_title_cell_rect;
GFont agenda_time_font;
GRect agenda_time_cell_rect;
void init_agenda_layout(){

    agenda_left_padding = 7;
    agenda_bottom_padding = 10;
    agenda_time_padding = 3;
    agenda_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
    agenda_time_font  = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    agenda_title_font_height = graphics_text_layout_get_content_size("Hygjp",agenda_title_font, GRect(0,0,144,144), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft).h;
    agenda_title_cell_rect = (GRect){ .origin = (GPoint){.x=agenda_left_padding,.y=0}, .size = (GSize){.h=agenda_title_rows*agenda_title_font_height,.w=144-2*agenda_left_padding} };
    agenda_time_cell_rect = (GRect){ .origin = (GPoint){.x=agenda_left_padding,.y=0}, .size = (GSize){.h=168,.w=144-2*agenda_left_padding} };
}
int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context){


    int title_height = graphics_text_layout_get_content_size(
        agenda[cell_index->section][1],
        agenda_title_font, 
        agenda_title_cell_rect, 
        GTextOverflowModeTrailingEllipsis, 
        GTextAlignmentLeft).h;

    int time_height = 0;    
    if(strlen(agenda[cell_index->section][2])>1){
        time_height = graphics_text_layout_get_content_size(
            agenda[cell_index->section][2],
            agenda_time_font, 
            agenda_time_cell_rect, 
            GTextOverflowModeTrailingEllipsis, 
            GTextAlignmentLeft).h + agenda_time_padding;    
    }
        
        
    return title_height + time_height + agenda_bottom_padding;
}
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
      return 1;
}
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return agendaLength+1;
}
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
    
    menu_cell_basic_header_draw(ctx, cell_layer, agenda[section_index][0]);
}
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
        
    int title_height = graphics_text_layout_get_content_size(
        agenda[cell_index->section][1],
        agenda_title_font, 
        agenda_title_cell_rect, 
        GTextOverflowModeTrailingEllipsis, 
        GTextAlignmentLeft).h;
        
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(
        ctx, 
        agenda[cell_index->section][1],
        agenda_title_font, 
        agenda_title_cell_rect, 
        GTextOverflowModeTrailingEllipsis, 
        GTextAlignmentLeft, 
        NULL); 
            
    if(strlen(agenda[cell_index->section][2])>1){
        graphics_draw_text(
            ctx, 
            agenda[cell_index->section][2],
            agenda_time_font, 
            (GRect){ .origin = (GPoint){.x=agenda_left_padding,.y=title_height+agenda_time_padding}, .size = (GSize){.h=168,.w=144-2*agenda_left_padding} }, 
            GTextOverflowModeTrailingEllipsis, 
            GTextAlignmentLeft,
            NULL);    
    }
}
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {

}

void agenda_window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    menu_layer = menu_layer_create(bounds);
    init_agenda_layout();
    
    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
        .get_num_sections = menu_get_num_sections_callback,
        .get_num_rows = menu_get_num_rows_callback,
        .get_cell_height = menu_get_cell_height_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_header = menu_draw_header_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
    });

    menu_layer_set_click_config_onto_window(menu_layer, window);

    layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
    
}

void agenda_mark_dirty(){
    if(menu_layer!=NULL){
        init_agenda_layout();
        menu_layer_reload_data(menu_layer);        
    }
}

void agenda_window_unload(Window *window) {
    menu_layer_destroy(menu_layer);
}
void launchAgenda(){
    window_stack_push(agenda_window, true);
    get_event_details();
}
