/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server_CGI.c
 * Purpose: HTTP Server CGI Module
 * Rev.:    V6.00
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "rl_net.h"
#include "rl_net_lib.h"
#include "Board_LED.h"

#define osObjectsPublic                     // define objects in main module
#include <cmsis_os.h>

#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"
#include "RTC.h"
#ifndef _LPC17xx_IAP_H
#include "lpc17xx_iap.h"
#endif

#define PUERTO_LED 			1
#define LED_1 					18
#define LED_2 					20
#define LED_3 					21
#define LED_4 					23

#define EV_GANANCIA_1   			0x00
#define EV_GANANCIA_5   			0x01
#define EV_GANANCIA_10  			0x02
#define EV_GANANCIA_50  			0x03
#define EV_GANANCIA_100 			0x04
#define EV_CHANGE_OVERLOAD		0X05
#define EV_INT_OVERLOAD_ON		0x06
#define EV_INT_OVERLOAD_OFF		0x07

#define signal_i2c  					0x100


// http_server.c
extern uint16_t AD_in (uint32_t ch);
extern uint8_t  get_button (void);
extern bool rtc_update;
extern uint8_t situacion_leds;
extern uint8_t estado_agp;	

const char hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
// net_sys.c
extern  LOCALM localm[];
#define LocM   localm[NETIF_ETH]

// Net_Config.c
extern struct tcp_cfg   tcp_config;
extern struct http_cfg  http_config;
#define tcp_NumSocks    tcp_config.NumSocks
#define tcp_socket      tcp_config.Scb
#define http_EnAuth     http_config.EnAuth
#define http_auth_passw http_config.Passw

extern bool LEDrun;
extern bool LCDupdate;
extern bool rtc_update;
extern char lcd_text[2][20+1];
extern char hora [64];
extern char fecha [64];
extern uint8_t umbral[1];
extern bool SNTP_server;
bool borrado_flash;
char contenido_memoria[210];
char lectura_memoria[70];
uint8_t ganancia = 1;
char umbral_OL[4] = {0,0,0,0};
uint8_t overload_th = 0;
bool interrupcion_OL = false;
bool estado_OL = false;
extern uint8_t num_eventos[1];
extern osThreadId tid_I2C; 

// Local variables.
static uint8_t P2;
uint16_t hour_date [6];
char config_text [2][21];
char umbral_adc[3];
uint8_t buffer_eventos[8];
uint8_t registro_leido = 1;
char registro_pedido[3] = {1,0,0};

// My structure of CGI status variable.
typedef struct {
  uint16_t xcnt;
  uint16_t unused;
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)


void get_registro_evento(uint8_t evento, uint8_t *buf){
	
	buf[0] = evento;
	buf[1] = get_RTC_param(T_HORA);
	buf[2] = get_RTC_param(T_MINUTO);
	buf[3] = get_RTC_param(T_SEGUNDO);
	buf[4] = get_RTC_param(T_DIA_MES);
	buf[5] = get_RTC_param(T_MES);
	buf[6] = get_RTC_param(T_ANIO) >> 8;
	buf[7] = get_RTC_param(T_ANIO) & 0xFF;
}


// Process query string received by GET request.
void cgi_process_query (const char *qstr) {
  char var[40];

  do {
    // Loop through all the parameters
    qstr = http_get_env_var (qstr, var, sizeof (var));
    // Check return string, 'qstr' now points to the next parameter
    if (var[0] != 0) {
      // First character is non-null, string exists
      if (strncmp (var, "ip=", 3) == 0) {
        // Local IP address
        ip4_aton (&var[3], LocM.IpAddr);
      }
      else if (strncmp (var, "msk=", 4) == 0) {
        // Local network mask
        ip4_aton (&var[4], LocM.NetMask);
      }
      else if (strncmp (var, "gw=", 3) == 0) {
        // Default gateway IP address
        ip4_aton (&var[3], LocM.DefGW);
      }
      else if (strncmp (var, "pdns=", 5) == 0) {
        // Primary DNS server IP address
        ip4_aton (&var[5], LocM.PriDNS);
      }
      else if (strncmp (var, "sdns=", 5) == 0) {
        // Secondary DNS server IP address
        ip4_aton (&var[5], LocM.SecDNS);
      }
    }
  } while (qstr);
}

