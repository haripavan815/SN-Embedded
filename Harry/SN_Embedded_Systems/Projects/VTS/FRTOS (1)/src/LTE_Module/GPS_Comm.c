/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   GPS_Comm.c
/*                                                                                                                                  */
/*  Description:   Contains Initialization of queues reading gps data from queue and LTE module Initialization .																*/
/*																																	*/
/*  -----------------------------------------------------------------------------------------------------------------------------   */
/*  |  Author              |    Date      | Rev  |									Comments								|		*/
/*  -----------------------------------------------------------------------------------------------------------------------------	*/
/*  | Harish N  		   |  16-08-2022  | 1.0  |			Added LTE_Intialization, GPS Functionalities                    |
/*------------------------------------------------------------------------------------------------------------------------------	*/
/*																																	*/
/*  Copyright (c) 2021, SNES																										*/
/*  All rights reserved.																											*/
/*																																    */
/************************************************************************************************************************************/

/* GPS Communication */
#include "LTE_Module/GPS_Comm.h"
#include "UART_Module/uart_app.h"

#define GPS_BUFFER_LENGTH 250
#define INITIALIZE_BUFFER_LENGTH 250

QueueHandle_t xQueue_Initialize_command = NULL;
QueueHandle_t xQueue_Initialize_response = NULL;

QueueHandle_t xQueue_GPS_command = NULL;
QueueHandle_t xQueue_GPS_response = NULL;

static int next_state = S_IDLE;
static int next_event = E_COMMAND;

uint8_t TEMPARORY_BUFFER[500] = {0};

enum LTE_Cmd_Resp_State get_LTE_InitState()
{
	return next_state;
}	

enum LTE_Cmd_Resp_Event get_LTE_InitEvent()
{
	return next_event;
}

static int time_out = 0;		
static TimeOut_t xTimeOut;
static TickType_t xTicksToWait = 300;

