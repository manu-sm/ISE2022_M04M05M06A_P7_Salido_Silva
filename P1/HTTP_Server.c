/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include "cmsis_os.h"                   /* CMSIS RTOS definitions             */
#include "rl_net.h"                     /* Network definitions                */

#include "Board_GLCD.h"
#include "GLCD_Config.h"
#include "Board_LED.h"
#include "Board_Buttons.h"
//#include "Board_ADC.h"

#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"
#include "potenciometro.h"
#include "lcd.h"
#include "time.h"
#include "rtc.h"
#include "rebotes_joystick.h"

#define PUERTO_LED 			1
#define LED_1 					18
#define LED_2 					20
#define LED_3 					21
#define LED_4 					23

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

bool LEDrun;
bool LCDupdate;
char lcd_text[2][20+1];
char buffer_LCD[512];
char hora [64];
char fecha [64];


static void BlinkLed (void const *arg);
static void Display (void const *arg);
static void RTC (void const *arg);

osThreadId tid_RTC;
osThreadDef(BlinkLed, osPriorityNormal, 1, 0);
osThreadDef(Display, osPriorityNormal, 1, 0);
osThreadDef(RTC, osPriorityNormal, 1, 0);

uint64_t segundos = 0;


void InitLED(){
	GPIO_SetDir (PUERTO_LED, LED_1, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED1
	GPIO_SetDir (PUERTO_LED, LED_2, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED2
	GPIO_SetDir (PUERTO_LED, LED_3, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED3
	GPIO_SetDir (PUERTO_LED, LED_4, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED4	
}

/// Read analog inputs
uint16_t AD_in (uint32_t ch) {
  int32_t val = 0;

  if (ch == 0) {
    ADC_StartConversion();
    while (ADC_ConversionDone () < 0);
    val = ADC_GetValue();
  }
  return (val);
}

/// Read digital inputs
uint8_t get_button (void) {
  return (Buttons_GetState ());
}

/// IP address change notification
void dhcp_client_notify (uint32_t if_num,
                         dhcpClientOption opt, const uint8_t *val, uint32_t len) {
  if (opt == dhcpClientIPaddress) {
    // IP address has changed
    sprintf (lcd_text[0],"IP address:");
    sprintf (lcd_text[1],"%s", ip4_ntoa (val));
    LCDupdate = true;
  }
}

/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
static void Display (void const *arg) {
  //char lcd_buf[20+1];

  /*GLCD_Initialize         ();
  GLCD_SetBackgroundColor (GLCD_COLOR_BLUE);
  GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
  GLCD_ClearScreen        ();
  GLCD_SetFont            (&GLCD_Font_16x24);
  GLCD_DrawString         (0, 1*24, "       MDK-MW       ");
  GLCD_DrawString         (0, 2*24, "HTTP Server example ");
*/
	
  sprintf (lcd_text[0], "");
  sprintf (lcd_text[1], "Waiting for DHCP");
  LCDupdate = true;

  while(1) {
    if (LCDupdate == true) {
      /*sprintf (lcd_buf, "%-20s", lcd_text[0]);
      GLCD_DrawString (0, 5*24, lcd_buf);
      sprintf (lcd_buf, "%-20s", lcd_text[1]);
      GLCD_DrawString (0, 6*24, lcd_buf);*/
			EscribeLinea_1(lcd_text[0]);
			EscribeLinea_2(lcd_text[1]);
      LCDupdate = false;
    }
    osDelay (250);
  }
}

/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
static void BlinkLed (void const *arg) {
  const uint32_t led_val[13] = {0x040000,0x140000,0x340000,0xB40000,0xB00000,0xA00000,
															0x800000,0xA00000,0xB00000,0xB40000,0x340000,0x140000,0x040000};
	//const uint32_t led_val[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
	
  int cnt = 0;

  LEDrun = true;
  while(1) {
    // Every 100 ms
    if (LEDrun == true) {
      GPIO_PortWrite(PUERTO_LED,0x00B40000,led_val[cnt]);
      if (++cnt >= 13) {
        cnt = 0;
      }
    }
    osDelay (100);
  }
}

/*----------------------------------------------------------------------------
  Thread 'RTC': 
 *---------------------------------------------------------------------------*/
static void RTC (void const *arg) {
  rtc_control();
}

/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
int main (void) {
 // LED_Initialize     ();
 // Buttons_Initialize ();
 // ADC_Initialize     ();
	Init_Pot1();
	InitLED();
  net_initialize     ();
	LCD_init();
	LCD_reset();
	get_time();
	
  osThreadCreate (osThread(BlinkLed), NULL);
	tid_RTC = osThreadCreate (osThread(RTC), NULL);
  //osThreadCreate (osThread(Display), NULL);
	Init_rebotes_joystick();
	

  while(1) {
    net_main ();
		
		//get_time();
    osThreadYield ();
  }
}
