/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions


extern void Init_i2c(void);
extern int Init_Thread (void);

/*
 * main: initialize and start the system
 */
int main (void) {
	
	
	Init_i2c();
	
  osKernelInitialize ();                    // initialize CMSIS-RTOS

  // initialize peripherals here

  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	
	Init_Thread();

  osKernelStart ();                         // start thread execution 
}