//LTE state machine to initialize LTE module
bool SM_LTE_Init()
{
	uint8_t InitializeReceiveBuffer[INITIALIZE_BUFFER_LENGTH] = {0};
	uint8_t EndofMessage[] = { 0x1A,0x1A,0x0D};
	strcpy(SNES_LTE.Is_LTE_OK, "AT\r\n");
	strcpy(SNES_LTE.Error_message_format, "AT+CMEE=2\r\n");
	strcpy(SNES_LTE.Is_SIM_Ready, "AT+CPIN?\r\n");
	strcpy(SNES_LTE.Is_Nw_Reg_Ready, "AT+CREG?\r\n");
	strcpy(SNES_LTE.Is_GPS_Nw_Reg_Ready, "AT+CGREG?\r\n");
	strcpy(SNES_LTE.Enable_Incoming_Messages, "AT+QURCCFG=\"urcport\",\"All\"\r\n");
	strcpy(SNES_LTE.Turn_On_GNSS_Engine, "AT+QGPS=1\r\n");
	strcpy(SNES_LTE.Turn_Off_GNSS_Engine, "AT+QGPSEND\r\n");
	strcpy(SNES_LTE.Configure_Gps_debug_Port, "AT+QGPSCFG=\"outport\",\"uartdebug\"\r\n");
	strcpy(SNES_LTE.Config_Gps_Nmea, "AT+QGPSCFG=\"gpsnmeatype\",3\r\n");
	strcpy(SNES_LTE.Config_GPS_AutoOutput, "AT+QGPSCFG=\"autogps\",1\r\n");
	strcpy(SNES_LTE.Save_AT_Settings_To_Nvm, "ATE&W\r\n");
	strcpy(SNES_LTE.Turn_Off_Echo, "ATE0\r\n");
	strcpy(SNES_LTE.msgautodelete_aftermaxmemorystorage, "AT\r\n");
	strcpy(SNES_LTE.Set_op_mode_cmd,"AT+CMGF=1\r\n");
	
	switch(next_state)
	{
		case S_IDLE:
		{
			next_state = S_IsLTE_OK;
		}break;
		
		case S_IsLTE_OK:
		{
				switch(next_event)
				{										
					case E_COMMAND:
					{
						if(IsSpaceAvailableInInitializeCommandQueue())
						{
							vTaskSetTimeOutState( &xTimeOut );
							xTicksToWait = 300;
							xQueueSend( xQueue_Initialize_command, SNES_LTE.Is_LTE_OK, 0);
							next_event = E_RESPONSE;
						}
						else
						{
							next_event = E_COMMAND;
						}
	
					}break;
					
					case E_RESPONSE:
					{
						memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
						if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
						{
							if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
							{
								next_state = S_ErrorMessageFormat;
								next_event = E_COMMAND;
								break;
							}
						}
						if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
						{
							next_event = E_COMMAND;
							break;
						}
						
					}break;
										
					default: break;
				}
		}break;
		
		case S_ErrorMessageFormat:
		{		
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command,SNES_LTE.Error_message_format, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_IsSimReady;
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;
		
		case S_IsSimReady:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.Is_SIM_Ready, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						if(strstr(InitializeReceiveBuffer, "READY" ) != NULL )
						{
							next_state = S_IsNwRegReady;
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}					
				}break;
				
				default: break;
			}
		}break;
				
		case S_IsNwRegReady:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.Is_Nw_Reg_Ready, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						if(strstr(InitializeReceiveBuffer, "+CREG" ) != NULL )
						{
							next_state = S_IsGPSNwRegReady;
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;
		
		case S_IsGPSNwRegReady:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.Is_GPS_Nw_Reg_Ready, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "+CGREG" ) != NULL )
						{
							next_state = S_TurnOnGNSSEngine;
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;

		case S_TurnOnGNSSEngine:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.Turn_On_GNSS_Engine, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
			
				}break;
		
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_ConfigGpsDebugPort;
							next_event = E_COMMAND;
							break;
						}
						if(strstr(InitializeReceiveBuffer, "+CME ERROR" ) != NULL ) // Session is ongoing
						{
							next_state = S_TurnOffGNSSEngine;
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
		
				default: break;
			}
		}break;

		case S_TurnOffGNSSEngine:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command,SNES_LTE.Turn_Off_GNSS_Engine, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
			
				}break;
		
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_TurnOnGNSSEngine;
							next_event = E_COMMAND;
							break;
						}
						if(strstr(InitializeReceiveBuffer, "+CME ERROR" ) != NULL ) // Session is ongoing
						{
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
		
				default: break;
			}
		}break;
		case S_ConfigGpsDebugPort:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.Configure_Gps_debug_Port, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_ConfigGPSNmea;
							next_event = E_COMMAND;
							break;
						}
						if(strstr(InitializeReceiveBuffer, "+CME ERROR" ) != NULL )
						{
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;

		case S_ConfigGPSNmea:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.Config_Gps_Nmea, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_ConfigGPSAutoOutput;
							next_event = E_COMMAND;
							break;
						}
						if(strstr(InitializeReceiveBuffer, "+CME ERROR" ) != NULL )
						{
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;

		case S_ConfigGPSAutoOutput:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.Config_GPS_AutoOutput, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_SaveATSettingsToNvm;
							next_event = E_COMMAND;
							break;
						}
						if(strstr(InitializeReceiveBuffer, "+CME ERROR" ) != NULL )
						{
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;

		case S_SaveATSettingsToNvm:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.Save_AT_Settings_To_Nvm, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_EnableIncomingMessages;
							next_event = E_COMMAND;
							break;
						}
						if(strstr(InitializeReceiveBuffer, "+CME ERROR" ) != NULL )
						{
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;
		
		case S_EnableIncomingMessages:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, "AT+QURCCFG=\"urcport\",\"All\"\r\n", 0);
						// 						xQueueSend( xQueue_Initialize_command, "\r\n", 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_MsgAutoDelete;
							next_event = E_COMMAND;
							break;
						}
						if(strstr(InitializeReceiveBuffer, "+CME ERROR" ) != NULL )
						{
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;
		
		case S_TurnOffEcho:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.Turn_Off_Echo, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_Init_SUCCESS;
							next_event = E_COMMAND;
							break;
						}
						if(strstr(InitializeReceiveBuffer, "+CME ERROR" ) != NULL )
						{
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;

		
		case S_GPSRead:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.GPS_Read, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_TurnOnTextMode;
							next_event = E_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;
						
		
		case S_MsgAutoDelete:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInInitializeCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.msgautodelete_aftermaxmemorystorage, 0);
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_Initialize_response );
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							//Delete all messages
							xQueueSend( xQueue_Initialize_command, "AT+CMGD=1,4\r\n", 0);
							vTaskDelay(100);
							next_state = S_TurnOnTextMode;
							next_event = E_COMMAND;
							break;
						}
						else
						{
							next_event = E_COMMAND;
							break;
						}
					}
					
					
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_event = E_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;
		
		case S_TurnOnTextMode:
		{
			switch(next_event)
			{
				case E_COMMAND:
				{
					if(IsSpaceAvailableInSmsCommandQueue())
					{

						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_Initialize_command, SNES_LTE.Set_op_mode_cmd, 10);
						next_state = S_TurnOnTextMode;
						next_event = E_RESPONSE;
					}
					else
					{
						next_event = E_COMMAND;
					}
					
				}break;
				
				case E_RESPONSE:
				{
					memset(InitializeReceiveBuffer,0,sizeof(InitializeReceiveBuffer));
					vTaskDelay(20);
					if( true == ReadInitializeResponce(InitializeReceiveBuffer, 0 ) )
					{
						if(strstr(InitializeReceiveBuffer, "OK" ) != NULL )
						{
							next_state = S_TurnOffEcho;
							next_event = E_SMS_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_state = S_TurnOnTextMode;
						next_event = E_SMS_COMMAND;
						
						break;
					}
				}break;
				
				default: break;
			}
		}break;
		
		case S_Init_SUCCESS:
		{
			next_state = S_Init_SUCCESS;
		}
		break;
		
		default: break;
	}
	return (next_state == S_Init_SUCCESS);
}
		
		
	
