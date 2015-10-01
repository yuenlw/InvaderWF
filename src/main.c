#include <pebble.h>

#define COLORS       true
#define ANTIALIASING true

#define ANIMATION_DURATION 500
#define ANIMATION_DELAY    600

typedef struct {
  int hours;
  int minutes;
} Time;

static Window *s_main_window;
static Layer *s_window_layer;

static GBitmap  *s_hour_invader;
static GBitmap  *s_minute_12_invader;
static GBitmap  *s_minute_5_invader;
static GBitmap  *s_ship;

static Time s_last_time;

static TextLayer *s_date_layer;
static GFont s_date_font;
static char s_date_buffer[16];


/************************************ UI **************************************/

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;

  // Copy date into buffer from tm structure
  
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d %b", tick_time);

 
  // Redraw
  if(s_window_layer) {
    layer_mark_dirty(s_window_layer);
  }
}

static void update_proc(Layer *layer, GContext *ctx) {
  // Color background?
  layer_remove_child_layers(layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
  
   // Show the date
  text_layer_set_text(s_date_layer, s_date_buffer);
  layer_add_child(layer, text_layer_get_layer(s_date_layer));
  
  //fill hours
  for(int i = 1; i <= s_last_time.hours  ; i++){
    GPoint origin = GPoint( 3 + ( ((i-1)%6 ) * 23), 25 + (16 * (i < 7 ? 0 : 1)));
      
    BitmapLayer *s_bitmap_layer;
    s_bitmap_layer = bitmap_layer_create(GRect(origin.x, origin.y, 22, 18));
    bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
    bitmap_layer_set_background_color(s_bitmap_layer, GColorClear);
    bitmap_layer_set_bitmap(s_bitmap_layer, s_hour_invader);
   
    layer_add_child(layer, bitmap_layer_get_layer(s_bitmap_layer));
  }

  //fill minutes
  int minutes_12 = s_last_time.minutes / 5 ;
  for(int i = 1; i <= minutes_12  ; i++){
    GPoint origin = GPoint( 3 + ( ((i-1)%6 ) * 23), 25 + (16 * (i < 7 ? 2 : 3)));
      
    BitmapLayer *s_bitmap_layer;
    s_bitmap_layer = bitmap_layer_create(GRect(origin.x, origin.y, 22, 18));
    bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
    bitmap_layer_set_background_color(s_bitmap_layer, GColorClear);
    bitmap_layer_set_bitmap(s_bitmap_layer, s_minute_12_invader);
   
    layer_add_child(layer, bitmap_layer_get_layer(s_bitmap_layer));
  }
  
  //fill minutes
  int minutes_5 = s_last_time.minutes % 5 ;
  for(int i = 1; i <= minutes_5  ; i++){
    GPoint origin = GPoint( 3 + (i * 23), 25 + (16 * 4));
      
    BitmapLayer *s_bitmap_layer;
    s_bitmap_layer = bitmap_layer_create(GRect(origin.x, origin.y, 22, 18));
    bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
    bitmap_layer_set_background_color(s_bitmap_layer, GColorClear);
    bitmap_layer_set_bitmap(s_bitmap_layer, s_minute_5_invader);
   
    layer_add_child(layer, bitmap_layer_get_layer(s_bitmap_layer));
  }
  
  //ship
  int i = rand() % 6;
  GPoint origin = GPoint( 3 + (i * 23), 25 + (16 * 6));
  BitmapLayer *s_bitmap_layer;
  s_bitmap_layer = bitmap_layer_create(GRect(origin.x, origin.y, 22, 18));
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  bitmap_layer_set_background_color(s_bitmap_layer, GColorClear);
  bitmap_layer_set_bitmap(s_bitmap_layer, s_ship);
   
  layer_add_child(layer, bitmap_layer_get_layer(s_bitmap_layer));
  
  
}
static void load_images(){
   // Load the image and check it was succcessful
  s_hour_invader = gbitmap_create_with_resource(RESOURCE_ID_INVADER_PINK);  
  s_minute_12_invader = gbitmap_create_with_resource(RESOURCE_ID_INVADER_BLUE);  
  s_minute_5_invader = gbitmap_create_with_resource(RESOURCE_ID_INVADER_YELLOW);  
  s_ship = gbitmap_create_with_resource(RESOURCE_ID_SHIP);  
  
}

static void window_load(Window *window) {
  s_window_layer = window_get_root_layer(window);
  
  load_images();
  
  layer_set_update_proc(s_window_layer, update_proc);
  
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 0, 144, 25));
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

 
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DOS_FONT_16));
  text_layer_set_font(s_date_layer, s_date_font);

  
}

static void window_unload(Window *window) {

  
}

/*********************************** App **************************************/


static void init() {
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, MINUTE_UNIT);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  fonts_unload_custom_font(s_date_font);
  text_layer_destroy(s_date_layer);
  
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}

