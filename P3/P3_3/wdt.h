#ifndef __WDT_H__
#define __WDT_H__

#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"

#define PUERTO_LED 				1
#define LED_1 						18
#define LED_2 						20
#define LED_3 						21
#define LED_4 						23

void feed_WDT(void);
void config_WDT(void);

#endif  // __WDT_H__

