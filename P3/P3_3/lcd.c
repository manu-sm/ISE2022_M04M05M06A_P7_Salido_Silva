#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "lcd.h"
#include "Driver_SPI.h"
#include "GPIO_LPC17xx.h"
#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "Arial12x12.h"
#include <string.h>
#include "wdt.h"

#define gpioPORT_SSP1		0
#define	pin_nRSET_SSP1	8
#define	pin_A0_SSP1			6
#define CS							18

#define PUERTO_LED		1
#define	LED_1					18

 // Variables globales
extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

extern char lcd_text [2][21];
extern char buffer_LCD [512];
extern bool LCDupdate;
uint16_t array_lcd = 0;
extern char hora [64];
extern char fecha [64];

void lcd (void const *argument);                             // thread function
osThreadId tid_lcd;                                          // thread id
osThreadDef (lcd, osPriorityNormal, 1, 0);                   // thread object

osEvent printf_1_line_event;																			 // Señal que manda escribir en la primera línea
osEvent printf_2_line_event;																			 // Señal que manda escribir en la segunda línea

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          Init_lcd (void)
  \brief       Función que se encarga de lanzar el hilo del lcd para inicializar su funcionamiento
  \returns
   - \b  0: función correcta
   - \b -1: error
*/
int Init_lcd (void) {

  tid_lcd = osThreadCreate (osThread(lcd), NULL);
  if (!tid_lcd) return(-1);
  
  return(0);
}



//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          init (void)
  \brief       Función que inicializa el driver SPI y configura el lcd para poder usarlo.
