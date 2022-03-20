#include "RTC.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"
#include <string.h>
#include <stdio.h>
#include "cmsis_os.h"                   /* CMSIS RTOS definitions             */
#include "rl_net.h" 
#include "time.h"
#include "lcd.h"

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
extern char lcd_text[2][20+1];
bool flag_min;

time_t tiempo_unix;


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          init_RTC (void)
  \brief       Función que se encarga de inicializar el RTC
	
							
*/
void init_RTC (){

	LPC_SC->PCONP |= 1 << RTC_POWER_CONTROL; //Habilitamos alimentación RTC
	LPC_RTC->ILR = 0x00; // Registro de interrupciones borrado.
	LPC_RTC->CCR = 0x00; // Registro de control inicializado.
	LPC_RTC->CIIR = 0x00; //Interrupciones por incremento de contador deshabilitadas
	LPC_RTC->AMR = 0xFF; // Enmascaradas alarmas RTC
	LPC_RTC->CALIBRATION = 0x00;
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          enable_RTC (void)
  \brief       Función que se encarga de habilitar la cuenta del RTC
	\param[in]   local      		la hora se configura localmente, sin el uso de ningún servidor sntp.
							 sntp						la hora se configura con un servidor sntp.
*/
void enable_RTC (){
	
	LPC_RTC->CCR |= 1 << SBIT_CTCRST;								// Reset 
	LPC_RTC->CCR &= (~(1 << SBIT_CTCRST))& 0x13;		
	LPC_RTC->CCR |= 1 << SBIT_CLKEN;								// CLK clok enable
	LPC_RTC->CCR |= 1 << SBIT_CCALEN;
		
	get_time();
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          set_hour (uint8_t hora, uint8_t minuto, uint8_t segundo)
  \brief       Función que se encarga de fijar una hora en el RTC
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
  \brief       Función que se encarga de fijar una fecha en el RTC
	\param[in]   dia      		dia a configurar.
							 mes				  mes a configurar.
							 anio		      año a configurar.
*/
void set_date (uint8_t dia_, uint8_t mes_, uint16_t anio_){

		LPC_RTC->DOM    = dia_;   	// Update date value 
		LPC_RTC->MONTH  = mes_;   	// Update month value
		LPC_RTC->YEAR   = anio_; 	// Update year value
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          get_hora (void)
  \brief       Función que se encarga de leer la hora del sistema y almacenar el valor en el array @hora [64]
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
  \brief       Función que se encarga de leer la fecha del sistema y almacenar el valor en el array @fecha [64]
*/
void get_fecha (){
	
    date  = LPC_RTC->DOM;   
    month = LPC_RTC->MONTH;  
    year  = LPC_RTC->YEAR; 
		
		sprintf(fecha,"%.2d/%.2d/%4u",date,month,year);
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          get_RTC_param (void)
  \brief       Función que devuelve el dato que se le pide como parametro del RTC
*/
uint32_t get_RTC_param (uint8_t	tipo){
	
	uint32_t dato;
	
	if(tipo == T_SEGUNDO) dato = LPC_RTC->SEC;
	if(tipo == T_MINUTO) dato = LPC_RTC->MIN;
	if(tipo == T_HORA) dato = LPC_RTC->HOUR;
	if(tipo == T_DIA_MES) dato = LPC_RTC->DOM;
	if(tipo == T_MES) dato = LPC_RTC->MONTH;
	if(tipo == T_ANIO) dato = LPC_RTC->YEAR;
	
	return dato;	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  \fn          set_hour_intrrupt (uint8_t state)
  \brief       Función que se encarga de configurar las interrupciones provocadas por incremento en la hora.
	\param[in]   XX1      		interrupción de segundos activada.
							 X1X		  		interrupción de minutos activada.
							 1XX		      interrupción de horas activada.
*/
void set_hour_intrrupt (uint8_t state){
	
	// Evaluamos si hay que activar las interrupciones por incremento de segundos
	if (state == 1 << 0){
			LPC_RTC -> CIIR |= 1 << 0; 		// Habilita interrupción por incremento en contador segundos 
	}
	else {
			LPC_RTC -> CIIR &= ~(1 << 0);		// Deshabilita interrupción por incremento en contador segundos 
	}
	
	// Evaluamos si hay que activar las interrupciones por incremento de minutos
	if (state == 1 << 1){
			LPC_RTC -> CIIR |= 1 << 1; 		// Habilita interrupción por incremento en contador segundos 
	}
	else {
			LPC_RTC -> CIIR &= ~(1 << 1);		// Deshabilita interrupción por incremento en contador segundos 
	}
	
	// Evaluamos si hay que activar las interrupciones por incremento de horas
	if (state == 1 << 2){
			LPC_RTC -> CIIR |= 1 << 2; 		// Habilita interrupción por incremento en contador segundos 
	}
	else {
			LPC_RTC -> CIIR &= ~(1 << 2);		// Deshabilita interrupción por incremento en contador segundos 
	}
	
	NVIC_EnableIRQ(RTC_IRQn);

}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
static void time_cback (uint32_t time);
 
void get_time (void) {
  const uint8_t ntp_server[4] = {130,206,0,1};
 
  if (sntp_get_time (&ntp_server[0], time_cback) == netOK) {
    sprintf (lcd_text[0],"SNTP request sent");
  }
  else {
    sprintf (lcd_text[0],"Failed, SNTP not ready");
  }
}
 
static void time_cback (uint32_t time) {
  if (time == 0) {
    sprintf (lcd_text[0],"Error, no SNTP answer");
  }
  else {
		//sprintf (lcd_text[0], "no error");
		tiempo_unix = time;
		convert_unix_to_local (tiempo_unix);
		set_hour (hour, min, sec);
		set_date (date, month, year);
		EscribeLinea_1(hora);
		EscribeLinea_2(fecha);
		LCD_update();
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void convert_unix_to_local (uint32_t time){
	
	time_t ahora = time;
	struct tm* hora_local = localtime(&ahora);
	
	year  = hora_local->tm_year+1900;	// Te devuelve los años desde 1900
	hour  = hora_local->tm_hour;
	min   = hora_local->tm_min;
	sec   = hora_local->tm_sec;
	date  = hora_local->tm_mday; 
	month = hora_local->tm_mon+1;		  // Los meses son de 0 a 11	
}


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void rtc_control (void){

	uint8_t contador;
	
	init_RTC ();
	enable_RTC();
	set_hour_intrrupt (2);
	get_hora ();
	get_fecha ();
	EscribeLinea_1(hora);
	EscribeLinea_2(fecha);
	LCD_update();
	
	while (1){
			if (flag_min == true) {
				if (contador == 3){
					contador = 0;
					get_time();
					//convert_unix_to_local (tiempo_unix);
					//set_hour (hour, min, sec);
					//set_date (date, month, year);
				}
				else {
				contador++;
			}
			  flag_min = false;
		}
			get_hora ();
			get_fecha ();
			EscribeLinea_1(hora);
			EscribeLinea_2(fecha);
			LCD_update();
	}	
}


void RTC_IRQHandler(void){
	
	if((LPC_RTC->ILR&0x01) == 1){
		LPC_RTC->ILR |= 1 << ILR_RTCCIF;
		flag_min = true;
	}
}
