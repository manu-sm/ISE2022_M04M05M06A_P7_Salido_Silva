
#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "Driver_I2C.h"
#include "lpc17xx.h"
#include "GPIO_LPC17xx.h"
#include "PIN_LPC17xx.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 

#define LPC1768_SLAVE       	0x28      
      
#define PUERTO_RGB            2
#define RGB_ROJO              3
#define RGB_VERDE             2
#define RGB_AZUL              1

#define PUERTO_INT            0
#define INT_EXTERNA_SLAVE     5
 


#define SIG_TEMP							0x0001

#define TIME_ONESHOT_1s       1000
void callback_Timer(void const *arg);
osTimerDef(timer_1s,callback_Timer);
osTimerId id_timer1s;

int32_t status;
uint32_t addr = LPC1768_SLAVE;
uint8_t data_tx, data_rx, buf[10];



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
	
  status = I2Cdrv->Initialize (I2C_SignalEvent);
  status = I2Cdrv->PowerControl (ARM_POWER_FULL);
  status = I2Cdrv->Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  status = I2Cdrv->Control      (ARM_I2C_BUS_CLEAR, 0);
	
}

void Init(void){
	GPIO_SetDir(PUERTO_RGB,RGB_ROJO,GPIO_DIR_OUTPUT);
	GPIO_SetDir(PUERTO_RGB,RGB_VERDE,GPIO_DIR_OUTPUT);
  GPIO_SetDir(PUERTO_RGB,RGB_AZUL,GPIO_DIR_OUTPUT);
	GPIO_PinWrite(PUERTO_RGB,RGB_AZUL,1);
	GPIO_PinWrite(PUERTO_RGB,RGB_ROJO,1);
  GPIO_PinWrite(PUERTO_RGB,RGB_VERDE,1);
  
  LPC_GPIOINT->IO0IntEnR |= (1 << 5);
  NVIC_EnableIRQ(EINT3_IRQn);
}

int Init_Thread (void) {

  tid_Thread = osThreadCreate (osThread(Thread), NULL);
  if (!tid_Thread) return(-1);
  
  return(0);
}

void Thread (void const *argument) {
  
  data_tx = 0;
  Init();
  id_timer1s = osTimerCreate(osTimer(timer_1s), osTimerOnce, (void *)0);

  while (1) {
    osDelay(3000);
		
		status = I2Cdrv->MasterTransmit(addr, &data_tx, 1, true);
		osSignalWait (SIG_TEMP, osWaitForever); 
		
	
		
		status = I2Cdrv->MasterReceive(addr, buf, 1, true);
		osSignalWait (SIG_TEMP, osWaitForever); 
		
		data_rx = ~buf[0];
    
    if(data_rx == data_tx){
      GPIO_PinWrite(PUERTO_RGB,RGB_VERDE,0);
      osTimerStart(id_timer1s,TIME_ONESHOT_1s);
    }
    else{
      GPIO_PinWrite(PUERTO_RGB,RGB_ROJO,0);  
      osTimerStart(id_timer1s,TIME_ONESHOT_1s);
    }

		data_tx++;
		
				
    osThreadYield ();                                           // suspend thread
  
  }
}

void callback_Timer (void const *arg){
  GPIO_PinWrite(PUERTO_RGB,RGB_VERDE,1);
  GPIO_PinWrite(PUERTO_RGB,RGB_ROJO,1);
  GPIO_PinWrite(PUERTO_RGB,RGB_AZUL,1);

}

void EINT3_IRQHandler (void){
  GPIO_PinWrite(PUERTO_RGB,RGB_AZUL,0);
  LPC_GPIOINT->IO0IntClr |= (1 << 5);
  osTimerStart(id_timer1s,TIME_ONESHOT_1s);
}
