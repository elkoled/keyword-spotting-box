#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ TFT_WIDTH * TFT_HEIGHT / 10 ];
TFT_eSPI tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT); /* TFT instance */

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp_drv );
}

lv_chart_series_t * ser;
void setup()
{
    Serial.begin( 115200 ); /* prepare for possible serial debug */

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println( LVGL_Arduino );

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

    tft.begin();          /* TFT init */
    tft.setRotation( 4 ); /* Portrait orientation */

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, TFT_WIDTH * TFT_HEIGHT / 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );
  
    ui_init();
    ser = lv_chart_add_series(ui_dbaChart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

    Serial.println( "Setup done" );
}



#define MIN_VALUE 20
#define MAX_VALUE 100

int previous_value = 50; // Starting value

int generate_random_data(void) {
    srand(time(NULL));

    // Generate a random offset between -5 and 5
    int offset = (rand() % 11) - 2; 
    
    // Add the offset to the previous value to get continuity
    int new_value = previous_value + offset;
    
    // Make sure the new value is within the specified range
    if (new_value < MIN_VALUE) {
        new_value = MIN_VALUE;
    } else if (new_value > MAX_VALUE) {
        new_value = MAX_VALUE;
    }
    
    previous_value = new_value;
    
    return new_value;
}

void loop()
{
    int new_value = generate_random_data();
    lv_chart_set_next_value(ui_dbaChart, ser, new_value);
    lv_label_set_text_fmt(ui_dbaValue, "%d", new_value);
    
    lv_timer_handler();
    delay(100);
}