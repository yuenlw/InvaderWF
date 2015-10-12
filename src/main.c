#include <pebble.h>

#define COLORS       true
#define ANTIALIASING true

#define TOP_MARGIN  25
#define LEFT_MARGIN  3
 
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
  

typedef struct {
  int hours;
  int minutes;
} Time;

static Time s_last_time;

static Window *s_main_window;
static Layer *s_window_layer;

static GBitmap  *s_hour_invader;
static GBitmap  *s_minute_12_invader;
static GBitmap  *s_minute_5_invader;
static GBitmap  *s_ship_invader;
static GBitmap  *s_black_invader;

static TextLayer *s_date_layer;
static GFont s_date_font;
static char s_date_buffer[32];

typedef struct bm{
  BitmapLayer* bitmap;
} BM;

static BM s_hours[12];
static BM s_minutes_12[12];
static BM s_minutes_5[4];
static BM s_ship[6];


/************************************ UI **************************************/

void update_proc( Layer* layer, GContext* context )
{
    graphics_context_set_fill_color( context, GColorBlack );
    graphics_fill_rect( context, GRect( 0, 0, 144, 168 ), 0, GCornerNone );
   
}

static void update() {
  
   
  int minutes_12 = s_last_time.minutes / 5 ;
  int minutes_5 = s_last_time.minutes % 5 ;
  int ship = rand() % 6;
  
  
  for(int i = 0; i < 12  ; i++){
    if (i <  s_last_time.hours ){
      bitmap_layer_set_bitmap(s_hours[i].bitmap, s_hour_invader); 
    } else {
      bitmap_layer_set_bitmap(s_hours[i].bitmap, s_black_invader);  
    }
    
    if( i < minutes_12 ){
       bitmap_layer_set_bitmap(s_minutes_12[i].bitmap, s_minute_12_invader);  
    } else {
      bitmap_layer_set_bitmap(s_minutes_12[i].bitmap, s_black_invader);  
    }
    if( i < minutes_5){
       bitmap_layer_set_bitmap(s_minutes_5[i].bitmap, s_minute_5_invader);  
    } else if (i<4){
      bitmap_layer_set_bitmap(s_minutes_5[i].bitmap, s_black_invader);  
    }
    if( i == ship){
       bitmap_layer_set_bitmap(s_ship[i].bitmap, s_ship_invader);  
    } else if (i<6){
      bitmap_layer_set_bitmap(s_ship[i].bitmap, s_black_invader);  
    }
    
  }
  
  
  
}

/*
static void request_weather(){
  text_layer_set_text(s_weather_layer, "Updating...");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, 0, 0);
  app_message_outbox_send();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "weather requested");
}
*/
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
   
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;
  
  update();
  
  // Copy date into buffer from tm structure
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d.%m.", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);
  
}

static void load_images(){
   // Load the image and check it was succcessful
  s_hour_invader = gbitmap_create_with_resource(RESOURCE_ID_INVADER_PINK);  
  s_minute_12_invader = gbitmap_create_with_resource(RESOURCE_ID_INVADER_BLUE);  
  s_minute_5_invader = gbitmap_create_with_resource(RESOURCE_ID_INVADER_YELLOW);  
  s_ship_invader = gbitmap_create_with_resource(RESOURCE_ID_SHIP);  
  s_black_invader = gbitmap_create_with_resource(RESOURCE_ID_BLACK);  
}