// Process data received by POST request.
// Type code: - 0 = www-url-encoded form data.
//            - 1 = filename for file upload (null-terminated string).
//            - 2 = file upload raw data.
//            - 3 = end of file upload (file close requested).
//            - 4 = any XML encoded POST data (single or last stream).
//            - 5 = the same as 4, but with more XML data to follow.
void cgi_process_data (uint8_t code, const char *data, uint32_t len) {
  char var[40],passw[12];

  if (code != 0) {
    // Ignore all other codes
    return;
  }

  P2 = 0;
	GPIO_PortWrite(PUERTO_LED,0x00B40000,P2);
  LEDrun = true;
	situacion_leds = 0x10;
  if (len == 0) {
    // No data or all items (radio, checkbox) are off
    LED_SetOut (P2);
    return;
  }
  passw[0] = 1;
  do {
    // Parse all parameters
    data = http_get_env_var (data, var, sizeof (var));
    if (var[0] != 0) {
      // First character is non-null, string exists
      /*if (strcmp (var, "led0=on") == 0) {
        P2 |= 0x01;
				situacion_leds |= 0x01;
				GPIO_PinWrite(PUERTO_LED,LED_1,1);
      }
      else if (strcmp (var, "led1=on") == 0) {
        P2 |= 0x02;
				situacion_leds |= 0x02;
				GPIO_PinWrite(PUERTO_LED,LED_2,1);
      }
      else if (strcmp (var, "led2=on") == 0) {
        P2 |= 0x04;
				situacion_leds |= 0x04;
				GPIO_PinWrite(PUERTO_LED,LED_3,1);
      }
      else if (strcmp (var, "led3=on") == 0) {
        P2 |= 0x08;
				situacion_leds |= 0x08;
				GPIO_PinWrite(PUERTO_LED,LED_4,1);
      }*/
      if (strcmp (var, "ctrl=1") == 0) {
				if (ganancia != 1){
					ganancia = 1;
					get_registro_evento(EV_GANANCIA_1,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
					osSignalSet (tid_I2C, signal_i2c);
					estado_agp = EV_GANANCIA_1;
				}
      }
			else if (strcmp (var, "ctrl=5") == 0) {
				if (ganancia != 5){
					estado_agp =EV_GANANCIA_5;
					ganancia = 5;
					osSignalSet (tid_I2C, signal_i2c);
					get_registro_evento(EV_GANANCIA_5,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
				}
      }
			else if (strcmp (var, "ctrl=10") == 0) {
        if (ganancia != 10){
					estado_agp =EV_GANANCIA_10;
					ganancia = 10;
					get_registro_evento(EV_GANANCIA_10,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
					osSignalSet (tid_I2C, signal_i2c);
				}
      }
			else if (strcmp (var, "ctrl=50") == 0) {
				if (ganancia != 50){
					estado_agp = EV_GANANCIA_50;
					ganancia = 50;
					get_registro_evento(EV_GANANCIA_50,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
					osSignalSet (tid_I2C, signal_i2c);
				}
      }
			else if (strcmp (var, "ctrl=100") == 0) {
        if (ganancia != 100){
					estado_agp = EV_GANANCIA_100;
					ganancia = 100;
					get_registro_evento(EV_GANANCIA_100,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
					osSignalSet (tid_I2C, signal_i2c);
				}
      }
			else if (strncmp (var, "umbral_OL=", 5) == 0) {
				
				// Config Umbral OverLoad
				strcpy (umbral_OL, var + 10);
				if(overload_th != ((umbral_OL[0] - '0')*10 + umbral_OL[2] - '0')){
					if(umbral_OL[0] != 0){
						overload_th = (umbral_OL[0] - '0')*10;
						if(umbral_OL[1] != 0){
							if(umbral_OL[2] != 0) overload_th += (umbral_OL[2] - '0');
						}
					}
					get_registro_evento(overload_th|0x80,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
					estado_agp = EV_CHANGE_OVERLOAD;
					osSignalSet (tid_I2C, signal_i2c);
				}
      }
			else if (strcmp (var, "ctrl2=Activar") == 0) {
        if(interrupcion_OL == false){
					interrupcion_OL = true;
					/*estado_agp = EV_INT_OVERLOAD_ON;
					osSignalSet (tid_I2C, signal_i2c);*/
				}
      }
			else if (strcmp (var, "ctrl2=Desactivar") == 0) {
				if(interrupcion_OL == true){
					interrupcion_OL = false;
					/*estado_agp = EV_INT_OVERLOAD_OFF;
					osSignalSet (tid_I2C, signal_i2c);*/
				}
      }
			else if (strncmp (var, "num_reg=",6) == 0) {
				strcpy (registro_pedido, var+8);
				if((registro_pedido[2]!=0 && registro_pedido[1]!=0 && registro_pedido[0]!=0))
					registro_leido = (registro_pedido[0] - '0')*100 + (registro_pedido[1] - '0')*10 + (registro_pedido[2] - '0');
				else if((registro_pedido[0]!=0 && registro_pedido[1]!=0 && registro_pedido[2]==0))
					registro_leido = (registro_pedido[0] - '0')*10 + (registro_pedido[1] - '0');
				else if((registro_pedido[0]!=0 && registro_pedido[1]==0 && registro_pedido[2]==0))
					registro_leido = (registro_pedido[0] - '0');
      }
      else if ((strncmp (var, "pw0=", 4) == 0) ||
               (strncmp (var, "pw2=", 4) == 0)) {
        // Change password, retyped password
        if (http_EnAuth) {
          if (passw[0] == 1) {
            strcpy (passw, var+4);
          }
          else if (strcmp (passw, var+4) == 0) {
            // Both strings are equal, change the password
            strcpy (http_auth_passw, passw);
          }
        }
      }
			else if (strncmp (var, "hora=", 5) == 0) {
        // Config hora text
        strcpy (config_text[0], var+5);
				hour_date[0] = (config_text[0][0] - '0')*10 + config_text[0][1] - '0';
				hour_date[1] = (config_text[0][3] - '0')*10 + config_text[0][4] - '0';
				hour_date[2] = (config_text[0][6] - '0')*10 + config_text[0][7] - '0';
				set_hour (hour_date[0],hour_date[1],hour_date[2]);
        LCDupdate = true;
      }
			else if (strncmp (var, "fecha=", 5) == 0) {
        // Config fechatext
        strcpy (config_text[1], var+6);
				hour_date[3] = (config_text[1][0] - '0')*10 + config_text[1][1] - '0';
				hour_date[4] = (config_text[1][3] - '0')*10 + config_text[1][4] - '0';
				hour_date[5] = (config_text[1][6] - '0')*1000 + (config_text[1][7] - '0')*100 + (config_text[1][8] - '0')*10 + (config_text[1][9] - '0');
				set_date (hour_date[3],hour_date[4],hour_date[5]);
        LCDupdate = true;
      }
			else if (strcmp (var, "sntp_1=on") == 0) {
				SNTP_server = false;
      }
			else if (strcmp (var, "sntp_2=on") == 0) {
				SNTP_server = true;
      }
			/*else if (strcmp (var, "erase_flash=on") == 0) {
				borrado_flash = true;
				EraseSector(18,18);
				borrado_flash = false;
      }*/
			else if (strncmp (var, "umbral_adc=", 5) == 0) {
				// Config adc umbral
				strcpy (umbral_adc, var + 11);
				umbral [0] = (umbral_adc[0] - '0')*100 + (umbral_adc[1] - '0')*10 + (umbral_adc[2] - '0');
				if (umbral[0]> 255 ){
					umbral[0] = 255;
				}
				escribir_posicion(12,1,umbral);
      }
			else if (strcmp (var, "update=on") == 0) {
				get_time();
      }
    }
  } while (data);
	escribir_posicion(11,1,&situacion_leds);
}

// Generate dynamic web data from a script line.
uint32_t cgi_script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
  //TCP_INFO *tsoc;
  //const char *lang;
  uint32_t len = 0;
  //uint8_t id;
  //static uint32_t adv;
	uint32_t segundo_, minuto_, hora_, dia_, mes_, anio_;
	uint32_t k;
	
  switch (env[0]) {
    // Analyze a 'c' script line starting position 2
    /*case 'a' :
      // Network parameters from 'network.cgi'
      switch (env[2]) {
        case 'i':
          // Write local IP address
          len = sprintf (buf, &env[4], ip4_ntoa (LocM.IpAddr));
          break;
        case 'm':
          // Write local network mask
          len = sprintf (buf, &env[4], ip4_ntoa (LocM.NetMask));
          break;
        case 'g':
          // Write default gateway IP address
          len = sprintf (buf, &env[4], ip4_ntoa (LocM.DefGW));
          break;
        case 'p':
          // Write primary DNS server IP address
          len = sprintf (buf, &env[4], ip4_ntoa (LocM.PriDNS));
          break;
        case 's':
          // Write secondary DNS server IP address
          len = sprintf (buf, &env[4], ip4_ntoa (LocM.SecDNS));
          break;
      }
      break;*/

    case 'b':
      // AGP control by "agp.cgi'
      if (env[2] == 'c') {
        // Select Control
				if (ganancia == 1){
        len = sprintf (buf, &env[4], "selected","","","","");
				}
				else if (ganancia == 5){
        len = sprintf (buf, &env[4], "","selected","","","");
				}
				else if (ganancia == 10){
        len = sprintf (buf, &env[4], "","","selected","","");
				}
				else if (ganancia == 50){
        len = sprintf (buf, &env[4], "","","","selected","");
				}
				else if (ganancia == 100){
        len = sprintf (buf, &env[4],"","","","","selected");
				}
				break;
      }
			
			if (env[2] == 'd') {
				len = sprintf (buf, &env[4],umbral_OL);
				break;
      }
			
			if (env[2] == 'e') {
        // Select Control
				if (interrupcion_OL) len = sprintf (buf, &env[4], "selected","");				
				else len = sprintf (buf, &env[4], "","selected");
				break;
			}
		
		case 'd':
			// Config
			switch (env[2]) {
				case '1':
					len = sprintf (buf, &env[4],registro_leido);
				break;
				case '2':
					for(k = 0; k<8; k++){
						lectura_memoria[k] = leer_posicion(registro_leido*8+k);
						contenido_memoria[3*k] = hex[(lectura_memoria[k] >> 4)];
						contenido_memoria[3*k+1] =hex[(lectura_memoria[k] & 0x0F)];
						contenido_memoria[3*k+2] =' ';
					}
					len = sprintf (buf, &env[4],contenido_memoria);
				break;
      }		
			break;
			
		case 'i':
			// Config
			switch (env[2]) {
        case '1':
					strcpy (config_text[0], lcd_text[0]);
          len = sprintf (buf, &env[4], config_text[0]);
          break;
        case '2':
          strcpy (config_text[1], lcd_text[1]);
					len = sprintf (buf, &env[4], config_text[1]);
          break;
				case '3':
					len = sprintf (buf, &env[4], umbral[0]);
				break;
				case '4':
					len = sprintf (buf, &env[4],SNTP_server ?     ""     : "selected");
					break;
				case '5':
					len = sprintf (buf, &env[4],SNTP_server ?     "selected"     : "");
				break;
				/*case '6':
					len = sprintf (buf, &env[4],borrado_flash ?     "selected"     : "");
				break;
				case '7':
					for(k = 0; k<70; k++){
						lectura_memoria[k] = leer_posicion(k);
						contenido_memoria[3*k] = hex[(lectura_memoria[k] >> 4)];
						contenido_memoria[3*k+1] =hex[(lectura_memoria[k] & 0x0F)];
						contenido_memoria[3*k+2] =' ';
					}
					len = sprintf (buf, &env[4],contenido_memoria);
				break;*/
      }		
			break;
	
		case 'l':			
      // Overload State
			len = sprintf (buf, &env[4],estado_OL ?     "True"     : "False");
      break;
		
		case 'h':
			// Time and date
		  segundo_ = get_RTC_param(T_SEGUNDO);
			minuto_ = get_RTC_param(T_MINUTO);
			hora_ = get_RTC_param(T_HORA);
			dia_ = get_RTC_param(T_DIA_MES);
		  mes_ = get_RTC_param(T_MES);
		  anio_ = get_RTC_param(T_ANIO);
		
			switch (env[2]) {
        case '1':
          len = sprintf (buf, &env[4], hora_, minuto_, segundo_);
          break;
        case '2':
          len = sprintf (buf, &env[4], dia_, mes_, anio_);
          break;
				case '3':
          len = sprintf (buf, &env[4],"");
          break;
				
      }		
			rtc_update = true;
			break;
			
  }
  return (len);
}



