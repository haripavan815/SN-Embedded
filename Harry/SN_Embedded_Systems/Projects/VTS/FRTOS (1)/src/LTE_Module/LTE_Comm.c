
/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   LTE_Comm.c                                                                                                         */
/*                                                                                                                                  */
/*  Description:   Contains processing of  all LTE commands transmission operations and parsing all LTE command responses			*/
/*					and Asynchronous events											                                                */
/*																																	*/
/*  -----------------------------------------------------------------------------------------------------------------------------   */
/*  |  Author              |    Date      | Rev  |									Comments								|		*/
/*  -----------------------------------------------------------------------------------------------------------------------------	*/
/*  | Harish N  		   |  04-08-2022  | 1.0  |			Added LTE commands processing and parsing Functionalities.		|		*/
/*------------------------------------------------------------------------------------------------------------------------------	*/
/*																																	*/
/*  Copyright (c) 2021, SNES																										*/
/*  All rights reserved.																											*/
/*																																    */
/************************************************************************************************************************************/
/* LTE Communication */
#include "LTE_Module/LTE_Comm.h"
#include "USART_Module/usart_app.h"
#include "LTE_Module/SMS_Comm.h"
#include <string.h>

#define MESSAGE_QUEUE_SIZE 512

extern uint8_t tx_buffer_usart0[MESSAGE_QUEUE_SIZE];
extern bool tx_completed_usart0;

extern QueueHandle_t xQueue_usart0_response;
extern QueueHandle_t xQueue_usart1_response;

extern QueueHandle_t xQueue_Initialize_command;
extern QueueHandle_t xQueue_Initialize_response;

extern QueueHandle_t xQueue_GPS_command;
extern QueueHandle_t xQueue_GPS_response;

extern QueueHandle_t xQueue_MQTT_command;
extern QueueHandle_t xQueue_MQTT_response;
extern QueueHandle_t xQueue_MQTT_Async_response;

extern QueueHandle_t xQueue_SMS_command;
extern QueueHandle_t xQueue_SMS_response;
extern QueueHandle_t xQueue_SMS_Async_response;

// brief This task, when activated, it will process all LTE commands transmission operations
void task_LTE_Commander(void *pvParameters)
{
	UNUSED(pvParameters);
	
	uint8_t tx_buffer_initialize[MESSAGE_QUEUE_SIZE];
	memset(tx_buffer_initialize, 0, MESSAGE_QUEUE_SIZE);

	//// We are not Commanding GPSLOC anymore!!
	//uint8_t tx_buffer_gps[MESSAGE_QUEUE_SIZE];
	//memset(tx_buffer_gps, 0, MESSAGE_QUEUE_SIZE);
	
	uint8_t tx_buffer_mqtt[MESSAGE_QUEUE_SIZE];
	memset(tx_buffer_mqtt, 0, MESSAGE_QUEUE_SIZE);
	
	uint8_t tx_buffer_sms[MESSAGE_QUEUE_SIZE];
	memset(tx_buffer_sms, 0, MESSAGE_QUEUE_SIZE);
	
	for (;;)
	{
		if( uxQueueMessagesWaiting(xQueue_Initialize_command) != 0)
		{
			if( xQueueReceive( xQueue_Initialize_command, tx_buffer_initialize, 0) == pdPASS )
			{
				processAT_Command(tx_buffer_initialize);
				memset(tx_buffer_initialize, 0, MESSAGE_QUEUE_SIZE);
			}
		}
		
		//// We are not Commanding GPSLOC anymore!!
		//if( uxQueueMessagesWaiting(xQueue_GPS_command) != 0)
		//{
		//if( xQueueReceive( xQueue_GPS_command, tx_buffer_gps, 0) == pdPASS )
		//{
		//processAT_Command(tx_buffer_gps);
		//memset(tx_buffer_gps, 0, MESSAGE_QUEUE_SIZE);
		//}
		//}
		
		if( uxQueueMessagesWaiting(xQueue_MQTT_command) != 0)
		{
			if( xQueueReceive( xQueue_MQTT_command, tx_buffer_mqtt, 0) == pdPASS )
			{
				processAT_Command(tx_buffer_mqtt);
				memset(tx_buffer_mqtt, 0, MESSAGE_QUEUE_SIZE);
			}
		}
		
		if( uxQueueMessagesWaiting(xQueue_SMS_command) != 0)
		{
			if( xQueueReceive( xQueue_SMS_command, tx_buffer_sms, 0) == pdPASS )
			{
				processAT_Command(tx_buffer_sms);
				memset(tx_buffer_sms, 0, MESSAGE_QUEUE_SIZE);
			}
		}
	}
}