*/
void init (void){

		int32_t status;
		status = SPIdrv->Initialize(NULL);													// Esta función siempre va la primera.
		status = SPIdrv->PowerControl(ARM_POWER_FULL);	
		status = SPIdrv->Control (ARM_SPI_MODE_MASTER |							// LPC ==> Master.
															ARM_SPI_CPOL1_CPHA1 |							// Configuración de la temporización.
															ARM_SPI_MSB_LSB 		|							// Set the bit order from MSB to LSB.
															ARM_SPI_DATA_BITS(8), 20000000);	// Tamaño de palabra de datos de 8 bits.
	
	// Interrupcion de timer0 para controlar los tiempos del reset
	LPC_SC->PCONP |= 1 << 1;      	// Alimentamos el timer0 (aunque por defecto tras el reset se alimenta automáticamente
	LPC_TIM0->MR0 = 0x00000019;     // Equivale a 1 us para controlar los rebotes
	LPC_TIM0->MCR |= 1 << 0;      	// Interrupt on Match0 compare
	
	// Configuramos el Reset, el CS y el puero A0. Todos gestionados por el GPIO.
	GPIO_SetDir (gpioPORT_SSP1, pin_nRSET_SSP1, GPIO_DIR_OUTPUT);
	GPIO_SetDir (gpioPORT_SSP1, pin_A0_SSP1, GPIO_DIR_OUTPUT);
	GPIO_SetDir (gpioPORT_SSP1, CS, GPIO_DIR_OUTPUT);
	
	GPIO_PinWrite (gpioPORT_SSP1, pin_A0_SSP1, 0);
	GPIO_PinWrite (gpioPORT_SSP1, pin_nRSET_SSP1, 1);
	GPIO_PinWrite (gpioPORT_SSP1, CS, 1);
	GPIO_PinWrite (gpioPORT_SSP1, pin_nRSET_SSP1, 0);
	
	GPIO_SetDir (PUERTO_LED, LED_1, GPIO_DIR_OUTPUT);
	
	// Generamos el Reset
	retardo_1us();
	
	GPIO_PinWrite (gpioPORT_SSP1, pin_nRSET_SSP1, 1);
	
	retardo_1ms();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          retardo_1us (void)
  \brief       Función que realiza un retardo de 1 micro segundo para el reseteo del lcd
*/
void retardo_1us (void){
	
	static bool tic_ena = true;
	
	LPC_TIM0->TCR |= 1 << 1;      	// Reset Timer0			
	LPC_TIM0->TCR &= ~(1 << 1);   	// stop resetting the timer.
	LPC_TIM0->TCR |= 1 << 0;        // Start timer. 
	
	while (tic_ena) {
			if((LPC_TIM0->IR & 0x01) == 0x01){
				LPC_TIM0->IR |= 1 << 0; 	// Clear MR0 interrupt flag}
				GPIO_PinWrite (PUERTO_LED, LED_1, 1);
				tic_ena = false;
			}
	}	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          retardo_1ms (void)
  \brief       Función que realiza un retardo de 1 mili segundo para el reseteo del lcd
*/
void retardo_1ms (void){
	
	static bool tic_ena = true;
	LPC_TIM0->TCR |= 1 << 1;      	// Reset Timer0			
	LPC_TIM0->TCR &= ~(1 << 1);   	// stop resetting the timer.
	LPC_TIM0->TCR |= 1 << 0;        // Start timer. 
	
	while (tic_ena) {
			if((LPC_TIM0->IR & 0x01) == 0x01){
				LPC_TIM0->IR |= 1 << 0; 	// Clear MR0 interrupt flag}
				tic_ena = false;
				GPIO_PinWrite (PUERTO_LED, LED_1, 0);
			}
	}	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          wr_cmd (uint8_t cmd)
  \brief       Función que se encarga de enviar los comandos de configuración al LCD. ==> A0 = 0.
	\param[in]   cmd      Comando de configuración.
*/
void wr_cmd (uint8_t cmd){
	
	GPIO_PinWrite (gpioPORT_SSP1, CS, 0);
	GPIO_PinWrite (gpioPORT_SSP1, pin_A0_SSP1, 0);
	SPIdrv->Send(&cmd,sizeof(cmd));
	GPIO_PinWrite (gpioPORT_SSP1, CS, 1);

}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          wr_data (uint8_t dat)
  \brief       Función que se encarga de escribir en el lcd. ==> A0 = 1.
	\param[in]   dat      Dato a escribir
*/
void wr_data (uint8_t dat){
	
	GPIO_PinWrite (gpioPORT_SSP1, CS, 0);
	GPIO_PinWrite (gpioPORT_SSP1, pin_A0_SSP1, 1);
	SPIdrv->Send(&dat,sizeof(dat));
	GPIO_PinWrite (gpioPORT_SSP1, CS, 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          LCD_reset (void)
  \brief       Función que se encarga de realizar el reset necesario para que el LCD funcione.
*/
void LCD_reset (void){
	
	wr_cmd(0xAE); 	// Display off
	wr_cmd(0xA2); 	// Fija el valor de la relación de la tensión de polarización del LCD a 1/9 
	wr_cmd(0xA0); 	// El direccionamiento de la RAM de datos del display es la normal
	wr_cmd(0xC8); 	// El scan en las salidas COM es el normal
	wr_cmd(0x22); 	// Fija la relación de resistencias interna a 2
	wr_cmd(0x2F); 	// Power on
	wr_cmd(0x40); 	// Display empieza en la línea 0
	wr_cmd(0xAF);   // Display ON
	wr_cmd(0x81);   // Contraste
	wr_cmd(0x17);   // Valor Contraste
	wr_cmd(0xA4);   // Display all points normal
	wr_cmd(0xA6);   // LCD Display normal 
	
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          copy_to_lcd(void)
  \brief       Función que se encarga de pintar en el lcd una secuencia de texto almacenada en el array @param buffer_LCD[]
*/

void copy_to_lcd(void){
		
	int i;
    wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0
    wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0
    wr_cmd(0xB0);      // Página 0
    
    for(i=0;i<128;i++){
        wr_data(buffer_LCD[i]);
        }
  
    wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0
    wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0
    wr_cmd(0xB1);     
				
		// Página 1
    for(i=128;i<256;i++){
        wr_data(buffer_LCD[i]);
			  }
    
    wr_cmd(0x00);       
    wr_cmd(0x10);      
    wr_cmd(0xB2);      
		
		//Página 2
    for(i=256;i<384;i++){
        wr_data(buffer_LCD[i]);
        }
    
    wr_cmd(0x00);       
    wr_cmd(0x10);       
    wr_cmd(0xB3);      
				
		// Pagina 3
    for(i=384;i<512;i++){
        wr_data(buffer_LCD[i]);
        }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          print_lcd (char texto[])
  \brief       Función que recibe un array de carácteres, los envía uno a uno a las funciones de escritura y controla el desbordamiento de memoria del buffer
  \param[in]   char texto[]      Array que contiene los datos que se van a mostrar por pantalla
*/
int print_lcd (char texto[]){
	
	uint8_t i, j, valor1, valor2;
	uint16_t comienzo = 0;
	uint16_t posicionL1 = 0;
	uint8_t letra;
	uint8_t long_string = strlen(texto);
	
	for (j = 0; j<long_string; j++){
	
		letra = texto[j];
		comienzo = 25 * (letra - ' ');
		
		for (i = 0; i < 12; i++){
			
			valor1 = Arial12x12[comienzo+i*2+1];
			valor2 = Arial12x12[comienzo+i*2+2];
			
			if (i+posicionL1 < 128){
				if (i+posicionL1+Arial12x12[comienzo]>127){
						posicionL1 = 121;
				}
				else {
					buffer_LCD [i+posicionL1] = valor1;
					buffer_LCD [i+128+posicionL1] = valor2;
				}
					
			}
			else if ((i+posicionL1 >= 128) && (i+posicionL1+Arial12x12[comienzo] < 256)){
					buffer_LCD [i+128+posicionL1] = valor1;
					buffer_LCD [i+256+posicionL1] = valor2;
			}
			else return -1; // Hemos sobrepasado la memoria del display
	}
		posicionL1 = posicionL1 + Arial12x12[comienzo];
	}

	return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          print_line_1 (char texto[])
  \brief       Función que recibe un array de carácteres, los envía uno a uno a las funciones de escritura y controla el desbordamiento de memoria del buffer. 
							 Escribe únicamente en la primera línea del lcd
  \param[in]   char texto[]      Array que contiene los datos que se van a mostrar por pantalla
*/
int print_line_1 (char texto[]){
	
	uint8_t i, j, valor1, valor2;
	uint16_t comienzo = 0;
	uint16_t posicionL1 = 0;
	uint8_t letra;
	uint8_t long_string = strlen(texto);
	
	for (j = 0; j<long_string; j++){
	
		letra = texto[j];
		comienzo = 25 * (letra - ' ');
		
		for (i = 0; i < 12; i++){
			
			valor1 = Arial12x12[comienzo+i*2+1];
			valor2 = Arial12x12[comienzo+i*2+2];
			
			if (i+posicionL1 < 128){
				if (i+posicionL1+Arial12x12[comienzo]>127){
						posicionL1 = 121;
				}
				else {
					buffer_LCD [i+posicionL1] = valor1;
					buffer_LCD [i+128+posicionL1] = valor2;
				}
			}
			else if ((i+posicionL1 >= 128) && (i+posicionL1+Arial12x12[comienzo] < 256)){
					return -1;
			}
			else return -1; // Hemos sobrepasado la memoria del display
	}
		posicionL1 = posicionL1 + Arial12x12[comienzo];
	}

	return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          print_line_2 (char texto[])
  \brief       Función que recibe un array de carácteres, los envía uno a uno a las funciones de escritura y controla el desbordamiento de memoria del buffer. 
							 Escribe únicamente en la segunda línea del lcd
  \param[in]   char texto[]      Array que contiene los datos que se van a mostrar por pantalla
*/
int print_line_2 (char texto[]){
	
	uint8_t i, j, valor1, valor2;
	uint16_t comienzo = 0;
	uint16_t posicionL1 = 0;
	uint8_t letra;
	uint8_t long_string = strlen(texto);
	
	for (j = 0; j<long_string; j++){
	
		letra = texto[j];
		comienzo = 25 * (letra - ' ');
		
		for (i = 0; i < 12; i++){
			
			valor1 = Arial12x12[comienzo+i*2+1];
			valor2 = Arial12x12[comienzo+i*2+2];
			
			if (i+posicionL1 < 128){
				if (i+posicionL1+Arial12x12[comienzo]>127){
						posicionL1 = 121;
				}
				else {
					buffer_LCD [i+256+posicionL1] = valor1;
					buffer_LCD [i+384+posicionL1] = valor2;
				}
			}
			else if ((i+posicionL1 >= 128) && (i+posicionL1+Arial12x12[comienzo] < 256)){
					return -1;
			}
			else return -1; // Hemos sobrepasado la memoria del display
	}
		posicionL1 = posicionL1 + Arial12x12[comienzo];
	}

	return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          buffer_clear()
  \brief       Función que borra el bufer de texto para que no se representen datos incorrectos 
*/
/**
*	Función que borra el bufer de texto para que no se representen datos incorrectos.
*/
void buffer_clear (){

	int a = 0;
	for (a=0; a<512; a++){
		buffer_LCD[a] = 0;
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
void lcd (void const *argument) {

	// Configuración del LCD
	init();
	LCD_reset ();
	buffer_clear ();
	copy_to_lcd();
	
  /*while (1) {
		
		printf_1_line_event = osSignalWait (signal_print_1_line, 100);
		printf_2_line_event = osSignalWait (signal_print_2_line, 100);
		
    if (printf_1_line_event.status == osEventSignal)  {
			print_line_1(lcd_text[0]);
			copy_to_lcd();
    }*/
		
		while(1) {
    if (LCDupdate == true) {
			buffer_clear ();
      print_line_1(lcd_text[0]);
			print_line_2(lcd_text[1]);
			copy_to_lcd();			
      LCDupdate = false;
			osDelay (5000);
    }
		else{
			for  (array_lcd = 0; array_lcd<20; array_lcd++){
				lcd_text[0][array_lcd] = hora[array_lcd];
				lcd_text[1][array_lcd] = fecha[array_lcd];
				
			}
			buffer_clear ();
      print_line_1(lcd_text[0]);
			print_line_2(lcd_text[1]);
			copy_to_lcd();	
		}
		feed_WDT();
    osDelay (250);
		osThreadYield ();                                           // suspend thread
  }
}

