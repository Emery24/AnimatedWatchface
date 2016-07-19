#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;  
static GFont s_time_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GBitmapSequence *s_sequence;
static GBitmap *s_bitmap;

static void timer_handler(void *context);

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // format the way that the time text is going to be displayed
strftime(buffer, sizeof("00:00"),"%I*%M", tick_time);


  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void load_sequence() {
  // Free old data
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "pre gbitmap_sequence_create_with_resource()");
  // Create sequence
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_KOBE_DUNK_GIF);

  // Create GBitmap
APP_LOG(APP_LOG_LEVEL_DEBUG, "pre gbitmap_create_blank()");
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

  // Begin animation
  app_timer_register(1, timer_handler, NULL);
}

static void timer_handler(void *context) {
  uint32_t next_delay;
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Hi");

  // Advance to the next APNG frame, and get the delay for this frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
    // Set the new frame into the BitmapLayer
    bitmap_layer_set_bitmap(s_background_layer, s_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(s_background_layer));

    // Timer for that frame's delay
    app_timer_register(10, timer_handler, NULL);
    
    APP_LOG(APP_LOG_LEVEL_WARNING, "Hello");
  }
  else{
    load_sequence();
  }
}

static void main_window_load(Window *window) {
  // Create sequence
s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_KOBE_DUNK_GIF);

// Create blank GBitmap using APNG frame size
GSize frame_size = gbitmap_sequence_get_bitmap_size(s_sequence);
s_bitmap = gbitmap_create_blank(frame_size, GBitmapFormat8Bit);

  // Create GBitmap, then set to created BitmapLayer
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create time TextLayer
//  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ATE_BIT_24));

  //time layer
  s_time_layer = text_layer_create(GRect(15, 141, 120, 40));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  uint32_t first_delay_ms = 10;

// Schedule a timer to advance the first frame
app_timer_register(first_delay_ms, timer_handler, NULL);

}

static void main_window_unload(Window *window) {
    gbitmap_sequence_destroy(s_sequence);
gbitmap_destroy(s_bitmap);

    // Unload GFont
    fonts_unload_custom_font(s_time_font);
    // Destroy time layer
    text_layer_destroy(s_time_layer);
    
}

//register all events
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
 update_time();
 
}

  


static void init() {
 
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}


//this is the main road map for the app  first it will initialize by running init(), then it will run the app until
//the user exits the app and then will run the deinit() code, which will free up all of the memory that we 
//set aside at the for the app.
int main(void) {
  init();
  app_event_loop();
  deinit();
}

