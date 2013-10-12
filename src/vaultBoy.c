/*
VaultBoy - a pebble watchface by Helco

Version: 1.0
License: GNU GPL v3
*/
#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "config.h"

#ifdef INVERTED
#define INVERTED_OP
#else
#define INVERTED_OP !
#endif

#if defined INVERTED || defined NIGHT_INDICATOR
#define NEED_INVERTER
#endif

#define MY_UUID { 0x63, 0x5D, 0xD4, 0xD3, 0x07, 0x7D, 0x4D, 0x76, 0xA7, 0x57, 0xFC, 0x00, 0x28, 0x8C, 0xF4, 0x91 }
PBL_APP_INFO(MY_UUID,
             "Vault Boy", "Helco",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
BmpContainer background;
RotBmpPairContainer leftContainer;
RotBmpPairContainer rightContainer;
#define hourLayer leftContainer.layer
#define minuteLayer rightContainer.layer
#ifdef NEED_INVERTER
InverterLayer inverterLayer;
#endif

void setHandDirection (RotBmpPairLayer* layer,bool right) {
    RotBmpPairContainer* container;
    if (right)
        container=&rightContainer;
    else
        container=&leftContainer;
    layer->white_layer.bitmap=&container->white_bmp;
    layer->black_layer.bitmap=&container->black_bmp;
    rotbmp_pair_layer_set_src_ic (layer,GPoint((right?9:15),63));
}

void initHand (RotBmpPairLayer* layer) {
    //no need to set a hand direction as this is done in handle_time_tick
    GRect frame=GRect(0,0,144,168);
    layer_set_frame((Layer*)layer,frame);
    layer_set_frame((Layer*)&layer->white_layer,frame);
    layer_set_frame((Layer*)&layer->black_layer,frame);
    GPoint dest_ic=GPoint(144/2,168/2);
    layer->white_layer.dest_ic=dest_ic;
    layer->black_layer.dest_ic=dest_ic;
}

void handle_time_tick (AppContextRef app,PebbleTickEvent* event) {
    //update minute hand
    setHandDirection (&minuteLayer,event->tick_time->tm_min<=30);
    rotbmp_pair_layer_set_angle(&minuteLayer,event->tick_time->tm_min*TRIG_MAX_ANGLE/60);
    layer_mark_dirty((Layer*)&minuteLayer);

    //update hour hand
    if (event->tick_time->tm_hour>=12)
        event->tick_time->tm_hour-=12;
    setHandDirection (&hourLayer,event->tick_time->tm_hour<=6);
    rotbmp_pair_layer_set_angle(&hourLayer,event->tick_time->tm_min*TRIG_MAX_ANGLE/720+event->tick_time->tm_hour*TRIG_MAX_ANGLE/12);
    layer_mark_dirty((Layer*)&hourLayer);

#ifdef NIGHT_INDICATOR
    bool inverted=true;
    if (event->tick_time->tm_hour>=DAY_START&&
        event->tick_time->tm_hour<=DAY_END)
        inverted=false;
    layer_set_hidden((Layer*)&inverterLayer,INVERTED_OP inverted);
#endif
}
    //rotbmp_pair_layer_set_angle(&layer,segment*TRIG_MAX_ANGLE/60);

void handle_init(AppContextRef ctx) {
    window_init(&window, "Vault Boy");
    window_stack_push(&window, true /* Animated */);

    bmp_init_container (RESOURCE_ID_IMAGE_BACKGROUND,&background);
    layer_add_child (&window.layer,(Layer*)&background.layer);

    rotbmp_pair_init_container(RESOURCE_ID_IMAGE_HAND_LEFT_WHITE,RESOURCE_ID_IMAGE_HAND_LEFT_BLACK,&leftContainer);
    rotbmp_pair_init_container(RESOURCE_ID_IMAGE_HAND_RIGHT_WHITE,RESOURCE_ID_IMAGE_HAND_RIGHT_BLACK,&rightContainer);
    initHand (&hourLayer);
    layer_add_child (&window.layer,(Layer*)&hourLayer);
    initHand (&minuteLayer);
    layer_add_child (&window.layer,(Layer*)&minuteLayer);

#ifdef NEED_INVERTER
    inverter_layer_init (&inverterLayer,layer_get_frame(&window.layer));
    layer_add_child (&window.layer,(Layer*)&inverterLayer);
#endif

    //trigger the first draw
    PebbleTickEvent tickEvent;
    PblTm time;
    get_time(&time);
    tickEvent.tick_time=&time;
    handle_time_tick(ctx,&tickEvent);
}

void handle_deinit (AppContextRef ctx) {
    bmp_deinit_container (&background);
    rotbmp_pair_deinit_container (&leftContainer);
    rotbmp_pair_deinit_container (&rightContainer);
}

void pbl_main(void *params) {
    PebbleAppHandlers handlers = {
        .init_handler = &handle_init,
        .deinit_handler = &handle_deinit,

        .tick_info={
            .tick_handler=&handle_time_tick
        }
    };
    app_event_loop(params, &handlers);
}
