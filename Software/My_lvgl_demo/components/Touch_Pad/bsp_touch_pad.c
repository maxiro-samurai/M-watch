#include "bsp_touch_pad.h"
#include "driver/touch_pad.h"
#include "soc/rtc_periph.h"
#include "soc/sens_periph.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static const char *TAG = "Touch pad";
extern QueueHandle_t xtouchpadque;
static void tp_example_rtc_intr(void *arg)
{   

    uint32_t pad_intr = touch_pad_get_status();
    //taskENTER_CRITICAL_FROM_ISR();
    //clear interrupt
    touch_pad_clear_status();    
    //xSemaphoreGiveFromISR(xtouchpadsem,NULL);
    //taskEXIT_CRITICAL_FROM_ISR();
    //printf("Touch pad");
      xQueueSendFromISR(xtouchpadque,&pad_intr,NULL);
}


void Touch_Pad_Init(void)
{
    uint16_t touch_value;

    touch_pad_init();//初始化触摸传感器库
    // If use interrupt trigger mode, should set touch sensor FSM mode at 'TOUCH_FSM_MODE_TIMER'.
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

    touch_pad_config(TOUCH_PAD_NUM7,TOUCH_THRESH_NO_USE);//初始化引脚 27为触摸引脚
    touch_pad_config(TOUCH_PAD_NUM5,TOUCH_THRESH_NO_USE);
    touch_pad_config(TOUCH_PAD_NUM8,TOUCH_THRESH_NO_USE);
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    //read filtered value
    touch_pad_read_filtered(TOUCH_PAD_NUM7, &touch_value);
    ESP_LOGI(TAG, "test init: touch pad [5] val is %d", touch_value);
    touch_pad_read_filtered(TOUCH_PAD_NUM5, &touch_value);
    ESP_LOGI(TAG, "test init: touch pad [7] val is %d", touch_value);
    touch_pad_read_filtered(TOUCH_PAD_NUM8, &touch_value);
    ESP_LOGI(TAG, "test init: touch pad [8] val is %d", touch_value);
    //set interrupt threshold.
    touch_pad_set_thresh(TOUCH_PAD_NUM7, touch_value * 1 / 3);
    touch_pad_set_thresh(TOUCH_PAD_NUM5, touch_value * 1 / 3);
    touch_pad_set_thresh(TOUCH_PAD_NUM8, touch_value * 1 / 3);
    touch_pad_isr_register(tp_example_rtc_intr, NULL);
    touch_pad_intr_enable();

}