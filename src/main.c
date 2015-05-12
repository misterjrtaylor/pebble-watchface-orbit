#include <pebble.h>

	
#define COLORS       true
#define ANTIALIASING true

#define HAND_MARGIN  10
#define FINAL_RADIUS 55

#define ANIMATION_DURATION 500
#define ANIMATION_DELAY    600

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

static GPoint s_center;
static Time s_last_time, s_anim_time;
static int s_radius = 0; 
//static bool s_animating = false;

/************************************ UI **************************************/

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
}

static int hours_to_minutes(int hours_out_of_12) {
  return (int)(float)(((float)hours_out_of_12 / 12.0F) * 60.0F);
}

static void update_proc(Layer *layer, GContext *ctx) {
  // Color background?
  if(COLORS) {
    graphics_context_set_fill_color(ctx, GColorCadetBlue);
    graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
  }

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

  // Adjust for minutes through the hour
  float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
  //float hour_angle;
  //if(s_animating) {
    // Hours out of 60 for smoothness
    //hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 60;
  //} else {
    //hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;
  //}
  //hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);
  
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
  
  //GPoint hour_hand = (GPoint) {
  //  .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(s_radius - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.x,
  //  .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(s_radius - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.y,
  //};
  
  //set colour for hands
  //graphics_context_set_stroke_color(ctx, GColorChromeYellow );
	
  graphics_context_set_fill_color(ctx, GColorChromeYellow );
  
  //draw second hand
  graphics_fill_circle(ctx, second_hand, SECONDS_HAND_RADIUS);
  
   //draw minute hand
  graphics_fill_circle(ctx, minute_hand, MINUTES_HAND_RADIUS);
  
  //draw hour hand
  graphics_fill_circle(ctx, hour_hand, HOURS_HAND_RADIUS);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_center = grect_center_point(&window_bounds);

  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
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

