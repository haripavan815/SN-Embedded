
/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   frtos_app.c																								                */
/*																																	*/
/*  Description:  Contains LTE, Mqtt and checking Health status of board Functionalities .																                                        */
/*																																	*/
/*  -----------------------------------------------------------------------------------------------------------------------------   */
/*  |  Author              |    Date      | Rev  |									Comments								|		*/
/*  -----------------------------------------------------------------------------------------------------------------------------	*/
/*  | Naveen G S		   |  01-12-2021  | 1.0  |			Created													        |	    */
/*  | Harish N  		   |  16-12-2021  | 1.0  |			Added Checking board health status, LTE,Mqtt,SMS initialization modules	*/
/*------------------------------------------------------------------------------------------------------------------------------	*/
/*																																	*/
/*  Copyright (c) 2021, SNES																										*/
/*  All rights reserved.																											*/
/*																																    */
/************************************************************************************************************************************/


#include <asf.h>
#include <string.h>
#include "Free-RTOS_Module/frtos_app.h"
#include "UART_Module/uart_app.h"
#include "USART_Module/usart_app.h"
#include "LTE_Module/LTE_Comm.h"
#include "LTE_Module/GPS_Comm.h"
#include "LTE_Module/MQTT_Comm.h"
#include "LTE_Module/SMS_Comm.h"
#include "Services_Module/Services.h"
#include "samv71.h"
#include <string.h>
#include "CmdRes.h"
#include "conf_board.h"
#include "function.h"
#include "Memory_Module/VTS_Flash.h"
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern QueueHandle_t xQueue_GPS_response;
/**
 * \brief Called if stack overflow during execution
 */

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName)
{
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	 * identify which task has overflowed its stack.
	 */
	for (;;) 
	{
		
	}
}

 //brief This function is called by FreeRTOS idle task
extern void vApplicationIdleHook(void)
{
	
}

  //brief This function is called by FreeRTOS each tick

extern void vApplicationTickHook(void)
{
}

extern void vApplicationMallocFailedHook(void)
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}

extern SemaphoreHandle_t xSemaphoreLTE;// Use this if Any shared resource between tasks
/*
const char* LTE_Cmd_Resp_State_Names[] = {	"S_IDLE", "S_IsLTE_OK",
	"S_ErrorMessageFormat",
	"S_IsSimReady", "S_IsNwRegReady", "S_IsGPSNwRegReady", "S_Init_SUCCESS", "S_ERROR",
	"S_TurnOffGNSSEngine", "S_TurnOnGNSSEngine", "S_ConfigGpsDebugPort", "S_ConfigGPSNmea", "S_ConfigGPSAutoOutput",
	"S_SaveATSettingsToNvm",
	"S_GPSRead", "S_SMS_Recieve_SUCCESS"
};
	
const char* LTE_Cmd_Resp_Event_Names[] =  {	"E_COMMAND",
	"E_RESPONSE"
};
*/

enum LTE_Cmd_Resp_State state_init;
enum LTE_Cmd_Resp_Event event_init;

enum MQTT_Cmd_Resp_State state_mqtt;
enum MQTT_Cmd_Resp_Event event_mqtt;

enum SMS_Cmd_Resp_State state_sms;
enum SMS_Cmd_Resp_Event event_sms;

bool CheckFactorySettings = false;
// Perform an action every 50 ticks.


