#include "lcd.h"
#include "Arial12x12.h"

#define PORT_SSP1			0
#define PIN_nRESET_SSP1		8			
#define PIN_A0_SSP1				6
#define PIN_CS_SSP1				18


extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI *SPIDrv = &Driver_SPI1;

unsigned char buffer[512];
uint8_t posicionL1 = 0, posicionL2 = 0;



void retardo_us (uint32_t n_microsegundos){
	
LPC_SC->PCONP |= (1<<22); 											//Encendido del Timer 2
LPC_SC->PCLKSEL1 |= ((1<<13)|(1<<12)); 					//Asignacion del reloj de perifericos para el timer 0
LPC_TIM2->MR0 = (n_microsegundos*12.5); 				// Asignacion del valor de comparación 
LPC_TIM2->MCR |= 0x03; 													// Reset contandor del match 
LPC_TIM2->IR  |= (1<<0);												// Habilitacion de la interrupcion
LPC_TIM2 ->TCR = (0x10); 												//Resetear el contador   
LPC_TIM2 ->TCR = (0x01);												// Inicia el contador 
	while((LPC_TIM2->IR & 0x01)  !=0x01){

	}
	LPC_TIM2->IR  |= (1<<0);
	LPC_TIM2 ->TCR = (0x02); 	//Resetear el contador

}


void LCD_init(void){
	
	//Configuración DRIVER SPI
	SPIDrv->Initialize(NULL); //Inicializamos interfaz SPI
	
	SPIDrv->PowerControl(ARM_POWER_FULL);  
	
	SPIDrv->Control (ARM_SPI_MODE_MASTER |  //SPI Modo master 
									 ARM_SPI_CPOL1_CPHA1 |  //Envio datos flanco subida y escritura bit flanco
									 //ARM_SPI_SS_MASTER_UNUSED  | //
									 ARM_SPI_MSB_LSB |     	//Bits recibidos en orden de mayor a menor
									 ARM_SPI_DATA_BITS(8), 20000000); //Transferencias de 8 bits a 20MHz
	
	//Configuración e inicialización pines de control (Reset,CS y Comando/Dato)
	GPIO_SetDir(PORT_SSP1,PIN_A0_SSP1,GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_SSP1,PIN_CS_SSP1,GPIO_DIR_OUTPUT);
	GPIO_SetDir(PORT_SSP1,PIN_nRESET_SSP1,GPIO_DIR_OUTPUT);
	
	GPIO_PinWrite (PORT_SSP1, PIN_A0_SSP1, 0);
	GPIO_PinWrite (PORT_SSP1, PIN_nRESET_SSP1, 1);
	GPIO_PinWrite (PORT_SSP1, PIN_CS_SSP1, 1);
	GPIO_PinWrite (PORT_SSP1, PIN_nRESET_SSP1, 0);
	
	/*GPIO_PinWrite(PORT_SSP1,PIN_A0_SSP1,0);
	GPIO_PinWrite(PORT_SSP1,PIN_CS_SSP1,1);
	GPIO_PinWrite(PORT_SSP1,PIN_nRESET_SSP1,1);*/
	
	//Generar reset
	
	GPIO_PinWrite(PORT_SSP1,PIN_nRESET_SSP1,0);
	retardo_us(2); //Retardo necesario >1us
	GPIO_PinWrite(PORT_SSP1,PIN_nRESET_SSP1,1);
	retardo_us(1100); //Retardo necesario >1ms
	
}

void LCD_wr_cmd(unsigned char cmd){
	
	GPIO_PinWrite(PORT_SSP1,PIN_CS_SSP1,0);
	GPIO_PinWrite(PORT_SSP1,PIN_A0_SSP1,0);
	SPIDrv->Send(&cmd,sizeof(cmd));
	GPIO_PinWrite(PORT_SSP1,PIN_CS_SSP1,1);

}

void LCD_wr_dat(unsigned char dat){
	
	GPIO_PinWrite(PORT_SSP1,PIN_CS_SSP1,0);
	GPIO_PinWrite(PORT_SSP1,PIN_A0_SSP1,1);
	SPIDrv->Send(&dat,sizeof(dat));
	GPIO_PinWrite(PORT_SSP1,PIN_CS_SSP1,1);

}


void LCD_reset(void){
	
	LCD_wr_cmd(0xAE);	//Display off
  LCD_wr_cmd(0xA2);	//Tensión de polarización del LCD a 1/9 (0xA3 para 1/7)
  LCD_wr_cmd(0xA0);	//El direccionamiento de la RAM de datos del display es la normal
	LCD_wr_cmd(0xC8);	//El scan en las salidas COM es el normal
	LCD_wr_cmd(0x22);	//Fija la relación de resistencias interna a 2
	LCD_wr_cmd(0x2F);	//Power on
  LCD_wr_cmd(0x40);	//Display empieza en la línea 0
  LCD_wr_cmd(0xAF);	//Display ON
	LCD_wr_cmd(0x81);	//Fija el contraste
	LCD_wr_cmd(0x0F); //contraste //con 0x3F máximo contraste
	LCD_wr_cmd(0xA4); // Display all points normal
	LCD_wr_cmd(0xA6);	//LCD Display normal

}


void LCD_update(void){
	int i;
  LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
  LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
  LCD_wr_cmd(0xB0); // Página 0

  for(i=0;i<128;i++){
		LCD_wr_dat(buffer[i]);
	}

  LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
  LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
  LCD_wr_cmd(0xB1); // Página 1

  for(i=128;i<256;i++){
		LCD_wr_dat(buffer[i]);
	}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2); //Página 2
	
	for(i=256;i<384;i++){
		LCD_wr_dat(buffer[i]);
	}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3); // Pagina 3


	for(i=384;i<512;i++){
		LCD_wr_dat(buffer[i]);
	}
	
}

void Escribe_Letra_L1(uint8_t letra){
	
	uint8_t i, valor1, valor2;
	uint16_t comienzo = 0;
	
	comienzo = 25*(letra - ' ');
	
	for(i=0; i<12; i++){
		valor1 = Arial12x12[comienzo+i*2+1];
		valor2 = Arial12x12[comienzo+i*2+2];
		
		buffer[i+posicionL1] = valor1;
		buffer[i+128+posicionL1] = valor2;
	}
	
	posicionL1 = posicionL1+Arial12x12[comienzo];
}

void EscribeLinea_1(char *letras){
	uint8_t c;

	posicionL1 = 0;
	
	for (c = 0; c < strlen(letras); c++)
		Escribe_Letra_L1(letras[c]);
	
}


void Escribe_Letra_L2(uint8_t letra){
	
	uint8_t i, valor1, valor2;
	uint16_t comienzo = 0;
	
	comienzo = 25*(letra - ' ');
	
	for(i=0; i<12; i++){
		valor1 = Arial12x12[comienzo+i*2+1];
		valor2 = Arial12x12[comienzo+i*2+2];
		
		buffer[i+256+posicionL2] = valor1;
		buffer[i+384+posicionL2] = valor2;
	}
	
	posicionL2 = posicionL2+Arial12x12[comienzo];
}


void EscribeLinea_2(char *letras){
	uint8_t c;

	posicionL2 = 0;
	
	for (c = 0; c < strlen(letras); c++)
		Escribe_Letra_L2(letras[c]);
	
}


