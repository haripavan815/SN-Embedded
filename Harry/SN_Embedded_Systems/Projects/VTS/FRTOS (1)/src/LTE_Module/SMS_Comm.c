/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   SMS_Comm.c                                                                                                         */
/*                                                                                                                                  */
/*  Description:   Contains initialization of sms queue, sending sms command and reading sms response functionalities.
/*															                                                                        */
/*																																	*/
/*  -----------------------------------------------------------------------------------------------------------------------------   */
/*  |  Author              |    Date      | Rev  |									Comments								|		*/
/*  -----------------------------------------------------------------------------------------------------------------------------	*/
/*  | Harish N  		   |  27-06-2022  | 1.0  |			Added SMS module Functionalities.		                        |		*/
/*------------------------------------------------------------------------------------------------------------------------------	*/
/*																																	*/
/*  Copyright (c) 2021, SNES																										*/
/*  All rights reserved.																											*/
/*																																    */
/************************************************************************************************************************************/

#include "LTE_Module/SMS_Comm.h"
#include "UART_Module/uart_app.h"
#include "asf.h"
#include <string.h>
#define SMS_BUFFER_LENGTH 500
#define Max_sms_memorysize 9
QueueHandle_t xQueue_SMS_command = NULL;
QueueHandle_t xQueue_SMS_response = NULL;
QueueHandle_t xQueue_SMS_Async_response = NULL;
uint8_t msgID;
bool IsValidationSuccess = false;
bool IsDeleteInProgress = false;
bool Rx_MSG_flag, SMS_Validation_flag = false;
extern bool IsSMSValid = false;
extern bool IsPhonenumberValid = false;
extern int msgindex=0;
#define MobileNum_Start_Index 22
#define MobileNum_Last_Index 34
#define MsgID_Start_Index 12
#define MsgID_Last_Index 13
bool DeleteMsgsInInbox();
bool Validate_Recieved_SMS();




static int next_sms_state =S_IDLE;
static int next_sms_event = E_COMMAND;

enum SMS_Cmd_Resp_State state_sms;
enum SMS_Cmd_Resp_Event event_sms;

bool SMS_Data;

char* substr(const char *src, int m, int n)
{
	// get the length of the destination string
	int len = n - m;
	
	// allocate (len + 1) chars for destination (+1 for extra null character)
	char *dest = (char*)malloc(sizeof(char) * (len + 1));
	
	// extracts characters between m'th and n'th index from source string
	// and copy them into the destination string
	for (int i = m; i < n && (*(src + i) != '\0'); i++)
	{
		*dest = *(src + i);
		dest++;
	}
	
	// null-terminate the destination string
	*dest = '\0';
	
	// return the destination string
	return dest - len;
}


enum SMS_Cmd_Resp_State get_SMS_State()
{
	return next_sms_state;
}

enum SMS_Cmd_Resp_Event get_SMS_Event()
{
	return next_sms_event;
}

static int time_out = 0;
static TimeOut_t xTimeOut;
static TickType_t xTicksToWait = 0;

bool InitialiseSmsQueue()
{
	xQueue_SMS_command = xQueueCreate( 4, SMS_BUFFER_LENGTH );
	if( xQueue_SMS_command == 1 )
	{
		printf("Failed to create the xQueue_SMS_command\n");
		return false;
	}
	
	xQueue_SMS_response = xQueueCreate( 4, SMS_BUFFER_LENGTH );
	if( xQueue_SMS_response == 1 )
	{
		printf("Failed to create the xQueue_SMS_response\n");
		return false;
	}
	
	xQueue_SMS_Async_response = xQueueCreate( 4, SMS_BUFFER_LENGTH );
	if( xQueue_SMS_Async_response == 0 )
	{
		printf("Failed to create the xQueue_SMS_Async_response\n");
	}
	
		
		 
		strcpy(SNES_SMS.Set_op_mode_cmd,"AT+CMGF=1\r\n");
		strcpy(SNES_SMS.Set_mobile_num_cmd,"AT+CMGS=\"+917676890629\"\r\n");
		strcpy(SNES_SMS.SMS_text_cmd, "Sample SMS\r\n");
		strcpy(SNES_SMS.Read_sms_cmd, "AT+CMGR=\r\n");
		strcpy(SNES_SMS.Receive_sms_cmd, "AT+CMTI=3\r\n");
		
		return true;
}

