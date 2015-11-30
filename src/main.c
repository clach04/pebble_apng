/*
 * main.c
 * Sets up a Window, BitmapLayer and blank GBitmap to be used as the display
 * container for the GBitmapSequence. It also counts the number of frames.
 *
 * Animation source:
 * http://bestanimations.com/Science/Physics/Physics2.html
 */

#include <pebble.h>

static Window *s_main_window;

static GBitmap *s_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static GBitmapSequence *s_sequence = NULL;

static void load_sequence();

static void timer_handler(void *context) {
  uint32_t next_delay;

APP_LOG(APP_LOG_LEVEL_DEBUG, "timer_handler() ENTRY");
  // Advance to the next APNG frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
APP_LOG(APP_LOG_LEVEL_DEBUG, "in if; pre bitmap_layer_set_bitmap()");
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
APP_LOG(APP_LOG_LEVEL_DEBUG, "in if; pre layer_mark_dirty()");
    layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));

    // Timer for that delay
APP_LOG(APP_LOG_LEVEL_DEBUG, "in if; about to call timer reg");
    app_timer_register(next_delay, timer_handler, NULL);
  } else {
APP_LOG(APP_LOG_LEVEL_DEBUG, "in else");
    // Start again
      /*
      ** Docs say restart, but this does NOT restart after:
      gbitmap_sequence_update_bitmap_next_frame() results in error trace
[ERROR] tmap_sequence.c:219: APNG memory allocation failed
[ERROR] tmap_sequence.c:336: gbitmap_sequence failed to update bitmap

from https://github.com/pebble-hacks/gbitmap-sequence-example/blob/master/src/gbitmap-sequence.c
call load_sequence(); instead
    gbitmap_sequence_restart(s_sequence);
    */
      load_sequence();
  }
APP_LOG(APP_LOG_LEVEL_DEBUG, "timer_handler() EXIT");
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
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ANIMATION);

  // Create GBitmap
APP_LOG(APP_LOG_LEVEL_DEBUG, "pre gbitmap_create_blank()");
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

  // Begin animation
  app_timer_register(1, timer_handler, NULL);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_bitmap_layer = bitmap_layer_create(bounds);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

  load_sequence();
}

static void main_window_unload(Window *window) {
  bitmap_layer_destroy(s_bitmap_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
#ifdef PBL_SDK_2
  window_set_fullscreen(s_main_window, true);
#endif
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
