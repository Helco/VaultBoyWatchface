/*
VaultBoy - a pebble watchface by Helco

Version: 2.0
License: GNU GPL v3
*/
#include <pebble.h>
#include "config.h"

#ifdef INVERTED
#define INVERTED_OP
#else
#define INVERTED_OP !
#endif

#if defined INVERTED || defined NIGHT_INDICATOR
#define NEED_INVERTER
#endif

enum ImageIds{
  IMAGE_BACKGROUND=0,
  IMAGE_HAND_LEFT_WHITE,
  IMAGE_HAND_LEFT_BLACK,
  IMAGE_HAND_RIGHT_WHITE,
  IMAGE_HAND_RIGHT_BLACK,
  IMAGE_COUNT
};

typedef enum {
  DIRECTION_LEFT=0,
  DIRECTION_RIGHT=1,
  DIRECTION_NOT_SET=2,
} Direction;
typedef struct {
  RotBitmapLayer* white;
  RotBitmapLayer* black;
  Direction dir;
} RotBitmapLayerPair;

Window* window;
GBitmap* images [IMAGE_COUNT];
BitmapLayer* backgroundLayer;
Layer* foregroundLayer;
RotBitmapLayerPair hourLayer={0,0,DIRECTION_NOT_SET};
RotBitmapLayerPair minuteLayer={0,0,DIRECTION_NOT_SET};
bool redoLayering=true;
#ifdef NEED_INVERTER
InverterLayer* inverterLayer;
#endif

void setHandDirection (RotBitmapLayerPair* layer,Direction dir) {
  if (layer->dir!=dir) {
    GRect frame=GRect(0,0,144,168);
    GPoint src_ic=GPoint((dir==DIRECTION_RIGHT?9:15),63);
    GBitmap* imageWhite, *imageBlack;
    redoLayering=true;
    layer->dir=dir;

    if (dir == DIRECTION_RIGHT) {
      imageWhite = images[IMAGE_HAND_RIGHT_WHITE];
      imageBlack = images[IMAGE_HAND_RIGHT_BLACK];
    }
    else {
      imageWhite = images[IMAGE_HAND_LEFT_WHITE];
      imageBlack = images[IMAGE_HAND_LEFT_BLACK];
    }

    if (layer->white != 0)
      rot_bitmap_layer_destroy(layer->white);
    layer->white=rot_bitmap_layer_create (imageWhite);
    rot_bitmap_set_compositing_mode(layer->white,GCompOpOr);
    rot_bitmap_set_src_ic (layer->white,src_ic);
    frame = layer_get_frame((Layer*)layer->white);
    frame.origin.x = 144/2 - frame.size.w/2;
    frame.origin.y = 168/2 - frame.size.h/2;
    layer_set_frame((Layer*)layer->white,frame);
    

    if (layer->black!=0)
      rot_bitmap_layer_destroy(layer->black);
    layer->black=rot_bitmap_layer_create (imageBlack);
    rot_bitmap_set_compositing_mode(layer->black,GCompOpClear);
    rot_bitmap_set_src_ic (layer->black,src_ic);
    frame = layer_get_frame((Layer*)layer->black);
    frame.origin.x = 144/2 - frame.size.w/2;
    frame.origin.y = 168/2 - frame.size.h/2;
    layer_set_frame((Layer*)layer->black,frame);
  }
}
void setHandRotation (RotBitmapLayerPair* layer,int32_t rot) {
  rot_bitmap_layer_set_angle (layer->white,rot);
  rot_bitmap_layer_set_angle (layer->black,rot);
}
void markHandDirty (RotBitmapLayerPair* layer) {
  layer_mark_dirty((Layer*)layer->white);
  layer_mark_dirty((Layer*)layer->black);
}
void destroyHand (RotBitmapLayerPair* layer) {
  rot_bitmap_layer_destroy(layer->white);
  rot_bitmap_layer_destroy(layer->black);
}
void setupLayers () {
  redoLayering=false;
  Layer* rootLayer=window_get_root_layer(window);
  layer_remove_child_layers(rootLayer);
  layer_add_child(rootLayer,(Layer*)backgroundLayer);
  layer_add_child(rootLayer,(Layer*)hourLayer.white);
  layer_add_child(rootLayer,(Layer*)hourLayer.black);
  layer_add_child(rootLayer,(Layer*)minuteLayer.white);
  layer_add_child(rootLayer,(Layer*)minuteLayer.black);
  layer_add_child(rootLayer,foregroundLayer);
#ifdef NEED_INVERTER
  layer_add_child(rootLayer,(Layer*)inverterLayer);
#endif
}

void update_foreground_layer (Layer* me,GContext* ctx) {
  graphics_context_set_fill_color(ctx,GColorBlack);
  graphics_fill_circle(ctx,GPoint(144/2,168/2),4);
}

void handle_time_tick (struct tm* tick_time,TimeUnits units_changed) {
#ifdef NIGHT_INDICATOR
  bool inverted=true;
  if (tick_time->tm_hour>=DAY_START&&tick_time->tm_hour<=DAY_END)
    inverted=false;
  layer_set_hidden((Layer*)inverterLayer,INVERTED_OP inverted);
#endif

  //update minute hand
  setHandDirection (&minuteLayer,tick_time->tm_min<=30);
  setHandRotation(&minuteLayer,tick_time->tm_min*TRIG_MAX_ANGLE/60);
  markHandDirty(&minuteLayer);

  //update hour hand
  if (tick_time->tm_hour>=12)
    tick_time->tm_hour-=12;
  setHandDirection (&hourLayer,tick_time->tm_hour<=6);
  setHandRotation(&hourLayer,tick_time->tm_min*TRIG_MAX_ANGLE/720+tick_time->tm_hour*TRIG_MAX_ANGLE/12);
  markHandDirty(&hourLayer);

  if (redoLayering)
    setupLayers ();
}

void handle_init() {
  window=window_create();

  images[IMAGE_BACKGROUND]=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  images[IMAGE_HAND_LEFT_WHITE]=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAND_LEFT_WHITE);
  images[IMAGE_HAND_LEFT_BLACK]=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAND_LEFT_BLACK);
  images[IMAGE_HAND_RIGHT_WHITE]=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAND_RIGHT_WHITE);
  images[IMAGE_HAND_RIGHT_BLACK]=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAND_RIGHT_BLACK);

  backgroundLayer=bitmap_layer_create(GRect(0,0,144,168));
  bitmap_layer_set_bitmap(backgroundLayer,images[IMAGE_BACKGROUND]);
  foregroundLayer=layer_create(GRect(0,0,144,168));
  layer_set_update_proc(foregroundLayer,update_foreground_layer);

#ifdef NEED_INVERTER
  inverterLayer=inverter_layer_create (GRect(0,0,144,168));
#endif

  window_stack_push(window, true /* Animated */);

  //trigger the first draw
  time_t timeSec=time(0);
  struct tm* time=localtime(&timeSec);
  handle_time_tick(time,0);
  tick_timer_service_subscribe(MINUTE_UNIT,handle_time_tick);
}

void handle_deinit () {
  for (int i=0;i<IMAGE_COUNT;i++)
    gbitmap_destroy(images[i]);
  bitmap_layer_destroy(backgroundLayer);
  layer_destroy(foregroundLayer);
  destroyHand (&hourLayer);
  destroyHand (&minuteLayer);
#ifdef NEED_INVERTER
  inverter_layer_destroy(inverterLayer);
#endif
  window_destroy(window);
}

int main (void) {
  handle_init ();
  app_event_loop();
  handle_deinit();
  return 0;
}