bool IsSpaceAvailableInSmsCommandQueue()
{
	if(uxQueueMessagesWaiting(xQueue_SMS_command) == 4 )
	return false;
	else
	return true;
}

//Reads SMS responses from the buffer
bool ReadSMSResponce( uint8_t *data, uint32_t Timeout )
{
	uint8_t ReceiveBuffer[SMS_BUFFER_LENGTH] = {0};
	uint8_t rx_buffer_SMS[SMS_BUFFER_LENGTH] = {0};
	uint8_t ReceiveBufferIndex = 0;
	memset(rx_buffer_SMS, 0, sizeof(rx_buffer_SMS));
	do
	{
		if( xQueueReceive( xQueue_SMS_response, rx_buffer_SMS, Timeout ) == pdPASS )
		{
			memcpy( &ReceiveBuffer[ReceiveBufferIndex], rx_buffer_SMS,strlen(rx_buffer_SMS));
			ReceiveBufferIndex += strlen(rx_buffer_SMS);
			memset(rx_buffer_SMS, 0, strlen(rx_buffer_SMS));
			if( ReceiveBufferIndex >= SMS_BUFFER_LENGTH )
			{
				memset(ReceiveBuffer, 0, sizeof(ReceiveBuffer));
				ReceiveBufferIndex = 0;
			}
		}
		else
		{
			;
		}
		
	}while( (uxQueueMessagesWaiting(xQueue_SMS_response) != 0) );
	
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

//Sends SMS to the provided mobile number
bool SM_SMS_Send()
{
	uint8_t SMSReceiveBuffer[SMS_BUFFER_LENGTH] = {0};
	uint8_t EndofMessage[] = { 0x1A,0x1A,0x0D};
		SMS_Data = ReadFactoryConfigDataFromFlash();
		switch(next_sms_state)
		{
			case S_SMS_IDLE:
			{
				next_sms_state = S_TurnOnTextMode;
			}break;
			
			case S_TurnOnTextMode:
			{
				switch(next_sms_event)
				{
					case E_SMS_COMMAND:
					{
						if(IsSpaceAvailableInSmsCommandQueue())
						{
							vTaskSetTimeOutState( &xTimeOut );
							xTicksToWait = 300;
							xQueueSend( xQueue_SMS_command, SNES_SMS.Set_op_mode_cmd, 0);
							next_sms_event = E_SMS_RESPONSE;
						}
						else
						{
							next_sms_event = E_SMS_COMMAND;
						}
					}break;
					
					case E_SMS_RESPONSE:
					{
						memset(SMSReceiveBuffer,0,sizeof(SMSReceiveBuffer));
						if( true == ReadSMSResponce(SMSReceiveBuffer, 0 ) )
						{
							if(strstr(SMSReceiveBuffer, "OK" ) != NULL )
							{
								next_sms_state = S_EnterNumber;
								next_sms_event = E_SMS_COMMAND;
								break;
							}
						}
						if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
						{
							next_sms_event = E_SMS_COMMAND;
							break;
						}
					}break;
					
					default: break;
				}
			}break;
			
			case S_EnterNumber:
			{
				switch(next_sms_event)
				{
					case E_SMS_COMMAND:
					{
						if(IsSpaceAvailableInSmsCommandQueue())
						{
							vTaskSetTimeOutState( &xTimeOut );
							xTicksToWait = 300;
							strcpy(SNES_SMS.Set_mobile_num_cmd,"AT+CMGS=\"+919663947730\"\r\n");
							SMS_Data = ReadFactoryConfigDataFromFlash();
							xQueueSend( xQueue_SMS_command, SNES_SMS.Set_mobile_num_cmd, 0);
							next_sms_event = E_SMS_RESPONSE;
						}
						else
						{
							next_sms_event = E_SMS_COMMAND;
						}
						
					}break;
					
					case E_SMS_RESPONSE:
					{
						memset(SMSReceiveBuffer,0,sizeof(SMSReceiveBuffer));
						if( true == ReadSMSResponce(SMSReceiveBuffer, 0 ) )
						{
							xQueueReset( xQueue_SMS_response );
							if(strstr(SMSReceiveBuffer, ">" ) != NULL )
							{
								next_sms_event = E_SMS_MESSAGE;
								break;
							}
						}
						if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
						{
							next_sms_event = E_SMS_COMMAND;
							break;
						}
					}break;	
									
					case E_SMS_MESSAGE:
					{
						if(IsSpaceAvailableInSmsCommandQueue())
						{
							vTaskSetTimeOutState( &xTimeOut );
							xQueueReset( xQueue_SMS_command);
							xTicksToWait = 300;
							xQueueSend( xQueue_SMS_command, SNES_SMS.SMS_text_cmd, 0);  
							next_sms_state = S_SMS_Send_SUCCESS;
							next_sms_event = E_SMS_RESPONSE;
						}
						else 
						{							
							next_sms_event = E_SMS_COMMAND;
						}						
					}break;	
					default: break;
				}
			}break;
			
			default: break;
		}
		return (next_sms_state == S_SMS_Send_SUCCESS);
}

//Reads all the SMS messages from the module
bool ReadSMSAsyncResponce( uint8_t *data, uint32_t Timeout )
{
	uint8_t ReceiveBuffer[SMS_BUFFER_LENGTH] = {0};
	uint8_t rx_buffer_SMS_Async[SMS_BUFFER_LENGTH] = {0};
	uint8_t ReceiveBufferIndex = 0;
	memset(rx_buffer_SMS_Async, 0, sizeof(rx_buffer_SMS_Async));
	do
	{
		if( xQueueReceive( xQueue_SMS_Async_response, rx_buffer_SMS_Async, Timeout ) == pdPASS )
		{
			memcpy( &ReceiveBuffer[ReceiveBufferIndex], rx_buffer_SMS_Async,strlen(rx_buffer_SMS_Async));
			ReceiveBufferIndex += strlen(rx_buffer_SMS_Async);
			memset(rx_buffer_SMS_Async, 1, strlen(rx_buffer_SMS_Async));
			if( ReceiveBufferIndex >= SMS_BUFFER_LENGTH )
			{
				memset(ReceiveBuffer, 0, sizeof(ReceiveBuffer));
				ReceiveBufferIndex = 0;
			}
		}
		else
		{
			;
		}
		
	}while( (uxQueueMessagesWaiting(xQueue_SMS_Async_response) != 0) );
	
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

//Receives all incoming SMS and validates both "Message"(##REG##) and "Mobile number"(+917676890629)
// If successful returns TRUE else returns FALSE
Bool SMSReceiveHandler()
{		
	uint8_t ID[1];
	IsValidationSuccess = false;
	uint8_t SMSReceiveBuffer[SMS_BUFFER_LENGTH] = {0};		
	memset(SMSReceiveBuffer,0,sizeof(SMSReceiveBuffer));
	if( true == ReadSMSAsyncResponce(SMSReceiveBuffer, 0 ) )
	{
		char data[50]="";
		Rx_MSG_flag = true;
		next_sms_state = S_SMS_IDLE;
		char* Msgid = substr(SMSReceiveBuffer, MsgID_Start_Index, MsgID_Last_Index);
		 sprintf(ID, "%s", Msgid);
		 printf("id is %s\n",ID);
		strcat(data,"AT+CMGR=");
		strcat(data,ID);
		strcat(data,"\r\n");
		 msgindex = atoi(ID);
  		memcpy(&SNES_SMS.Read_sms_cmd,&data,50);
	}
	
	
	{
	//If overflow validate received SMS and delete previous messages
		if((msgindex == Max_sms_memorysize) && (Rx_MSG_flag == true))
		{
			
			if((next_sms_state != S_SMS_Recieve_Read_SUCCESS) && (IsDeleteInProgress != true))	
			{
				Validate_Recieved_SMS();
				SMS_Validation_flag = true;
			}
			
			else if((SMS_Validation_flag = true) && (next_sms_state != S_SMS_Delete_SUCCESS))
			{
				DeleteMsgsInInbox();
				IsDeleteInProgress = true;
			}
			
			else
			{
				Rx_MSG_flag = false;
				SMS_Validation_flag = false;
				IsDeleteInProgress = false;
			}
			
		}
		
		//Validate received SMS
		else if(Rx_MSG_flag == true)
		{
			
			/*if(state_sms != S_SMS_Recieve_Read_SUCCESS) 	
			{*/
				Validate_Recieved_SMS();
			//}
			 if (next_sms_state == S_SMS_Recieve_Read_SUCCESS)
			{
				Rx_MSG_flag = false;
				//next_sms_state = S_SMS_IDLE;
			}	
		}
	}
	return IsValidationSuccess;
}


bool Validate_Recieved_SMS()
{
	uint8_t SMSReceiveBuffer[SMS_BUFFER_LENGTH] = {0};
	memset(SMSReceiveBuffer,0,sizeof(SMSReceiveBuffer));

	switch(next_sms_state)
	{
		case S_SMS_IDLE:
		{
			next_sms_state = S_Read_sms_data;
			next_sms_event = E_SMS_COMMAND;
		}break;
		
		case S_Read_sms_data:
		{
			switch(next_sms_event)
			{
				case E_SMS_COMMAND:
				{
					IsSMSValid = false;
					IsPhonenumberValid = false;
					if(IsSpaceAvailableInSmsCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 300;
						xQueueSend( xQueue_SMS_command, SNES_SMS.Read_sms_cmd, xTicksToWait);
						next_sms_state = S_Read_sms_data;
						next_sms_event = E_SMS_RESPONSE;
						break;
					}
					else
					{
						next_sms_event= E_SMS_COMMAND;
						break;
					}
				}break;
				
				case E_SMS_RESPONSE:
				{
					memset(SMSReceiveBuffer,0,sizeof(SMSReceiveBuffer));
					if( true == ReadSMSResponce(SMSReceiveBuffer, 0 ) )
					{
						char token[500];
						memset(token,0,sizeof(SMSReceiveBuffer));
						strcpy(token,SMSReceiveBuffer);
						
						//To extract SMS message info.
						
					
						if (strstr(token,"Reg") != NULL)
						{
							IsSMSValid = true;
						}
						
						//To extract mobile number
						char* Phone_num = substr(SMSReceiveBuffer, MobileNum_Start_Index, MobileNum_Last_Index);
				
						if(strstr(SMSReceiveBuffer, "7899688017" ) != NULL )
						{
							IsPhonenumberValid = true;
						}
						
						if ((IsSMSValid && IsPhonenumberValid) == true)
						{
							next_sms_state = S_SMS_Recieve_Read_SUCCESS;
							IsValidationSuccess = true;
						}
						
					}
					
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_sms_state = S_Read_sms_data;
						next_sms_event = E_SMS_COMMAND;
						break;
					}
					
				}break;
				
				default: break;
			}
			
		}break;
		
		default:break;
	}

	return (next_sms_state == S_SMS_Recieve_Read_SUCCESS);
}
	

bool DeleteMsgsInInbox(){
	uint8_t SMSReceiveBuffer[SMS_BUFFER_LENGTH] = {0};
	memset(SMSReceiveBuffer,0,sizeof(SMSReceiveBuffer));
  
	switch(next_sms_state)
	{
		case S_SMS_Recieve_Read_SUCCESS:
		{
			next_sms_state = S_Turnonpdumode;
			next_sms_event = E_SMS_COMMAND;
		}break;
				
		case S_Turnonpdumode:
		{
			
			switch(next_sms_event){
				
				case E_SMS_COMMAND:
				{
					
					if(IsSpaceAvailableInSmsCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 200;
						xQueueSend( xQueue_SMS_command,"AT+CMGF=0\r\n", 0);    //For PDU mode '0', for text mode '1'
						next_sms_state = S_Turnonpdumode;
						next_sms_event = E_SMS_RESPONSE;
					}
					else
					{
						next_sms_event = E_SMS_COMMAND;
					}
					
				}break;
				
				case E_SMS_RESPONSE:
				{
					memset(SMSReceiveBuffer,0,sizeof(SMSReceiveBuffer));
					if( true == ReadSMSResponce(SMSReceiveBuffer, 0 ) )
					{
						
						if(strstr(SMSReceiveBuffer, "OK" ) != NULL )
						{
							next_sms_state = S_deleteallmessages;
							next_sms_event = E_SMS_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						next_sms_event = E_SMS_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;
				
				
		case S_deleteallmessages:
			{
				switch(next_sms_event)
				{
					case E_SMS_COMMAND:
					{
						if(IsSpaceAvailableInSmsCommandQueue())
						{
							vTaskSetTimeOutState( &xTimeOut );
							xTicksToWait = 300;
							xQueueSend( xQueue_SMS_command, "AT+CMGD=1,4\r\n", 0);
							next_sms_event = E_SMS_RESPONSE;
						}
						else
						{
							next_sms_event = E_SMS_COMMAND;
						}
					}break;
					
					case E_SMS_RESPONSE:
					{
						memset(SMSReceiveBuffer,0,sizeof(SMSReceiveBuffer));
						if( true == ReadSMSResponce(SMSReceiveBuffer, 0 ) )
						{
							if(strstr(SMSReceiveBuffer, "OK" ) != NULL )
							{
								next_sms_state = S_TurnOnTextMode;
								next_sms_event = E_SMS_COMMAND;
								
								
							}
						}
						if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
						{
							next_sms_event = E_SMS_COMMAND;
							break;
						}
					}
					break;
				default: break;
				}
				
			}break;	
			
		case S_TurnOnTextMode:
		{
			switch(next_sms_event)
			{
				case E_SMS_COMMAND:
				{
					if(IsSpaceAvailableInSmsCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xTicksToWait = 200;
						xQueueSend( xQueue_SMS_command, "AT+CMGF=1\r\n", 0);    //For PDU mode '0', for text mode '1'
						next_sms_state = S_TurnOnTextMode;
						next_sms_event= E_SMS_RESPONSE;
					}
					else
					{
						next_sms_event = E_SMS_COMMAND;
					}
					
				}break;
				
				case E_SMS_RESPONSE:
				{
					memset(SMSReceiveBuffer,0,sizeof(SMSReceiveBuffer));
					if( true == ReadSMSResponce(SMSReceiveBuffer, 0 ) )
					{
						if(strstr(SMSReceiveBuffer, "OK" ) != NULL )
						{
							next_sms_state = S_SMS_Delete_SUCCESS;
							next_sms_event = E_SMS_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
					{
						
						next_sms_event = E_SMS_COMMAND;
						break;
					}
				}break;
				
				default: break;
			}
		}break;
				
	
	default:break;
	}
				
	return (next_sms_state == S_SMS_Delete_SUCCESS);	
}


	

   
   
   
   
   
   
   
   
   
   
   
	 
		   
		

