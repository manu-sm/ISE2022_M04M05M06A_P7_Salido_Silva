#include "cmsis_os.h"                                           // CMSIS RTOS header file

#define signal_pwd_pulse						0x04		// 100
#define pwd_up_signal_event  				0x08
#define pwd_down_signal_event 			0x10
#define pwd_left_signal_event 			0x20
#define pwd_right_signal_event 			0x40
#define pwd_center_signal_event  		0x80

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototipos
int Init_rebotes_joystick (void);

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Threads
void rebotes_joystick (void const *argument);                             // thread function



