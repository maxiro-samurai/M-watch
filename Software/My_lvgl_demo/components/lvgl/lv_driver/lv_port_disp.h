/**
 * @file lv_port_disp_templ.h
 *
 */

 /*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "st7735s.h"
/*********************
 *      DEFINES
 *********************/
#define SCREEN_WIDTH	172
#define SCREEN_HEIGHT	320
#define	OFFSET_X		0
#define OFFSET_Y		0
#define GPIO_MOSI   	13
#define GPIO_SCLK   	14
#define GPIO_CS     	15
#define GPIO_DC     	2
#define GPIO_BL         -1
#define GPIO_RESET  	4
#define MY_DISP_HOR_RES SCREEN_WIDTH
#define	INTERVAL		100
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_disp_init(void);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/

#endif /*Disable/Enable content*/
