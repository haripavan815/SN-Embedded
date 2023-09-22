/*
 * frtos_app.h
 *
 * Created: 12/16/2021 9:44:18 AM
 *  Author: 
 */ 
#include "function.h"

#ifndef frtos_APP_H_
#define frtos_APP_H_

#define task_Main_App_STACK_SIZE            ((2*4096)/sizeof(portSTACK_TYPE))
#define task_Main_App_PRIORITY				(tskIDLE_PRIORITY+1)
#define task_LTE_Comm_STACK_SIZE            ((2*4096)/sizeof(portSTACK_TYPE))
#define task_LTE_Comm_PRIORITY				(tskIDLE_PRIORITY+1)
#define task_health_monitor_STACK_SIZE		((2*4096)/sizeof(portSTACK_TYPE))
#define task_health_monitor_PRIORITY		(tskIDLE_PRIORITY+1)	

void task_Main_App( void * pvParameters );
void task_Health_monitor( void * pvParameters);
#endif /* frtos_APP_H_ */