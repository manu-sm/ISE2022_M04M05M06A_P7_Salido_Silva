#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"


void init_RTC (void);

void enable_RTC (void);

void set_hour (uint8_t hora_, uint8_t minuto_, uint8_t segundo_);

void set_date (uint8_t dia_, uint8_t mes_, uint16_t anio_);

void get_hora (void);
	
void get_fecha (void);

void set_hour_intrrupt (uint8_t state);

