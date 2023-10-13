// /*Using LVGL with Arduino requires some extra steps:
//  *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */

// #include <lvgl.h>
// #include <TFT_eSPI.h>

// /*To use the built-in examples and demos of LVGL uncomment the includes below respectively.
//  *You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for the demos `lvgl/demos` to `lvgl/src/demos`.
//  Note that the `lv_examples` library is for LVGL v7 and you shouldn't install it for this version (since LVGL v8)
//  as the examples and demos are now part of the main LVGL library. */

// #include <examples/lv_examples.h>
// #include <demos/lv_demos.h>

// /*Change to your screen resolution*/
// static const uint16_t screenWidth  = 240;
// static const uint16_t screenHeight = 280;

// static lv_disp_draw_buf_t draw_buf;
// static lv_color_t buf[ screenWidth * screenHeight / 10 ];

// TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

// #if LV_USE_LOG != 0
// /* Serial debugging */
// void my_print(const char * buf)
// {
//     Serial.printf(buf);
//     Serial.flush();
// }
// #endif

// /* Display flushing */
// void my_disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
// {
//     uint32_t w = ( area->x2 - area->x1 + 1 );
//     uint32_t h = ( area->y2 - area->y1 + 1 );

//     tft.startWrite();
//     tft.setAddrWindow( area->x1, area->y1, w, h );
//     tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
//     tft.endWrite();

//     lv_disp_flush_ready( disp_drv );
// }


// void setup()
// {
//     Serial.begin( 115200 ); /* prepare for possible serial debug */

//     String LVGL_Arduino = "Hello Arduino! ";
//     LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

//     Serial.println( LVGL_Arduino );
//     Serial.println( "I am LVGL_Arduino" );

//     lv_init();

// #if LV_USE_LOG != 0
//     lv_log_register_print_cb( my_print ); /* register print function for debugging */
// #endif

//     tft.begin();          /* TFT init */
//     tft.setRotation( 2 ); /* Portrait orientation */

//     lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

//     /*Initialize the display*/
//     static lv_disp_drv_t disp_drv;
//     lv_disp_drv_init( &disp_drv );
//     /*Change the following line to your display resolution*/
//     disp_drv.hor_res = screenWidth;
//     disp_drv.ver_res = screenHeight;
//     disp_drv.flush_cb = my_disp_flush;
//     disp_drv.draw_buf = &draw_buf;
//     lv_disp_drv_register( &disp_drv );
  
//     /* Create simple label */
//     lv_obj_t *label = lv_label_create( lv_scr_act() );
//     lv_label_set_text( label, "Hello Ardino and LVGL!");
//     lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );
 
//     /* Try an example. See all the examples 
//      * online: https://docs.lvgl.io/master/examples.html
//      * source codes: https://github.com/lvgl/lvgl/tree/e7f88efa5853128bf871dde335c0ca8da9eb7731/examples */
//      //lv_example_btn_1();
//     lv_example_meter_2();
//      /*Or try out a demo. Don't forget to enable the demos in lv_conf.h. E.g. LV_USE_DEMOS_WIDGETS*/
//     //lv_demo_widgets();               
//     // lv_demo_benchmark();          
//     // lv_demo_keypad_encoder();     
//     // lv_demo_music();              
//     // lv_demo_printer();
//     // lv_demo_stress();
    
//     Serial.println( "Setup done" );
// }

// uint32_t lastTick = 0;

// void loop()
// {
//     lv_timer_handler(); /* let the GUI do its work */
//     delay( 5 );
// }





// #if LV_USE_METER && LV_BUILD_EXAMPLES

// static lv_obj_t * meter;

// static void set_value(void *indic, int32_t v)
// {
//     lv_meter_set_indicator_end_value(meter, (lv_meter_indicator_t*)indic, v);
// }

// /**
//  * A meter with multiple arcs
//  */
// void lv_example_meter_2(void)
// {
//     meter = lv_meter_create(lv_scr_act());
//     lv_obj_center(meter);
//     lv_obj_set_size(meter, 200, 200);

//     /*Remove the circle from the middle*/
//     lv_obj_remove_style(meter, NULL, LV_PART_INDICATOR);

//     /*Add a scale first*/
//     lv_meter_scale_t * scale = lv_meter_add_scale(meter);
//     lv_meter_set_scale_ticks(meter, scale, 11, 2, 10, lv_palette_main(LV_PALETTE_GREY));
//     lv_meter_set_scale_major_ticks(meter, scale, 1, 2, 30, lv_color_hex3(0xeee), 15);
//     lv_meter_set_scale_range(meter, scale, 0, 100, 270, 90);

//     /*Add a three arc indicator*/
//     lv_meter_indicator_t * indic1 = lv_meter_add_arc(meter, scale, 10, lv_palette_main(LV_PALETTE_RED), 0);
//     lv_meter_indicator_t * indic2 = lv_meter_add_arc(meter, scale, 10, lv_palette_main(LV_PALETTE_GREEN), -10);
//     lv_meter_indicator_t * indic3 = lv_meter_add_arc(meter, scale, 10, lv_palette_main(LV_PALETTE_BLUE), -20);

//     /*Create an animation to set the value*/
//     lv_anim_t a;
//     lv_anim_init(&a);
//     lv_anim_set_exec_cb(&a, set_value);
//     lv_anim_set_values(&a, 0, 100);
//     lv_anim_set_repeat_delay(&a, 100);
//     lv_anim_set_playback_delay(&a, 100);
//     lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);

//     lv_anim_set_time(&a, 2000);
//     lv_anim_set_playback_time(&a, 500);
//     lv_anim_set_var(&a, indic1);
//     lv_anim_start(&a);

//     lv_anim_set_time(&a, 1000);
//     lv_anim_set_playback_time(&a, 1000);
//     lv_anim_set_var(&a, indic2);
//     lv_anim_start(&a);

//     lv_anim_set_time(&a, 1000);
//     lv_anim_set_playback_time(&a, 2000);
//     lv_anim_set_var(&a, indic3);
//     lv_anim_start(&a);
// }

// #endif