/**
* \brief This task, when activated, it will parse all LTE command responses and Asynchronous events
*/
void task_LTE_Parser(void *pvParameters)
{
	uint8_t rx_buffer_message_parser[MESSAGE_QUEUE_SIZE] = {0};
		
	UNUSED(pvParameters);

	uint8_t tx_buffer_mqtt[MESSAGE_QUEUE_SIZE];
	memset(tx_buffer_mqtt, 0, MESSAGE_QUEUE_SIZE);

	//uint8_t tx_buffer_gps[MESSAGE_QUEUE_SIZE];
	//memset(tx_buffer_gps, 0, MESSAGE_QUEUE_SIZE);
	
	for (;;) 
	{	
		static signed long xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;

		if( xQueueReceiveFromISR( xQueue_usart0_response, rx_buffer_message_parser ,&xHigherPriorityTaskWoken ) == pdPASS )
		{	
			static char token_list[10][300]; // Output token list after parsing
			char* token = strtok(rx_buffer_message_parser, "\r\n");
			int num_tokens = 0; // Index to token list. We will append to the list
			while (token != NULL) {
				// Keep getting tokens until we receive NULL from strtok()
				strcpy(token_list[num_tokens], token); // Copy to token list
				num_tokens++;
				token = strtok(NULL, "\r\n"); // Get the next token. Notice that input=NULL now!
			}
 
			// Print the list of tokens, the <CR> and <LF> separated strings
			for (int i=0; i < num_tokens; i++) 
			{
				printf("I: %s\n", token_list[i]);

				memcpy(rx_buffer_message_parser, token_list[i], strlen(token_list[i]));
				
				if( (strstr(rx_buffer_message_parser, "OK") != NULL) ||
					(strstr(rx_buffer_message_parser, "ERROR") != NULL) ||
					(strstr(rx_buffer_message_parser, "Session is ongoing") != NULL) ||
					(strstr(rx_buffer_message_parser, "Not fixed now") != NULL) )
				{
					xQueueSend( xQueue_Initialize_response, rx_buffer_message_parser , 0 );
					xQueueSend( xQueue_MQTT_response, rx_buffer_message_parser , 0 );
					xQueueSend( xQueue_SMS_response, rx_buffer_message_parser , 0 );
				}

				if( (strstr(rx_buffer_message_parser, "+CREG") != NULL) ||
					(strstr(rx_buffer_message_parser, "+CGREG") != NULL)||
					(strstr(rx_buffer_message_parser, "+CMGF") != NULL) ||
					(strstr(rx_buffer_message_parser, "+CMGL") != NULL) )
				{	
					xQueueSend( xQueue_Initialize_response, rx_buffer_message_parser , 0 );
				}

				if( (strstr(rx_buffer_message_parser, "QMTOPEN") != NULL) || 
					(strstr(rx_buffer_message_parser, "QMTCONN") != NULL) ||
					(strstr(rx_buffer_message_parser, "QMTSUB") != NULL) ||
					(strstr(rx_buffer_message_parser, "QMTPUB") != NULL) || 
					(strstr(rx_buffer_message_parser, ">") != NULL) )
				{
					xQueueSend( xQueue_MQTT_response, rx_buffer_message_parser , 0 );
					xQueueSend( xQueue_SMS_response, rx_buffer_message_parser , 0 );
				}
				
				if( (strstr(rx_buffer_message_parser, "QMTRECV") != NULL) || 
					(strstr(rx_buffer_message_parser, "QMTSTAT") != NULL) )
				{
					xQueueSend( xQueue_MQTT_Async_response, rx_buffer_message_parser , 0 );
				}
				
				if( //(strstr(rx_buffer_message_parser, "READY") != NULL) ||
					(strstr(rx_buffer_message_parser, "+CMGL") != NULL) ||
					(strstr(rx_buffer_message_parser, "+CMGR") != NULL) ||
					(strstr(rx_buffer_message_parser, "+CMGD") != NULL) ||
					(strstr(rx_buffer_message_parser, "+Registeration") != NULL))
				{
					//xQueueSend( xQueue_Initialize_response, rx_buffer_message_parser , 0 );
					xQueueSend( xQueue_SMS_response, rx_buffer_message_parser , 0 );
				}
				
				if (strstr(rx_buffer_message_parser, "+CMTI") != NULL)
				
				{
					//xQueueSend( xQueue_Initialize_response, rx_buffer_message_parser , 0 );
					xQueueSend( xQueue_SMS_Async_response, rx_buffer_message_parser , 0 );
				}
				
			}

			memset(rx_buffer_message_parser, 0, MESSAGE_QUEUE_SIZE);

		}
	} /* for (;;) */
}

