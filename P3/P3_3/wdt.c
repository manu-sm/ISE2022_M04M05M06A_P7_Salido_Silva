#include "wdt.h"


/*void WDT_IRQHandler(void){
	// Disable WDT interrupt
	NVIC_DisableIRQ(WDT_IRQn);
	LPC_WDT->WDMOD &= ~(0x4); //Borra el flag de interrupción
	GPIO_PinWrite(PUERTO_LED,LED_3,!GPIO_PinRead(PUERTO_LED,LED_3));
	//NVIC_EnableIRQ(WDT_IRQn);
}*/

void feed_WDT(){
	LPC_WDT->WDFEED = 0xAA;
	LPC_WDT->WDFEED = 0x55;
}
void config_WDT(){
	//NVIC_EnableIRQ(WDT_IRQn);
	LPC_SC->PCLKSEL0 &= ~(0x03); // PCLK_WDT --> PCLK/4
	LPC_WDT->WDCLKSEL = 0x01; // Seleccionamos WatchDog PCLK como reloj.
	LPC_WDT->WDTC = 0x1DCD650; // Equivale a 5 segundos  (Cada tick 160ns--> T = 4/25MHz)
	LPC_WDT->WDMOD |= 1 << 1;  // Si el WatchDog llega a su timeout --> RESET
	LPC_WDT->WDMOD |= 1 << 0 ; // Habilita WDT, falta hacer feed del WDT para que empiece a contar.
	feed_WDT();
}

