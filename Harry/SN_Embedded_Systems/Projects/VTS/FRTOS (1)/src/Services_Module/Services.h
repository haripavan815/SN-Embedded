
#ifndef SERVICES_H_
#define SERVICES_H_
#include <asf.h>
#include <conf_board.h>
#include "Free-RTOS_Module/frtos_app.h"
#include "UART_Module/uart_app.h"
#include "USART_Module/usart_app.h"
#include "LTE_Module/LTE_Comm.h"
#include "LTE_Module/GPS_Comm.h"
#include "LTE_Module/MQTT_Comm.h"
#include "LTE_Module/SMS_Comm.h"
#include "IMU_Module/VTS_IMU.h"
//#define task_LTE_Comm_STACK_SIZE            ((2*4096)/sizeof(portSTACK_TYPE))
//#define task_LTE_Comm_PRIORITY				(tskIDLE_PRIORITY+1)

void task_Services( void * pvParameters );
#endif SERVICES_H_