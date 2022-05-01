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

#define EV_GANANCIA_1   0x00
#define EV_GANANCIA_5   0x01
#define EV_GANANCIA_10  0x02
#define EV_GANANCIA_50  0x03
#define EV_GANANCIA_100 0x04



// http_server.c
extern uint16_t AD_in (uint32_t ch);
extern uint8_t  get_button (void);
extern bool rtc_update;
extern uint8_t situacion_leds;

const char hex[16] = {'0','1','2','3','4','4','6','7','8','9','A','B','C','D','E','F'};
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
char umbral_OL[3];
float overload_th;
bool interrupcion_OL = false;
extern uint8_t num_eventos[1];

// Local variables.
static uint8_t P2;
uint16_t hour_date [6];
char config_text [2][21];
char umbral_adc[3];
uint8_t buffer_eventos[8];

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
      if (strcmp (var, "led0=on") == 0) {
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
      }
      else if (strcmp (var, "ctrl=1") == 0) {
				if (ganancia != 1){
					ganancia = 1;
					get_registro_evento(EV_GANANCIA_1,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
				}
      }
			else if (strcmp (var, "ctrl=5") == 0) {
        if (ganancia != 5){
					ganancia = 5;
					get_registro_evento(EV_GANANCIA_5,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
				}
      }
			else if (strcmp (var, "ctrl=10") == 0) {
        if (ganancia != 10){
					ganancia = 10;
					get_registro_evento(EV_GANANCIA_10,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
				}
      }
			else if (strcmp (var, "ctrl=50") == 0) {
				if (ganancia != 50){
					ganancia = 50;
					get_registro_evento(EV_GANANCIA_50,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
				}
      }
			else if (strcmp (var, "ctrl=100") == 0) {
        if (ganancia != 100){
					ganancia = 100;
					get_registro_evento(EV_GANANCIA_100,buffer_eventos);
					escribir_posicion(num_eventos[0]*8,8,buffer_eventos);
					if(num_eventos[0]++ == 128) num_eventos[0] = 1 ;
					escribir_posicion(0,1,num_eventos);
				}
      }
			else if (strncmp (var, "umbral_OL=", 5) == 0) {
				// Config Umbral OverLoad
				strcpy (umbral_OL, var + 10);
				/*if (umbral_OL[0]> 255 ){
					umbral_OL[0] = 255;
				}*/
				if(umbral_OL[0] != 0){
					overload_th = umbral_OL[0] - '0';
					if(umbral_OL[1] != 0){
						if(umbral_OL[2] != 0) overload_th += (umbral_OL[2] - '0')*0.1;
					}
				} 
      }
			else if (strcmp (var, "ctrl2=Activar") == 0) {
        interrupcion_OL = true;
      }
			else if (strcmp (var, "ctrl2=Desactivar") == 0) {
        interrupcion_OL = false;
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
      else if (strncmp (var, "lcd1=", 5) == 0) {
        // LCD Module line 1 text
        strcpy (lcd_text[0], var+5);
				escribir_posicion_char(13,21,&lcd_text[0][0]);
        LCDupdate = true;
      }
      else if (strncmp (var, "lcd2=", 5) == 0) {
        // LCD Module line 2 text
        strcpy (lcd_text[1], var+5);
				escribir_posicion_char(34,21,&lcd_text[1][0]);
        LCDupdate = true;
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
			else if (strcmp (var, "erase_flash=on") == 0) {
				borrado_flash = true;
				EraseSector(18,18);
				borrado_flash = false;
      }
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
  TCP_INFO *tsoc;
  const char *lang;
  uint32_t len = 0;
  uint8_t id;
  static uint32_t adv;
	uint32_t segundo_, minuto_, hora_, dia_, mes_, anio_;
	uint32_t k;
	
  switch (env[0]) {
    // Analyze a 'c' script line starting position 2
    case 'a' :
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
      break;

    case 'b':
      // LED control from 'led.cgi'
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


    case 'c':
      // TCP status from 'tcp.cgi'
      while ((len + 150) < buflen) {
        tsoc = &tcp_socket[MYBUF(pcgi)->xcnt];
        MYBUF(pcgi)->xcnt++;
        // 'sprintf' format string is defined here
        len += sprintf (buf+len,   "<tr align=\"center\">");
        if (tsoc->State <= tcpStateCLOSED) {
          len += sprintf (buf+len, "<td>%d</td><td>%s</td><td>-</td><td>-</td>"
                                   "<td>-</td><td>-</td></tr>\r\n",
                                   MYBUF(pcgi)->xcnt,tcp_ntoa(tsoc->State));
        }
        else if (tsoc->State == tcpStateLISTEN) {
          len += sprintf (buf+len, "<td>%d</td><td>%s</td><td>%d</td><td>-</td>"
                                   "<td>-</td><td>-</td></tr>\r\n",
                                   MYBUF(pcgi)->xcnt, tcp_ntoa(tsoc->State), tsoc->LocPort);
        }
        else {
          len += sprintf (buf+len, "<td>%d</td><td>%s</td><td>%d</td>"
                                   "<td>%d</td><td>%s</td><td>%d</td></tr>\r\n",
                                   MYBUF(pcgi)->xcnt, tcp_ntoa(tsoc->State), tsoc->LocPort,
                                   tsoc->AliveTimer, ip4_ntoa (tsoc->RemAddr), tsoc->RemPort);
        }
        // Repeat for all TCP Sockets
        if (MYBUF(pcgi)->xcnt == tcp_NumSocks) {
          break;
        }
      }
      if (MYBUF(pcgi)->xcnt < tcp_NumSocks) {
        // Hi bit is a repeat flag
        len |= (1u << 31);
      }
      break;

    case 'd':
      // System password from 'system.cgi'
      switch (env[2]) {
        case '1':
          len = sprintf (buf, &env[4], http_EnAuth ? "Enabled" : "Disabled");
          break;
        case '2':
          len = sprintf (buf, &env[4], http_auth_passw);
          break;
      }
      break;

    case 'e':
      // Browser Language from 'language.cgi'
      lang = http_server_get_lang ();
      if      (strncmp (lang, "en", 2) == 0) {
        lang = "English";
      }
      else if (strncmp (lang, "de", 2) == 0) {
        lang = "German";
      }
      else if (strncmp (lang, "fr", 2) == 0) {
        lang = "French";
      }
      else if (strncmp (lang, "sl", 2) == 0) {
        lang = "Slovene";
      }
      else {
        lang = "Unknown";
      }
      len = sprintf (buf, &env[2], lang, http_server_get_lang());
      break;

    case 'f':
      // LCD Module control from 'lcd.cgi'
      switch (env[2]) {
        case '1':
          len = sprintf (buf, &env[4], lcd_text[0]);
					rtc_update = false;
          break;
        case '2':
          len = sprintf (buf, &env[4], lcd_text[1]);
					rtc_update = false;
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
				case '6':
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
				break;
      }		
			break;

    case 'g':
      // AD Input from 'ad.cgi'
      switch (env[2]) {
        case '1':
          adv = AD_in (0);
          len = sprintf (buf, &env[4], adv);
          break;
        case '2':
          len = sprintf (buf, &env[4], (float)adv*3.3f/4096);
          break;
        case '3':
          adv = (adv * 100) / 4096;
          len = sprintf (buf, &env[4], adv);
          break;
      }
      break;

    case 'x':
      // AD Input from 'ad.cgx'
      adv = AD_in (0);
      len = sprintf (buf, &env[1], adv);
      break;

    case 'y':
      // Button state from 'button.cgx'
      len = sprintf (buf, "<checkbox><id>button%c</id><on>%s</on></checkbox>",
                     env[1], (get_button () & (1 << (env[1]-'0'))) ? "true" : "false");
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
				
      }		
			rtc_update = true;
			break;
			
  }
  return (len);
}



