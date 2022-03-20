#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "SSP_LPC17xx.h"
#include "string.h"


#ifndef LCD_H
#define LCD_H
void retardo_us (uint32_t n_microsegundos);
void LCD_init(void);
void LCD_wr_cmd(unsigned char cmd);
void LCD_wr_dat(unsigned char dat);
void LCD_reset(void);
void LCD_update(void);
void Escribe_Letra_L1(uint8_t letra);
void EscribeLinea_1(char *letras);
void Escribe_Letra_L2(uint8_t letra);
void EscribeLinea_2(char *letras);
int Init_lcd (void);


#endif


