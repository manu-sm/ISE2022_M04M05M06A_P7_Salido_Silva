#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"

#define PUERTO_LED 				1
#define LED_1 						18
#define LED_2 						20
#define LED_3 						21
#define LED_4 						23

#define PUERTO_RGB				2
#define LED_GREEN					2
#define LED_RED						3
#define LED_RGB_OFF				1
#define LED_RGB_ON 				0

#define PUERTO_JOYSTICK		0
#define JOYSTICK_DOWN			17

#define MASK_RSID_CLEAR		0xF
#define MASK_RSID_WDT		  0x4


#define RIT_ENABLE				3 // Bit 3 del registro RIT_CTRL: Permite habilitar/deshabilitar el RIT.
#define ENABLE_POWER_RIT	16// Bit 16 del registro PCONP: Habilita/deshabilita alimentacion del RIT



bool flag_RIT = false;
bool estado = false;

void EINT3_IRQHandler (void) {
	static uint32_t cuenta;
  if (cuenta++ < 10) {
		estado = !estado;
		GPIO_PinWrite(PUERTO_LED,LED_4, estado);   
   }
	LPC_GPIOINT->IO0IntClr |= 1<< JOYSTICK_DOWN;
 }

 void init_joystick(){
	 PIN_Configure (PUERTO_JOYSTICK, JOYSTICK_DOWN, PIN_FUNC_0, PIN_PINMODE_PULLDOWN, PIN_PINMODE_NORMAL);
	 GPIO_SetDir   (PUERTO_JOYSTICK, JOYSTICK_DOWN, GPIO_DIR_INPUT);
	 LPC_GPIOINT->IO0IntEnR |=  (1 << JOYSTICK_DOWN) ;
	 NVIC_EnableIRQ(EINT3_IRQn);
 
 }

void WDT_IRQHandler(void){
	// Disable WDT interrupt
	NVIC_DisableIRQ(WDT_IRQn);
	LPC_WDT->WDMOD &= ~(0x4); //Borra el flag de interrupción
	GPIO_PinWrite(PUERTO_LED,LED_3,!GPIO_PinRead(PUERTO_LED,LED_3));
	//NVIC_EnableIRQ(WDT_IRQn);
}

void feed_WDT(){
	LPC_WDT->WDFEED = 0xAA;
	LPC_WDT->WDFEED = 0x55;
}
void config_WDT(){
	NVIC_EnableIRQ(WDT_IRQn);
	LPC_SC->PCLKSEL0 &= ~(0x03); // PCLK_WDT --> PCLK/4
	LPC_WDT->WDCLKSEL = 0x01; // Seleccionamos WatchDog PCLK como reloj.
	LPC_WDT->WDTC = 0x1DCD650; // Equivale a 5 segundos
	LPC_WDT->WDMOD |= 1 << 1;  // Si el WatchDog llega a su timeout --> RESET
	LPC_WDT->WDMOD |= 1 << 0 ; // Habilita WDT, falta hacer feed del WDT
	feed_WDT();
}


void InitLED(){
	GPIO_SetDir (PUERTO_LED, LED_1, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED1
	GPIO_SetDir (PUERTO_LED, LED_2, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED2
	GPIO_SetDir (PUERTO_LED, LED_3, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED3
	GPIO_SetDir (PUERTO_LED, LED_4, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto LED4	
	GPIO_SetDir (PUERTO_RGB, LED_GREEN, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto Led Verde
	GPIO_SetDir (PUERTO_RGB, LED_RED, GPIO_DIR_OUTPUT); 	// Establece dirección salida puerto Led Rojo
	GPIO_PinWrite(PUERTO_RGB,LED_RED, LED_RGB_OFF);
	GPIO_PinWrite(PUERTO_RGB,LED_GREEN, LED_RGB_OFF);
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
	
	if((LPC_SC->RSID & MASK_RSID_WDT) == MASK_RSID_WDT){
		GPIO_PinWrite(PUERTO_RGB,LED_RED,LED_RGB_ON);
		LPC_WDT->WDMOD &= ~(0x4);						// Borramos flag de interrupcion del WDT
	}
	else{
		GPIO_PinWrite(PUERTO_RGB,LED_GREEN,LED_RGB_ON);
	}
	LPC_SC->RSID |= MASK_RSID_CLEAR;
	
	

	init_joystick();
	config_RIT();
	config_WDT();
	GPIO_PinWrite(PUERTO_LED,LED_1,1);
	LPC_RIT->RICTRL |= 1 << RIT_ENABLE;
	
	while(1){
		
		if(flag_RIT){
			flag_RIT = false;
			if(inicial){
				if(cnt == 6){
					GPIO_PinWrite(PUERTO_RGB,LED_RED,LED_RGB_OFF);
					GPIO_PinWrite(PUERTO_RGB,LED_GREEN,LED_RGB_OFF);
				}
				if(cnt++ == 7){
					inicial = false;
					GPIO_PinWrite(PUERTO_LED,LED_1,estado);
					GPIO_PinWrite(PUERTO_LED,LED_2,~(estado));
				} 
					
			}
			else{
				feed_WDT();

				estado = ~(estado);
				GPIO_PinWrite(PUERTO_LED,LED_1,estado);
				GPIO_PinWrite(PUERTO_LED,LED_2,~(estado));
			}
		}
	}
	
}


