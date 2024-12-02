#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "bsp_touch_pad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "lvgl.h"
#include "src/lvgl.h"
#include "./lv_driver/lv_port_disp.h"
#include "src/misc/lv_timer.h"
#include "lv_demos.h"
#include "./lv_driver/lv_port_indev.h"

SemaphoreHandle_t xGuiSemaphore;
QueueHandle_t xtouchpadque;
extern lv_indev_t * indev_encoder;
//#define TAG     "button_cb"
static void button_cb(lv_event_t * e);
static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(1);
    
}

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_group_t*  g;
static lv_obj_t * tv;
static lv_obj_t * t1;
static lv_obj_t * t2;



static void scroll_event_cb(lv_event_t * e)
{
    lv_obj_t * cont = lv_event_get_target(e);

    lv_area_t cont_a;
    lv_obj_get_coords(cont, &cont_a);
    lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;

    lv_coord_t r = lv_obj_get_height(cont) * 7 / 10;
    uint32_t i;
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    for(i = 0; i < child_cnt; i++) {
        lv_obj_t * child = lv_obj_get_child(cont, i);
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);

        lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

        lv_coord_t diff_y = child_y_center - cont_y_center;
        diff_y = LV_ABS(diff_y);

        /*Get the x of diff_y on a circle.*/
        lv_coord_t x;
        /*If diff_y is out of the circle use the last point of the circle (the radius)*/
        if(diff_y >= r) {
            x = r;
        } else {
            /*Use Pythagoras theorem to get x from radius and y*/
            uint32_t x_sqr = r * r - diff_y * diff_y;
            lv_sqrt_res_t res;
            lv_sqrt(x_sqr, &res, 0x8000);   /*Use lvgl's built in sqrt root function*/
            x = r - res.i;
        }

        /*Translate the item by the calculated X coordinate*/
        lv_obj_set_style_translate_x(child, x, 0);
        /*Use some opacity with larger translations*/
        lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
        lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);
    }
}
/*
    时间设置测试
*/
void setTime(int sc, int mn, int hr, int dy, int mt, int yr) {
    // seconds, minute, hour, day, month, year $ microseconds(optional)
    // ie setTime(20, 34, 8, 1, 4, 2021) = 8:34:20 1/4/2021
    struct tm t = {0};        // Initalize to all 0's
    t.tm_year = yr - 1900;    // This is year-1900, so 121 = 2021
    t.tm_mon = mt - 1;
    t.tm_mday = dy;
    t.tm_hour = hr;
    t.tm_min = mn;
    t.tm_sec = sc;
    time_t timeSinceEpoch = mktime(&t);
    printf("timeSinceEpoch = %ld",timeSinceEpoch);
    //   setTime(timeSinceEpoch, ms);
    struct timeval now = { .tv_sec = timeSinceEpoch };
    settimeofday(&now, NULL);
}
void my_timer(lv_timer_t * timer ){

    lv_obj_t * label = timer->user_data ;//当前屏幕
    lv_style_t label_style;
    
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;
    lv_style_init(&label_style);
    //   struct timeval tv_now;
    // Set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
    // initialize_sntp();

   

    // adjtime();
    gettimeofday(&now, NULL);
    //  time(&now);
    localtime_r(&now, &timeinfo);

    //strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    lv_style_set_text_font(&label_style,&lv_font_montserrat_16);
    lv_label_set_text_fmt(label,"%d : %d",timeinfo.tm_hour,timeinfo.tm_min);
    //lv_label_set_text(label,strftime_buf);
    

}
static void menu_page1(void)
{   

    lv_obj_t * scr = lv_obj_create(NULL);

    t1 = scr;
    

    lv_obj_t * btn = lv_btn_create(scr);

    lv_obj_t* label = lv_label_create(scr);

    
    lv_obj_set_align(label,LV_ALIGN_CENTER);

    lv_obj_add_event_cb(btn,button_cb,LV_EVENT_CLICKED,1);

    lv_group_add_obj(g,btn);

    lv_scr_load(scr);

    lv_timer_t * timer = lv_timer_create(my_timer, 1000, label);
    
    setTime(0,0,9,11,1,2022);

}

static void button_cb(lv_event_t * e)
{
    lv_obj_t * cont = lv_event_get_target(e);

    uint8_t id = lv_event_get_user_data(e);
    switch(id)
    {

        case 0 :         
            menu_page1();
        break;

        case 1 :
            lv_scr_load(tv);          
        break;

    }

}

/**
 * Translate the object as they scroll
 */
