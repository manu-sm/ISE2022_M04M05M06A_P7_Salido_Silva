
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"


#define PUERTO_POTENCIOMETROS 1
#define PIN_POT_1							30

#define ADC_RESOLUTION        12        /* Number of A/D converter bits       */

static volatile uint16_t AD_last;       /* Last converted value               */
static volatile uint8_t  AD_done;       /* AD conversion done flag            */

int32_t Init_Pot1 (void);
int32_t ADC_StartConversion (void);
int32_t ADC_ConversionDone (void);
int32_t ADC_GetValue(void);
uint32_t ADC_GetResolution (void);
void ADC_IRQHandler(void);

