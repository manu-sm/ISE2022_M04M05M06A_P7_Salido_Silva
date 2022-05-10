#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"

#define T_SEGUNDO						0
#define T_MINUTO						1
#define T_HORA  						2
#define T_DIA_MES						3
#define T_MES								4
#define T_ANIO					    5

void init_RTC (void);

void enable_RTC (void);

void set_hour (uint8_t hora_, uint8_t minuto_, uint8_t segundo_);

void set_date (uint8_t dia_, uint8_t mes_, uint16_t anio_);

void get_hora (void);
	
void get_fecha (void);

uint32_t get_RTC_param (uint8_t tipo);

void set_hour_intrrupt (uint8_t state);

void get_time (void);

void convert_unix_to_local (uint32_t time);

void rtc_control (void);

void get_registro_evento(uint8_t evento, uint8_t *buf);