bool InitialiseGpsQueue()
{
	xQueue_GPS_command = xQueueCreate( 4, GPS_BUFFER_LENGTH );
	if( xQueue_GPS_command == 0 )
	{
		printf("Failed to create the xQueue_GPS_command\n");
	}	
	
	xQueue_GPS_response = xQueueCreate( 4, GPS_BUFFER_LENGTH );
	if( xQueue_GPS_response == 0 )
	{
		printf("Failed to create the xQueue_GPS_response\n");
	}
}

bool InitializeInitQueue()
{
	xQueue_Initialize_command = xQueueCreate( 4, GPS_BUFFER_LENGTH );
	if( xQueue_Initialize_command == 0 )
	{
		printf("Failed to create the xQueue_Initialize_command\n");
	}	
	
	xQueue_Initialize_response = xQueueCreate( 4, GPS_BUFFER_LENGTH );
	if( xQueue_Initialize_response == 0 )
	{
		printf("Failed to create the xQueue_Initialize_response\n");
	}
}

//Responses related to LTE functionalities are managed here
bool ReadInitializeResponce( uint8_t *data, uint32_t Timeout )
{
	uint8_t ReceiveBuffer[INITIALIZE_BUFFER_LENGTH] = {0};
	uint8_t rx_buffer_initialize[INITIALIZE_BUFFER_LENGTH] = {0};
	uint8_t ReceiveBufferIndex = 0;
	memset(rx_buffer_initialize, 0, sizeof(rx_buffer_initialize));
	do
	{
		if( xQueueReceive( xQueue_Initialize_response, rx_buffer_initialize, Timeout ) == pdPASS )
		{	
			memcpy( &ReceiveBuffer[ReceiveBufferIndex], rx_buffer_initialize,strlen(rx_buffer_initialize));
			ReceiveBufferIndex += strlen(rx_buffer_initialize);
			memset(rx_buffer_initialize, 0, strlen(rx_buffer_initialize));
			if( ReceiveBufferIndex >= INITIALIZE_BUFFER_LENGTH )
			{
				memset(ReceiveBuffer, 0, sizeof(ReceiveBuffer));
				ReceiveBufferIndex = 0;
			}
		}
		else
		{
			;//uart0_write_string("xQueue_Initialize_response Buffer Full\n", strlen("xQueue_Initialize_response Buffer Full\n"));
		}
		
	}while( (uxQueueMessagesWaiting(xQueue_Initialize_response) != 0) );
	
	if( ReceiveBufferIndex != 0 )
	{
		memcpy( data , ReceiveBuffer, ReceiveBufferIndex );
		return( true );
	}
	else
	{
		return( false );
	}		
}

