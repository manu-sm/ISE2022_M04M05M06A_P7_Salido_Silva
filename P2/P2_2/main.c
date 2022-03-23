#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "SSP_LPC17xx.h"
#include <string.h>
#include "RTC.h"

#include "lcd.h"

#define PUERTO_LEDS 1
#define PIN_LED4		23


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Variables globales
char hora [64];			// Variable global que almacena en cada momento la hora del sistema.
char fecha [64];		// Variable global que almacena en cada momento la fecha del sistema.

bool flag_min = false;
bool flag_RIT = false;
uint8_t toggle_led = false;
	


void RIT_IRQHandler(void){
	LPC_RIT->RICTRL |= 1 << 1;
	flag_RIT = true;
}

int main() {
	
	uint8_t cnt = 0;
	
	GPIO_SetDir(PUERTO_LEDS,PIN_LED4,GPIO_DIR_OUTPUT);
	
	LPC_SC->PCONP |= 1 << 16;
	
	LPC_RIT->RICOMPVAL = 0x005F5E10; //Equivale a 250 ms
	LPC_RIT->RIMASK = 0 ;
	LPC_RIT->RICTRL = 0x04;
	LPC_RIT->RICOUNTER = 0x00;
	
	LPC_RIT->RICTRL |= 1 << 1;
	
	NVIC_EnableIRQ(RIT_IRQn);
	
	LCD_init();
	LCD_reset();
	init_RTC();
	enable_RTC();
	set_hour(20,37,52);
	set_date(17,3,2022);
	set_hour_intrrupt (2);
	LPC_RTC->CIIR = 0x02; // Habilita interrupción por incremento en contador minutos
	
	NVIC_EnableIRQ(RTC_IRQn);
	
		
	while(1){
		
		get_hora();
		get_fecha();
		EscribeLinea_1(hora);
		EscribeLinea_2(fecha);
		LCD_update();
		
		if(flag_min){
			flag_min = false;
			LPC_RIT->RICTRL |= 1 << 3;
		}
		
		if(flag_RIT){
			flag_RIT = false;
			toggle_led = ~(toggle_led);
			GPIO_PinWrite(PUERTO_LEDS,PIN_LED4,toggle_led);
			if(cnt++>=20){
				cnt = 0;
				LPC_RIT->RICTRL &= 0xFFFFFFF7;
				GPIO_PinWrite(PUERTO_LEDS,PIN_LED4,0);
			}
		}
	}
}