// brief This task, when activated, it will parse GPS related data
void task_LTE_Gps_Parser(void *pvParameters)
{
	uint8_t rx_buffer_message_parser[MESSAGE_QUEUE_SIZE] = {0};
		
	UNUSED(pvParameters);

	uint8_t tx_buffer_gps[MESSAGE_QUEUE_SIZE];
	memset(tx_buffer_gps, 0, MESSAGE_QUEUE_SIZE);

	for (;;)
	{
		static signed long xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;

		if( xQueueReceiveFromISR( xQueue_usart1_response, rx_buffer_message_parser ,&xHigherPriorityTaskWoken ) == pdPASS )
		{
			static char token_list[10][300]; // Output token list after parsing
			char* token = strtok(rx_buffer_message_parser, "\r\n");
			int num_tokens = 0; // Index to token list. We will append to the list
			while (token != NULL) {
				// Keep getting tokens until we receive NULL from strtok()
				strcpy(token_list[num_tokens], token); // Copy to token list
				num_tokens++;
				token = strtok(NULL, "\r\n"); // Get the next token. Notice that input=NULL now!
			}
			
			// Print the list of tokens, the <CR> and <LF> separated strings
			for (int i=0; i < num_tokens; i++)
			{
				//printf("Ig: %s\n", token_list[i]);
				memset(rx_buffer_message_parser, 0, sizeof(rx_buffer_message_parser));
				memcpy(rx_buffer_message_parser, token_list[i], strlen(token_list[i]));

				// Commented here because whatever comes on this port belongs to GPS Communication
				//if ( (strstr(rx_buffer_message_parser, "OK") != NULL) ||
					 //(strstr(rx_buffer_message_parser, "ERROR") != NULL) ||
					 //(strstr(rx_buffer_message_parser, "Session is ongoing") != NULL) ||
					 //(strstr(rx_buffer_message_parser, "QGPS") != NULL) ||
					 //(strstr(rx_buffer_message_parser, "Not fixed now") != NULL) )				
				{
					xQueueSend( xQueue_GPS_response, rx_buffer_message_parser , 0 );
				}
			}
			memset(rx_buffer_message_parser, 0, MESSAGE_QUEUE_SIZE);
		}
	} 
}
	
static TimeOut_t xTimeOut;
static TickType_t xTicksToWait = 300;
// Transmit AT command 		
Bool processAT_Command(uint8_t *cmd )
{
	memset(tx_buffer_usart0, 0, MESSAGE_QUEUE_SIZE);
	memcpy(tx_buffer_usart0, cmd, strlen(cmd));
							
	do 
	{
		if ( true == tx_completed_usart0 )
		{
			vTaskSetTimeOutState( &xTimeOut );
			xTicksToWait = 300;
			usart0_xdmac_configure_tx(USART0, tx_buffer_usart0, strlen(cmd));
			tx_completed_usart0 = false;
			//printf("C: %s\r\n", tx_buffer_usart0);
			break;
		}
		
		if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE )
		{
			break;
		}
		
	} while (1);
	
	return (true);
}