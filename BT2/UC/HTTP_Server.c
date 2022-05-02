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
//#include "time.h"
#include "rtc.h"
//#include "Board_ADC.h"

#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"
#include "potenciometro.h"
#include "lcd.h"
#include "time.h"
#include "rl_net_lib.h"
#include "lpc17xx_iap.h"

/*-------------------------------------------------------------------------------------------------------------*/
#include "rebotes_joystick.h"
#define signal_pwd_pulse						0x04		// 100
#define signal_i2c  								0x100		

osEvent	i2c_event;
/*-------------------------------------------------------------------------------------------------------------*/

#define PUERTO_LED 			1
#define LED_1 					18
#define LED_2 					20
#define LED_3 					21
#define LED_4 					23

#define EV_GANANCIA_1   			0x00
#define EV_GANANCIA_5   			0x01
#define EV_GANANCIA_10  			0x02
#define EV_GANANCIA_50  			0x03
#define EV_GANANCIA_100 			0x04
#define EV_CHANGE_OVERLOAD		0X05
#define EV_INT_OVERLOAD_ON		0x06
#define EV_INT_OVERLOAD_OFF		0x07

// Led RGB
#define port_led_RGB	2
#define led_GREEN			2


extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

extern ETH_CFG eth0_config;
extern  LOCALM localm[];
#define LocM   localm[NETIF_ETH]


bool LEDrun;
bool LCDupdate;
bool rtc_update;
char lcd_text[2][20+1];
char buffer_LCD[512];
char hora [64];
char fecha [64];
uint8_t mac_ip[10];
uint32_t j;
uint8_t situacion_leds;
uint8_t umbral[1] = {0xA0};
static void BlinkLed (void const *arg);
static void RTC (void const *arg);
static void I2C (void const *arg);
uint8_t num_eventos[1] ={1};

// Estados.
uint8_t estado_agp;	

osThreadDef(BlinkLed, osPriorityNormal, 1, 0);
osThreadDef(RTC, osPriorityNormal, 1, 0);

osThreadId tid_I2C; 
osThreadDef(I2C, osPriorityNormal, 1, 0);


/*----------------------------------------------------------------------------------------------------------------*/
osThreadId tid_RTC; 
/*----------------------------------------------------------------------------------------------------------------*/

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
	uint8_t valor;
	uint8_t limite;

  if (ch == 0) {
    ADC_StartConversion();
    while (ADC_ConversionDone () < 0);
    val = ADC_GetValue();
		valor = val/16;
		limite = leer_posicion(11);
		if (valor > limite) GPIO_PinWrite(port_led_RGB,led_GREEN,0);
		else GPIO_PinWrite(port_led_RGB,led_GREEN,1);
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
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/
static void BlinkLed (void const *arg) {
  const uint32_t led_val[13] = {0x040000,0x140000,0x340000,0xB40000,0xB00000,0xA00000,
															0x800000,0xA00000,0xB00000,0xB40000,0x340000,0x140000,0x040000};
	int cnt = 0;
															
	situacion_leds = leer_posicion(11);															
  
	if((situacion_leds&0x10) == 0x10) LEDrun = true;
  else{
		LEDrun = false;
		if((situacion_leds&0x01) == 0x01) GPIO_PinWrite(PUERTO_LED,LED_1,1);
		if((situacion_leds&0x02) == 0x02) GPIO_PinWrite(PUERTO_LED,LED_2,1);
		if((situacion_leds&0x04) == 0x04) GPIO_PinWrite(PUERTO_LED,LED_3,1);
		if((situacion_leds&0x08) == 0x08) GPIO_PinWrite(PUERTO_LED,LED_4,1);
	}
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
  Thread 'I2C': Hilo para controlar el envío de la UC a través del I2C
 *---------------------------------------------------------------------------*/
static void I2C (void const *arg){

	while (1) {
    i2c_event = osSignalWait (signal_i2c, osWaitForever);
		if (i2c_event.status == osEventSignal){
			switch (estado_agp){
			
				case EV_GANANCIA_1:
					
				break;
				
				case EV_GANANCIA_5:
					
				break;
				
				case EV_GANANCIA_10:
					
				break;
				
				case EV_GANANCIA_50:
						
				break;
				
				case EV_GANANCIA_100:
					
				break;
				
				case EV_CHANGE_OVERLOAD:
					
				break;
				
				/*case EV_INT_OVERLOAD_ON:
					
				break;
				
				case EV_INT_OVERLOAD_OFF:
					
				break;*/

			}
		}
    osThreadYield ();                                           // suspend thread
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
	Init_lcd();
  osThreadCreate (osThread(BlinkLed), NULL);
	tid_I2C = osThreadCreate (osThread(I2C), NULL);
	/*----------------------------------------------------------------------------------------------------------------*/
	tid_RTC = osThreadCreate (osThread(RTC), NULL);
	// Lanzamos el hilo que se encarga de controlar los rebotes
	Init_rebotes_joystick();
	/*----------------------------------------------------------------------------------------------------------------*/
	rtc_update = true;
	osDelay(1000);
	get_time();
	
	/*for(j=0; j<6; j++){
		mac_ip[j] = eth0_config.MacAddr[j];
	}
	for(j=6; j<10; j++){
		mac_ip[j] = LocM.IpAddr[j-6];
	}
	
	escribir_posicion(0,10,mac_ip);
	escribir_posicion(12,1,umbral);*/
	
  while(1) {
    net_main ();
    osThreadYield ();
  }
}
