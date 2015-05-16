#include <pebble.h>

#define ANTIALIASING true

#define SECONDS_TRACK_RADIUS 62
#define SECONDS_TRACK_STROKE 5
  
#define MINUTES_TRACK_RADIUS 52
#define MINUTES_TRACK_STROKE 7

#define HOURS_TRACK_RADIUS 38
#define HOURS_TRACK_STROKE 9

#define SECONDS_HAND_RADIUS 3
#define MINUTES_HAND_RADIUS 4
#define HOURS_HAND_RADIUS 5

typedef struct {
  int hours;
  int minutes;
  int seconds;
} Time;

static Window *s_main_window;
static Layer *s_canvas_layer;
static TextLayer *s_time_layer;

static GPoint s_center;
static Time s_last_time;

/************************************ UI **************************************/

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;
  s_last_time.seconds = tick_time->tm_sec;
  
  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
	//update_time();
}

static void update_proc(Layer *layer, GContext *ctx) {
  // Color background?
  graphics_context_set_fill_color(ctx, GColorCadetBlue);
  graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);

  //set colour for tracks
  graphics_context_set_stroke_color(ctx, GColorMidnightGreen );

  graphics_context_set_antialiased(ctx, ANTIALIASING);

  // Draw seconds track
  graphics_context_set_stroke_width(ctx, SECONDS_TRACK_STROKE);
  graphics_draw_circle(ctx, s_center, SECONDS_TRACK_RADIUS);
  
  // Draw minutes track
  graphics_context_set_stroke_width(ctx, MINUTES_TRACK_STROKE);
  graphics_draw_circle(ctx, s_center, MINUTES_TRACK_RADIUS);
  
  // Draw hours track
  graphics_context_set_stroke_width(ctx, HOURS_TRACK_STROKE);
  graphics_draw_circle(ctx, s_center, HOURS_TRACK_RADIUS);

  // Don't use current time while animating
  Time mode_time = s_last_time;
  
  // generate position of hands
    GPoint second_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.seconds / 60) * (int32_t)(SECONDS_TRACK_RADIUS) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.seconds / 60) * (int32_t)(SECONDS_TRACK_RADIUS) / TRIG_MAX_RATIO) + s_center.y,
  };
  
  GPoint minute_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(MINUTES_TRACK_RADIUS) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(MINUTES_TRACK_RADIUS) / TRIG_MAX_RATIO) + s_center.y,
  };
  
  GPoint hour_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.hours/ 12) * (int32_t)(HOURS_TRACK_RADIUS) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.hours / 12) * (int32_t)(HOURS_TRACK_RADIUS) / TRIG_MAX_RATIO) + s_center.y,
  };
	
  graphics_context_set_fill_color(ctx, GColorChromeYellow );
  
  //draw second hand
  graphics_fill_circle(ctx, second_hand, SECONDS_HAND_RADIUS);
  
   //draw minute hand
  graphics_fill_circle(ctx, minute_hand, MINUTES_HAND_RADIUS);
  
  //draw hour hand
  graphics_fill_circle(ctx, hour_hand, HOURS_HAND_RADIUS);
	
	update_time();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_center = grect_center_point(&window_bounds);

  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);
	
// Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 72, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorMidnightGreen);
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	
	// Make sure the time is displayed from the start
  update_time();
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}

/*********************************** App **************************************/

static void init() {
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, SECOND_UNIT);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}