void lv_example_scroll_6(void)
{
 
     g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(indev_encoder, g);
    //lv_group_set_editing(g, true);
    tv = lv_scr_act();
    lv_obj_t * cont = lv_obj_create(tv);
    
    lv_obj_set_size(cont, 128, 128);
    lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_event_cb(cont, scroll_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(cont, true, 0);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);


    static lv_style_t style_btn,style_btn2;
    lv_style_init(&style_btn);  
    lv_style_set_bg_color(&style_btn,lv_palette_lighten(LV_PALETTE_GREEN, 3));
    lv_obj_t * btn = lv_btn_create(cont);

    lv_obj_add_style(btn, &style_btn,0);

        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text(label, "Configration");
        btn  = lv_btn_create(cont);
        lv_obj_add_event_cb(btn,button_cb,LV_EVENT_CLICKED,0);

        lv_style_init(&style_btn2); 
        
        lv_style_set_bg_color(&style_btn2,lv_palette_lighten(LV_PALETTE_ORANGE, 3));
        lv_obj_add_style(btn, &style_btn2,0);
        label = lv_label_create(btn);
        lv_label_set_text(label, "Exit");

    
    /*Update the buttons position manually for first*/
    lv_event_send(cont, LV_EVENT_SCROLL, NULL);

    /*Be sure the fist button is in the middle*/
    lv_obj_scroll_to_view(lv_obj_get_child(cont, 0), LV_ANIM_OFF);
    lv_scr_load(cont);
}

void lv_example_img_1(void)
{
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 172, 172);
    
}

static void guiTask(void *pvParameter) {
    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
/* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1 * 1000));
    lv_example_scroll_6();
    
    while(1){  
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));      
        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

}
// //更新时间回调函数
// void time_sync_notification_cb(struct timeval *tv)
// {
//     ESP_LOGI(TAG, "Notification of a time synchronization event");
// }

// //初始化
// void initialize_sntp(void)
// {
//     ESP_LOGI(TAG, "Initializing SNTP");
//     sntp_setoperatingmode(SNTP_OPMODE_POLL);
//     sntp_setservername(0, "ntp1.aliyun.com");
// 	sntp_setservername(1, "210.72.145.44");		// 国家授时中心服务器 IP 地址
//     sntp_setservername(2, "1.cn.pool.ntp.org"); 
//     sntp_set_time_sync_notification_cb(time_sync_notification_cb);
// #ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
//     sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
// #endif
//     sntp_init();
// }

static void RTC_timeTask(void *pvParameter)
{
    (void)pvParameter;
    const char* TAG =  "RTCtimer";
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;
    //   struct timeval tv_now;
    // Set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
    // initialize_sntp();
    setTime(0,0,9,11,1,2022);    
    // adjtime();
    
    while(1)
    {
        //settimeofday();

        gettimeofday(&now, NULL);

        //  time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void test(void)
{
    gpio_reset_pin( GPIO_DC );
	gpio_set_direction( GPIO_DC, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_DC, 0 );

    gpio_reset_pin( GPIO_CS );
	gpio_set_direction( GPIO_CS, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_CS, 0 );

    gpio_reset_pin( GPIO_RESET );
	gpio_set_direction( GPIO_RESET, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_RESET, 0 );
    while(1)
    {

        // gpio_set_level( GPIO_RESET, 0 );
        vTaskDelay(pdMS_TO_TICKS(10));   
        // gpio_set_level( GPIO_RESET, 1 );
        // vTaskDelay(10);

    }



}
void app_main(void)
{

 //   xTaskCreatePinnedToCore(RTC_timeTask, "RTC_timeTask", 1024*4, NULL, 1, NULL, PRO_CPU_NUM);
   // xTaskCreatePinnedToCore(guiTask, "gui", 4096*4, NULL, 2, NULL, APP_CPU_NUM);
   // xTaskCreatePinnedToCore(test,"test",4096*4, NULL, 2, NULL, APP_CPU_NUM);
    gpio_reset_pin( GPIO_DC );
	gpio_set_direction( GPIO_DC, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_DC, 0 );

    gpio_reset_pin( GPIO_CS );
	gpio_set_direction( GPIO_CS, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_CS, 0 );

    gpio_reset_pin( GPIO_RESET );
	gpio_set_direction( GPIO_RESET, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_RESET, 0 );
    while(1)
    {

        // gpio_set_level( GPIO_RESET, 0 );
        //vTaskDelay(pdMS_TO_TICKS(10));   
        // gpio_set_level( GPIO_RESET, 1 );
        // vTaskDelay(10);

    }
}