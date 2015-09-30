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

static Time s_last_time, s_anim_time;
static int s_radius = 0, s_anim_hours_60 = 0, s_color_channels[3];
static bool s_animating = false;

/*************************** AnimationImplementation **************************/

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}

static void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_set_implementation(anim, implementation);
  if(handlers) {
    animation_set_handlers(anim, (AnimationHandlers) {
      .started = animation_started,
      .stopped = animation_stopped
    }, NULL);
  }
  animation_schedule(anim);
}

/************************************ UI **************************************/


static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;


  // Redraw
  if(s_window_layer) {
    layer_mark_dirty(s_window_layer);
  }
}

static int hours_to_minutes(int hours_out_of_12) {
  return (int)(float)(((float)hours_out_of_12 / 12.0F) * 60.0F);
}

static void update_proc(Layer *layer, GContext *ctx) {
  // Color background?
  layer_remove_child_layers(layer);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
  
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
  GRect window_bounds = layer_get_bounds(s_window_layer);

  load_images();
  
  layer_set_update_proc(s_window_layer, update_proc);
  
}

static void window_unload(Window *window) {
  
}

/*********************************** App **************************************/

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(float)(((float)dist_normalized / (float)ANIMATION_NORMALIZED_MAX) * (float)max);
}

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
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}

