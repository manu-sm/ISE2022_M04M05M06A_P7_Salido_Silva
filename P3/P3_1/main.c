#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"

#define PUERTO_LED 				1
#define LED_1 						18
#define LED_2 						20
#define LED_3 						21
#define LED_4 						23

#define RIT_ENABLE				3 // Bit 3 del registro RIT_CTRL: Permite habilitar/deshabilitar el RIT.
#define ENABLE_POWER_RIT	16// Bit 16 del registro PCONP: Habilita/deshabilita alimentacion del RIT

bool flag_RIT = false;



void WDT_IRQHandler(void)
{
	// Disable WDT interrupt
	NVIC_DisableIRQ(WDT_IRQn);
	LPC_WDT->WDMOD &= ~(0x4); //Borra el flag de interrupción
	GPIO_PinWrite(PUERTO_LED,LED_3,!GPIO_PinRead(PUERTO_LED,LED_3));
	NVIC_EnableIRQ(WDT_IRQn);
}


void config_WDT(){
	LPC_SC->PCLKSEL0 &= ~(0x03); // PCLK_WDT --> PCLK/4
	LPC_WDT->WDCLKSEL = 0x01; // Seleccionamos WatchDog PCLK como reloj.
	LPC_WDT->WDTC = 0x1DCD650; // Equivale a 5 segundos
	LPC_WDT->WDMOD |= 1 << 0 ; // Habilita WDT, falta hacer feed del WDT
	LPC_WDT->WDFEED = 0xAA;
	LPC_WDT->WDFEED = 0x55;
	NVIC_EnableIRQ(WDT_IRQn);
}


void InitLED(){
	GPIO_SetDir (PUERTO_LED, LED_1, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED1
	GPIO_SetDir (PUERTO_LED, LED_2, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED2
	GPIO_SetDir (PUERTO_LED, LED_3, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED3
	GPIO_SetDir (PUERTO_LED, LED_4, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED4	
}

void config_RIT(){
	LPC_SC->PCONP |= 1 << ENABLE_POWER_RIT;
	LPC_RIT->RICOMPVAL = 0x00BEBC20; //Equivale a 500 ms
	LPC_RIT->RIMASK = 0 ;
	LPC_RIT->RICTRL = 0x04;
	LPC_RIT->RICOUNTER = 0x00;
	
	LPC_RIT->RICTRL |= 1 << 1;
	
	NVIC_EnableIRQ(RIT_IRQn);
}

void RIT_IRQHandler(void){
	LPC_RIT->RICTRL |= 1 << 1;
	flag_RIT = true;
}


int main(){
	bool inicial = true;
	int estado = 0;
	int cnt = 0;
	
	
	InitLED();
	config_RIT();
	GPIO_PinWrite(PUERTO_LED,LED_1,1);
	LPC_RIT->RICTRL |= 1 << RIT_ENABLE;
	
	while(1){
		
		if(flag_RIT){
			flag_RIT = false;
			if(inicial){
				if(cnt++ == 7){
					inicial = false;
					GPIO_PinWrite(PUERTO_LED,LED_1,estado);
					GPIO_PinWrite(PUERTO_LED,LED_2,~(estado));
				} 
					
			}
			else{
				estado = ~(estado);
				GPIO_PinWrite(PUERTO_LED,LED_1,estado);
				GPIO_PinWrite(PUERTO_LED,LED_2,~(estado));
			}
		}
	}
	
}