static void create_bitmap_layers(){
   for(int i = 0; i < 12  ; i++){
    GPoint origin_hours = GPoint( LEFT_MARGIN + ( (i%6 ) * 23), TOP_MARGIN + (16 * (i < 6 ? 0 : 1)));
      
    s_hours[i].bitmap = bitmap_layer_create(GRect(origin_hours.x, origin_hours.y, 22, 18));
    bitmap_layer_set_compositing_mode(s_hours[i].bitmap, GCompOpSet);
    bitmap_layer_set_background_color(s_hours[i].bitmap, GColorClear);
       
    layer_add_child(s_window_layer, bitmap_layer_get_layer(s_hours[i].bitmap));
     
    GPoint origin_minutes = GPoint( LEFT_MARGIN + ( (i%6 ) * 23), TOP_MARGIN + (16 * (i < 6 ? 2 : 3)));
     
    s_minutes_12[i].bitmap = bitmap_layer_create(GRect(origin_minutes.x, origin_minutes.y, 22, 18));
    bitmap_layer_set_compositing_mode(s_minutes_12[i].bitmap, GCompOpSet);
    bitmap_layer_set_background_color(s_minutes_12[i].bitmap, GColorClear);
       
    layer_add_child(s_window_layer, bitmap_layer_get_layer(s_minutes_12[i].bitmap));
     
  }
  
  for(int i = 0; i < 4  ; i++){
    GPoint origin = GPoint( LEFT_MARGIN + ((i+1) * 23), TOP_MARGIN + (16 * 4));
     
    s_minutes_5[i].bitmap = bitmap_layer_create(GRect(origin.x, origin.y, 22, 18));
    bitmap_layer_set_compositing_mode(s_minutes_5[i].bitmap, GCompOpSet);
    bitmap_layer_set_background_color(s_minutes_5[i].bitmap, GColorClear);
       
    layer_add_child(s_window_layer, bitmap_layer_get_layer(s_minutes_5[i].bitmap));
  }
  
  //ship
  for(int i = 0; i < 6  ; i++){
    GPoint origin = GPoint( LEFT_MARGIN + ( i * 23), TOP_MARGIN + (16 * 7.5));
     
    s_ship[i].bitmap = bitmap_layer_create(GRect(origin.x, origin.y, 22, 18));
    bitmap_layer_set_compositing_mode(s_ship[i].bitmap, GCompOpSet);
    bitmap_layer_set_background_color(s_ship[i].bitmap, GColorClear);
       
    layer_add_child(s_window_layer, bitmap_layer_get_layer(s_ship[i].bitmap));
  }
  
}

static void destroy_bitmap_layers(){
  for(int i = 0; i < 12; ++i){
    bitmap_layer_destroy(s_hours[i].bitmap);
    bitmap_layer_destroy(s_minutes_12[i].bitmap);
    if (i<6){
      bitmap_layer_destroy(s_ship[i].bitmap);
      if (i<4){
        bitmap_layer_destroy(s_minutes_5[i].bitmap);
      }
    }
    
  }
}


static void window_load(Window *window) {
  s_window_layer = window_get_root_layer(window);
  
  load_images();
  create_bitmap_layers();
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 0, 144, 25));
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, ".");

  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DOS_FONT_14));
  text_layer_set_font(s_date_layer, s_date_font);
  layer_add_child(s_window_layer, text_layer_get_layer(s_date_layer));
  
  layer_set_update_proc( window_get_root_layer ( s_main_window ), update_proc );
  //update();
}

static void window_unload(Window *window) {

  
}


/*Communication*/

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char s_weather_buffer[32];

  // Read first item
  Tuple *t = dict_read_first(iterator);

  
  
  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_TEMPERATURE:
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
        break;
      case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
      
    }
    // Look for next item
    t = dict_read_next(iterator);
  }
  // Assemble full string and display
  snprintf(s_weather_buffer, sizeof(s_weather_buffer), "%s, %s", temperature_buffer, conditions_buffer);


}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}



/*********************************** App **************************************/


static void init() {
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
 
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
  
  tick_handler(time_now, MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
}

static void deinit() {
  destroy_bitmap_layers();
  gbitmap_destroy(s_hour_invader);
  gbitmap_destroy(s_minute_12_invader);
  gbitmap_destroy(s_minute_5_invader);
  gbitmap_destroy(s_ship_invader);
  gbitmap_destroy(s_black_invader);
  
  fonts_unload_custom_font(s_date_font);
  text_layer_destroy(s_date_layer);
   
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}

