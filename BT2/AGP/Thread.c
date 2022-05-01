
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 

#define LM75B_I2C_ADDR       	0x28      
 
 
#define REG_TEMP    					0x00
#define REG_CONF	    				0x01
#define	REG_THYST   					0x02
#define REG_TOS     					0x03

#define SIG_TEMP							0x0001

#define PUERTO_INT					2
#define PIN_INTERRUP_I2C		0		// dip26
#define PUERTO_INT_LED			1
#define PIN_LED_4						23


uint32_t addr = LM75B_I2C_ADDR;
int32_t status = 0, nlect = 0;
uint8_t cmd, buf[10];
uint16_t temp;
float temperatura;


/* I2C driver instance */
extern ARM_DRIVER_I2C            Driver_I2C2;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C2;


static volatile uint32_t I2C_Event;


void Thread (void const *argument);                             // thread function
osThreadId tid_Thread;                                          // thread id
osThreadDef (Thread, osPriorityNormal, 1, 0);                   // thread object
 
/* I2C Signal Event function callback */
void I2C_SignalEvent (uint32_t event) {
 
  /* Save received events */
  I2C_Event |= event;
 
  /* Optionally, user can define specific actions for an event */
 
  if (event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) {
    /* Less data was transferred than requested */
  }
 
  if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
    /* Transfer or receive is finished */
		
		osSignalSet (tid_Thread, SIG_TEMP);

  }
 
  if (event & ARM_I2C_EVENT_ADDRESS_NACK) {
    /* Slave address was not acknowledged */
  }
 
  if (event & ARM_I2C_EVENT_ARBITRATION_LOST) {
    /* Master lost bus arbitration */
  }
 
  if (event & ARM_I2C_EVENT_BUS_ERROR) {
    /* Invalid start/stop position detected */
  }
 
  if (event & ARM_I2C_EVENT_BUS_CLEAR) {
    /* Bus clear operation completed */
  }
 
  if (event & ARM_I2C_EVENT_GENERAL_CALL) {
    /* Slave was addressed with a general call address */
  }
 
  if (event & ARM_I2C_EVENT_SLAVE_RECEIVE) {
    /* Slave addressed as receiver but SlaveReceive operation is not started */
  }
 
  if (event & ARM_I2C_EVENT_SLAVE_TRANSMIT) {
    /* Slave addressed as transmitter but SlaveTransmit operation is not started */
  }
}

void Init_i2c(void){
	
	int32_t status;
	
  status = I2Cdrv->Initialize 	(I2C_SignalEvent);
  status = I2Cdrv->PowerControl (ARM_POWER_FULL);
  status = I2Cdrv->Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  status = I2Cdrv->Control      (ARM_I2C_BUS_CLEAR, 0);
	
	
}

int Init_Thread (void) {

  tid_Thread = osThreadCreate (osThread(Thread), NULL);
  if (!tid_Thread) return(-1);
  
  return(0);
}

void Thread (void const *argument) {
	
	uint8_t cnt = 0;
 
  /* Initialize I2C peripheral */
  I2Cdrv->Initialize(I2C_SignalEvent);
  /* Power-on I2C peripheral */
  I2Cdrv->PowerControl(ARM_POWER_FULL);
  /* Configure I2C bus */
  I2Cdrv->Control(ARM_I2C_OWN_ADDRESS, 0x28);
 
  I2C_Event = 0;
	
		// Configuración de las salidas
	GPIO_SetDir (PUERTO_INT, PIN_INTERRUP_I2C, GPIO_DIR_OUTPUT);
	GPIO_SetDir (PUERTO_INT_LED, PIN_LED_4, GPIO_DIR_OUTPUT);
	
  while (1) {
		
    /* Receive chunk */
    I2Cdrv->SlaveReceive(buf, 1);
		osSignalWait (SIG_TEMP, osWaitForever); 
		
		temp = buf[0];
		
		// Evaluamos si es necesario ejecutar la interrupción
		if (buf[0] & 0x04) {
			GPIO_PinWrite (PUERTO_INT,PIN_INTERRUP_I2C,1);
			GPIO_PinWrite (PUERTO_INT_LED,PIN_LED_4,1);
			osDelay (250);
			GPIO_PinWrite (PUERTO_INT,PIN_INTERRUP_I2C,0);
			GPIO_PinWrite (PUERTO_INT_LED,PIN_LED_4,0);
		}
		else {
			GPIO_PinWrite (PUERTO_INT,PIN_INTERRUP_I2C,0);
		}
		
		buf[0] = ~buf[0];
    /* Transmit chunk back */
    I2Cdrv->SlaveTransmit(buf,1);
		osSignalWait (SIG_TEMP, osWaitForever);
		
		osThreadYield ();                                           // suspend thread
  }


}
