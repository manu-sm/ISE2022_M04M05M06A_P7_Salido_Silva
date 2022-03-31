
#include "potenciometro.h"

int32_t Init_Pot1 (void) {

  LPC_SC->PCONP |= ((1 << 12) | (1 << 15));  /* enable power to ADC & IOCON   */

  PIN_Configure (PUERTO_POTENCIOMETROS, PIN_POT_1, PIN_FUNC_3, PIN_PINMODE_TRISTATE, PIN_PINMODE_NORMAL);
  
  LPC_ADC->ADCR    =  ( 1 <<  4) |           /* select AD0.2 pin              */
                      ( 4 <<  8) |           /* ADC clock is 25MHz/5          */
                      ( 1 << 21);            /* enable ADC                    */

  LPC_ADC->ADINTEN =  ( 1 <<  8);            /* global ADC enable interrupt   */

  NVIC_EnableIRQ(ADC_IRQn);                  /* enable ADC Interrupt          */	

  return 0;
}

/**
  \fn          int32_t ADC_StartConversion (void)
  \brief       Start conversion
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t ADC_StartConversion (void) {

  LPC_ADC->ADCR &= ~( 7 << 24);              /* stop conversion               */
  LPC_ADC->ADCR |=  ( 1 << 24);              /* start conversion              */

  return 0;
}

/**
  \fn          int32_t ADC_ConversionDone (void)
  \brief       Check if conversion finished
  \returns
   - \b  0: conversion finished
   - \b -1: conversion in progress
*/
int32_t ADC_ConversionDone (void) {
  return (AD_done ? 0 : -1);
}

/**
  \fn          int32_t ADC_GetValue (void)
  \brief       Get converted value
  \returns
   - <b> >=0</b>: converted value
   - \b -1: conversion in progress or failed
*/
int32_t ADC_GetValue (void) {

  if (AD_done) {
    AD_done = 0;
    return AD_last;
  }
  return -1;
}

/**
  \fn          uint32_t ADC_GetResolution (void)
  \brief       Get resolution of Analog-to-Digital Converter
  \returns     Resolution (in bits)
*/
uint32_t ADC_GetResolution (void) {
  return ADC_RESOLUTION;
}

/**
  \fn          void ADC_IRQHandler (void)
  \brief       Analog-to-Digital Converter Interrupt Handler
*/
void ADC_IRQHandler(void) {
  volatile uint32_t adstat;

  adstat = LPC_ADC->ADSTAT;                        /* Read ADC clears interrupt     */

  AD_last = (LPC_ADC->ADGDR >> 4) & 0xFFF;         /* Store converted value   */

  AD_done = 1;
}

