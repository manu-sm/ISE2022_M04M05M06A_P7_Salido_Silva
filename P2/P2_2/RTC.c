#include "RTC.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"
#include <string.h>
#include <stdio.h>

#define RTC_POWER_CONTROL		9		 /* Power control RTC*/
#define SBIT_CLKEN     			0    /* RTC Clock Enable*/
#define SBIT_CTCRST   		  1    /* RTC Clock Reset */
#define SBIT_CCALEN         4    /* RTC Calibration counter enable */
#define ILR_RTCCIF          0		 /* interrupt RTC counter */

uint16_t year;
uint8_t hour, min, sec, date, month;

uint8_t prueba = 2;

extern char hora [64];
extern char fecha [64];
extern bool flag_min;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          init_RTC (void)
  \brief       Funci�n que se encarga de inicializar el RTC
*/
void init_RTC (){

	LPC_SC->PCONP |= 1 << RTC_POWER_CONTROL; //Habilitamos alimentaci�n RTC
	LPC_RTC->ILR = 0x00; // Registro de interrupciones borrado.
	LPC_RTC->CCR = 0x00; // Registro de control inicializado.
	LPC_RTC->CIIR = 0x00; //Interrupciones por incremento de contador deshabilitadas
	LPC_RTC->AMR = 0xFF; // Enmascaradas alarmas RTC
	LPC_RTC->CALIBRATION = 0x00;
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          enable_RTC (void)
  \brief       Funci�n que se encarga de habilitar la cuenta del RTC
*/
void enable_RTC (){
	LPC_RTC->CCR |= 1 << SBIT_CTCRST;
	LPC_RTC->CCR &= (~(1 << SBIT_CTCRST))& 0x13;
	LPC_RTC->CCR |= 1 << SBIT_CLKEN;
	LPC_RTC->CCR |= 1 << SBIT_CCALEN;
	
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          set_hour (uint8_t hora, uint8_t minuto, uint8_t segundo)
  \brief       Funci�n que se encarga de fijar una hora en el RTC
	\param[in]   hora      		hora a configurar.
							 minuto				minuto a configurar.
							 segundo		  segundo a configurar.
*/
void set_hour (uint8_t hora_, uint8_t minuto_, uint8_t segundo_){
	
		LPC_RTC->HOUR   = hora_;   	// Update hour value 
		LPC_RTC->MIN    = minuto_;   // Update min value
		LPC_RTC->SEC    = segundo_;  // Update sec value
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          set_date (uint8_t dia, uint8_t mes, uint8_t anio)
  \brief       Funci�n que se encarga de fijar una fecha en el RTC
	\param[in]   dia      		dia a configurar.
							 mes				  mes a configurar.
							 anio		      a�o a configurar.
*/
void set_date (uint8_t dia_, uint8_t mes_, uint16_t anio_){

		LPC_RTC->DOM    = dia_;   	// Update date value 
		LPC_RTC->MONTH  = mes_;   	// Update month value
		LPC_RTC->YEAR   = anio_; 	// Update year value
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          get_hora (void)
  \brief       Funci�n que se encarga de leer la hora del sistema y almacenar el valor en el array @hora [64]
*/
void get_hora (){

	  hour = LPC_RTC->HOUR;
    min  = LPC_RTC->MIN; 
    sec  = LPC_RTC->SEC; 
	  
		sprintf(hora,"%.2d:%.2d:%.2d",hour,min,sec);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          get_fecha (void)
  \brief       Funci�n que se encarga de leer la fecha del sistema y almacenar el valor en el array @fecha [64]
*/
void get_fecha (){
	
    date  = LPC_RTC->DOM;   
    month = LPC_RTC->MONTH;  
    year  = LPC_RTC->YEAR; 
		
		sprintf(fecha,"%.2d:%.2d:%4u",date,month,year);
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          set_hour_intrrupt (uint8_t state)
  \brief       Funci�n que se encarga de configurar las interrupciones provocadas por incremento en la hora.
	\param[in]   XX1      		interrupci�n de segundos activada.
							 X1X		  		interrupci�n de minutos activada.
							 1XX		      interrupci�n de horas activada.
*/
void set_hour_intrrupt (uint8_t state){
	
	// Evaluamos si hay que activar las interrupciones por incremento de segundos
	if (state == 1 << 0){
			LPC_RTC -> CIIR |= 1 << 0; 		// Habilita interrupci�n por incremento en contador segundos 
	}
	else {
			LPC_RTC -> CIIR &= ~(1 << 0);		// Deshabilita interrupci�n por incremento en contador segundos 
	}
	
	// Evaluamos si hay que activar las interrupciones por incremento de minutos
	if (state == 1 << 1){
			LPC_RTC -> CIIR |= 1 << 1; 		// Habilita interrupci�n por incremento en contador segundos 
	}
	else {
			LPC_RTC -> CIIR &= ~(1 << 1);		// Deshabilita interrupci�n por incremento en contador segundos 
	}
	
	// Evaluamos si hay que activar las interrupciones por incremento de horas
	if (state == 1 << 2){
			LPC_RTC -> CIIR |= 1 << 2; 		// Habilita interrupci�n por incremento en contador segundos 
	}
	else {
			LPC_RTC -> CIIR &= ~(1 << 2);		// Deshabilita interrupci�n por incremento en contador segundos 
	}
}

void RTC_IRQHandler(void){
	
	if((LPC_RTC->ILR&0x01) == 1){
		LPC_RTC->ILR |= 1 << ILR_RTCCIF;
		flag_min = true;
	}
}
	