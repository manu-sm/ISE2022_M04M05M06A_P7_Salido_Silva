#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "LPC17xx.h"
#include "Rebotes_joystick.h"
#include "rtc.h"

/*----------------------------------------------------------------------------
 *      Thread encargada de controlar los rebotes de los pulsadores
 *---------------------------------------------------------------------------*/
 
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Threads
osThreadId tid_rebotes_joystick;                                          // thread id
osThreadDef (rebotes_joystick, osPriorityNormal, 1, 0);                   // thread object

extern osThreadId tid_RTC; 
 
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// LEDS
#define PUERTO_LED		1
#define	LED_1					18
#define	LED_2					20
#define	LED_3					21
#define	LED_4					23

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// JOSTICK
#define PUERTO_INT	0
#define LINEA_INT_PIN_12	17			// pin 12 ==> sw_down
#define LINEA_INT_PIN_13	15			// pin 13 ==> sw_left
#define LINEA_INT_PIN_15	23			// pin 15 ==>	sw_up
#define LINEA_INT_PIN_16	24			// pin 16 ==> sw_right
#define LINEA_INT_PIN_14	16			// pin 15 ==> sw_center

// Definición del timer para controlar el rebote del pulsador
#define time_oneshot_timer 20
void callback_Timer_pwd (void const *arg);																// prototype for timer callback function
osTimerDef (one_shot_pwd, callback_Timer_pwd);														// define timer
osTimerId id_pwd_timer;	

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Señales
osEvent pwd_pulse_event;
extern int32_t signals;


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Variables globales
uint32_t contador_pwd_up = 0;
uint32_t contador_pwd_down = 0;
uint32_t contador_pwd_left = 0;
uint32_t contador_pwd_right = 0;
uint32_t contador_pwd_center = 0;

extern osThreadId tid_programa_principal;

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Init
/**
* Inicializa el hilo rebotes_joystick
*/
int Init_rebotes_joystick (void) {

  tid_rebotes_joystick = osThreadCreate (osThread(rebotes_joystick), NULL);
  if (!tid_rebotes_joystick){ 
		return(-1);
	}
  
  return(0);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Principal thread
/**
*	Hilo que se encarga de controlar los rebotes del pulsador y enviar una señal al programa principal indicando que pulsador se ha pulsado.
* 	Este hilo esta bloqueado esperando que se produzca un evento enviado por el main que indica que se ha pulsado un pulsador.
*		Una vez se produce un evento el hilo inicia un timer de 20 ms, (tiempo suficiente para que el estado del pulsador quede "estable").
*		La rutina de atencion a la interrupcion de dicho timer se encargara de evaluar que boton se ha pulsado y de enviar la señal.
*/
void rebotes_joystick (void const *argument) {
	
	// Creamos un timer one_shot para el rebote del pulsador.
	id_pwd_timer = osTimerCreate(osTimer(one_shot_pwd), osTimerOnce, (void *)0);
	
  while (1) {
    pwd_pulse_event = osSignalWait (signal_pwd_pulse, osWaitForever);
		if (pwd_pulse_event.status == osEventSignal){
			osTimerStart(id_pwd_timer, time_oneshot_timer);
		}
    osThreadYield ();                                           // suspend thread
  }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// IRQ timer
/**
*	Esta rutina de atencion a la interrupcion del timer se encarga de evaluar que boton del pulsador se ha pulsado y de enviar la señal correspondiente al programa principal. 
*	El algortimo es sencillo, pasado el timer de 20 ms se leen todas las entradas y la que se encuentre a nivel alto es la que se ha pulsado.
*	El algoritmo dispone de una serie de contadores para verificar que los rebotes se corrijen adecuadamente.
*/
void callback_Timer_pwd (void const *arg){
	
		// Control rebote pulsador derecho
		if (GPIO_PinRead(PUERTO_INT, LINEA_INT_PIN_16)){
				contador_pwd_right++;
				signals = osSignalSet (tid_RTC, pwd_right_signal_event);
		}	
		// Control rebote pulsador izquierdo
		if (GPIO_PinRead(PUERTO_INT, LINEA_INT_PIN_13)){
				contador_pwd_left++;
				signals = osSignalSet (tid_RTC, pwd_left_signal_event);
		}	
		// Control rebote pulsador arriba
		if (GPIO_PinRead(PUERTO_INT, LINEA_INT_PIN_15)){
				contador_pwd_up++;
				signals = osSignalSet (tid_RTC, pwd_up_signal_event);
		}	
		// Control rebote pulsador abajo
    if (GPIO_PinRead(PUERTO_INT, LINEA_INT_PIN_12)){
				contador_pwd_down++;
				signals = osSignalSet (tid_RTC, pwd_down_signal_event);
		}
		// Control rebote pulsador central
    if (GPIO_PinRead(PUERTO_INT, LINEA_INT_PIN_14)){
				contador_pwd_center++;
				signals = osSignalSet (tid_RTC, pwd_center_signal_event);
		}
}