//Responses related to GPS functionalities are managed here
bool ReadGPSResponce( uint8_t *data, uint32_t Timeout )
{
	uint8_t ReceiveBuffer[GPS_BUFFER_LENGTH] = {0};
	uint8_t rx_buffer_gps[GPS_BUFFER_LENGTH] = {0};
	uint8_t ReceiveBufferIndex = 0;
	
	memset(ReceiveBuffer, 0, sizeof(ReceiveBuffer));
	memset(rx_buffer_gps, 0, sizeof(rx_buffer_gps));
	do
	{
		if( xQueueReceive( xQueue_GPS_response, rx_buffer_gps, Timeout ) == pdPASS )
		{
			memcpy( &ReceiveBuffer[ReceiveBufferIndex], rx_buffer_gps,strlen(rx_buffer_gps));
			ReceiveBufferIndex += strlen(rx_buffer_gps);
			memset(rx_buffer_gps, 0, strlen(rx_buffer_gps));
			if( ReceiveBufferIndex >= GPS_BUFFER_LENGTH )
			{
				memset(ReceiveBuffer, 0, sizeof(ReceiveBuffer));
				ReceiveBufferIndex = 0;
			}
		}
		else
		{
			;//uart0_write_string("xQueue_GPS_response Buffer Full\n", strlen("xQueue_GPS_response Buffer Full\n"));
		}
		
	}while( (uxQueueMessagesWaiting(xQueue_GPS_response) != 0) );
	
	if( ReceiveBufferIndex != 0 )
	{
		memcpy( data , ReceiveBuffer, ReceiveBufferIndex );
		return( true );
	}
	else
	{
		return( false );
	}
		
}

bool IsSpaceAvailableInGpsCommandQueue()
{
	if( uxQueueMessagesWaiting(xQueue_GPS_command) == 4 )
		return false;
		
	return true;	
}

bool IsSpaceAvailableInInitializeCommandQueue()
{
	if( uxQueueMessagesWaiting(xQueue_Initialize_command) == 4 )
		return false;
		
	return true;	
}

int parse_comma_delimited_GPS_str(char *string, char **fields, int max_fields)
{
	int i = 0;
	fields[i++] = string;
	while ((i < max_fields) && NULL != (string = strchr(string, ','))) {
		*string = '\0';
		fields[i++] = ++string;
	}
	return --i;
}

int Gpsnmea0183_checksum(char *nmea_data)
{
	int crc = 0;
	int i;

	// the first $ sign and the last two bytes of original CRC + the * sign
	for (i = 1; i < strlen(nmea_data) - 3; i ++) {
		crc ^= nmea_data[i];
	}

	return crc;
}

//Fetches the GPS location and parses it using CRC method
bool GetGPSLocation( uint8_t** gpsLoc )
{
	bool ret = false;
	
	uint8_t rx_buffer_gps[GPS_BUFFER_LENGTH] = {0};
	do
	{
		memset(rx_buffer_gps,0,sizeof(rx_buffer_gps));
		if( xQueueReceive( xQueue_GPS_response, rx_buffer_gps, 0 ) == pdPASS )						
		{
			if(strstr(rx_buffer_gps, "$GPRMC" ) != NULL )
			{
				//printf("Ig: %s\n", rx_buffer_gps);
					
				char str[GPS_BUFFER_LENGTH] = {0};
				memcpy(&str, rx_buffer_gps, GPS_BUFFER_LENGTH);
					
				parse_comma_delimited_GPS_str(&rx_buffer_gps, gpsLoc, 20);
			
				//parse * separation
				static char token_list[3][12]; // Output token list after parsing
				char* token = strtok(gpsLoc[12], "*");
				int num_tokens = 0; // Index to token list. We will append to the list
				while (token != NULL) {
					// Keep getting tokens until we receive NULL from strtok()
					strcpy(token_list[num_tokens], token); // Copy to token list
					num_tokens++;
					token = strtok(NULL, "*"); // Get the next token. Notice that input=NULL now!
				}
			
				int Nmeachecksum = 0;
				sscanf(token_list[num_tokens-1], "%x", &Nmeachecksum);
				if(Nmeachecksum == Gpsnmea0183_checksum(&str))
				{
					//printf("UTC Time  :%s\r\n",gpsLoc[1]);
					//printf("Latitude  :%s\r\n",gpsLoc[2]);
					//printf("Longitude :%s\r\n",gpsLoc[4]);
					//printf("Satellites:%s\r\n",gpsLoc[7]);
					//printf("GPS data valid\r\n");
					ret = true;
				}	
			
				//if calculated checksum and NMEA checksum are same, then data is valid
			}
			// Similarly for GPGGA if required
		}
	}while( (uxQueueMessagesWaiting(xQueue_GPS_response) != 0) );
	return ret;
}