//This task monitors the health of board and application
void task_Health_monitor( void * pvParameters )
 {
	 TickType_t xLastWakeTime;
	 const TickType_t xFrequency = 50000;

	uint8_t rx_buffer_gps[TRANSFER_SIZE];
	memset(rx_buffer_gps, 0, TRANSFER_SIZE);
	
	xLastWakeTime = xTaskGetTickCount ();
	
	for( ;; )
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
			//Check board health status before moving on (i.e UART, USART, SPI, QSPI, I2C, IMU, BATTERY_VOLTAGE);
	}
 } 
 
 // LTE, MQTT and GPS are initialized and monitored in the task 
 void task_Main_App( void * pvParameters )
 {
	 TickType_t xLastWakeTime;
	 const TickType_t xFrequency = 50;

	uint8_t rx_buffer_gps[TRANSFER_SIZE];
	memset(rx_buffer_gps, 0, TRANSFER_SIZE);
	
	xLastWakeTime = xTaskGetTickCount ();
	
	for( ;; )
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
        //if( xSemaphoreTake( xSemaphoreLTE, ( TickType_t ) 0 ) )
        {
			if((true == IsBoardHealthSuccess) && (g_ui_device_mode == DEVICE_IN_NORMAL_MODE))
			{
				// LTE Initialization StateMachine
				SM_LTE_Init();
				
				if(true == SMSReceiveHandler())
				{
					printf("SMS is Valid and Authenticated Successfully!!\n");
				// For each SM, get State and Event
				// For each SM, set next State or Event based on previous events response or Asynchronous response or failures						
				state_init = get_LTE_InitState();
				event_init = get_LTE_InitEvent();
				state_mqtt = get_MQTT_State();
				state_sms = get_SMS_State();
				//printf("S: %s, E:%s\r\n", LTE_Cmd_Resp_State_Names[state_init], LTE_Cmd_Resp_Event_Names[event_init]);	
				//Initialization is successful and GPS debugging is enabled
				if (state_init == S_Init_SUCCESS)
				{
					//Initialize USART1 (if not initialized), to receive GPS data
					if(!IsGpsUartInitialized)
					{
						configure_usart(ID_USART1, 115200);
						IsGpsUartInitialized = true;
					}
					// Get GPS data every second
					static TimeOut_t xTimeOutGps;
					static TickType_t xTicksToWaitGps = 0;
					if( xTaskCheckForTimeOut( &xTimeOutGps, &xTicksToWaitGps ) != pdFALSE )
					{
						vTaskSetTimeOutState( &xTimeOutGps );
						xTicksToWaitGps = 1000;
						// Define structure for GPS data and update the fields properly inside GetGPSLocation()
						// Based on the required fields parse the required GPS Protocol just like the $RMC example in GetGPSLocation()
						// ToDo: Verify the outdoor validity of GPS data and parsing technique or do we need any other lightweight parser?
						char* gpsLoc[250] = {0};
						if(GetGPSLocation(&gpsLoc))
						{
							printf("GPS checksum valid\r\n");
						}
							else
						{
							printf("GPS checksum invalid or no data\r\n");
						}	
					}
										 
						// MQTT State Machine
						// Select the corresponding topic 
						if(state_mqtt != S_MQTT_SUCCESS)
						{
							SM_MQTT_Init();
							// Better to move publish out of MQTT SM for timely publishing messages
						}
							
						// Publish MQTT data every 20 sec
						static TimeOut_t xTimeOutMqttPub;
						static TickType_t xTicksToWaitMqttPub = 0;
						if( xTaskCheckForTimeOut(&xTimeOutMqttPub, &xTicksToWaitMqttPub) != pdFALSE )
						{
							vTaskSetTimeOutState( &xTimeOutMqttPub );
							xTicksToWaitMqttPub = 20000;
							if(IsSpaceAvailableInMqttCommandQueue())
							{
								//Publish_To_Mqtt_Server(Idx_value,1,1,0,SNES_Mqtt.sub_topic,"\"27.2046N,77.4977E\"");
							}
						}
						// Handle Subscribed command response and MQTT disconnection and Close scenarios here
							MQTTReceiveHandler();
							//Registration state machine to be added to get user details from server
							//SM_Registration();
							//Send SMS once after startup
							/*if(state_sms != S_SMS_Send_SUCCESS)
							{
								SM_SMS_Send();				//SMS state machine to send SMS to +917676890629 
							}
							*/
							//Send SMS once after startup
							
					// Conditional execution after GPRS/MQTT Failure, timeout and retry attempts.
					// Decide when to reset LTE module? (If 'any failure' or 'no GPS data' or 'no command response' or what?) 
					//--> Reset all statemachines in step back manner
				}				
				//xSemaphoreGive( xSemaphoreLTE );
			}
			}
        }
	}
 }


 
 

