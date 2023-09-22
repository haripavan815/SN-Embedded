/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   Services.c                                                                                                         */
/*                                                                                                                                  */
/*  Description:  This file Includes checking the health status of LTE, MQTT and SMS, executes every 50sec.
/*																					                                                */
/*																																	*/
/*  -----------------------------------------------------------------------------------------------------------------------------   */
/*  |  Author              |    Date      | Rev  |									Comments								|		*/
/*  -----------------------------------------------------------------------------------------------------------------------------	*/
/*  | Harish N             |  19-01-2020  | 1.0  |          Created                                                         |       */
/*------------------------------------------------------------------------------------------------------------------------------	*/
/*																																	*/
/*  Copyright (c) 2021, SNES																										*/
/*  All rights reserved.																											*/
/*																																    */
/************************************************************************************************************************************/
#include "Services_Module/Services.h"
#include <string.h>
#include "CmdRes.h"

const char* LTE_Cmd_Resp_State_Names[] = {	"S_IDLE", "S_IsLTE_OK", 
	"S_ErrorMessageFormat",
	"S_IsSimReady", "S_IsNwRegReady", "S_IsGPSNwRegReady", "S_Init_SUCCESS", "S_ERROR",
	"S_TurnOffGNSSEngine", "S_TurnOnGNSSEngine", "S_ConfigGpsDebugPort", "S_ConfigGPSNmea", "S_ConfigGPSAutoOutput",
	"S_SaveATSettingsToNvm", "S_TurnOffEcho",
	"S_GPSRead", "S_SMS_Recieve_SUCCESS"
};
const char* LTE_Cmd_Resp_Event_Names[] =  {	"E_COMMAND","E_RESPONSE"};
const char* MQTT_Cmd_Resp_State_Names[] = {"S_MQTT_IDLE", "S_MQTT_SET_CLIENT_ID", "S_MQTT_OPEN", "S_MQTT_CONNECT", "S_MQTT_SUBSCRIBE", "S_MQTT_PUBLISH","S_MQTT_SUCCESS","S_MQTT_ERROR"};
const char* MQTT_Cmd_Resp_Event_names[] = {"E_MQTT_COMMAND","E_MQTT_MESSAGE","E_MQTT_RESPONSE"};
const char* SMS_Cmd_Resp_State_Names[] = {"S_SMS_IDLE", "S_TurnOnTextMode", "S_EnterNumber", "S_EnterText", "S_SMS_Send_SUCCESS",
	"S_SMS_ERROR","S_SMS_Recieve_SUCCESS","S_ReadAllMsg"
};
const char* SMS_Cmd_Resp_Event_names[] = {"E_MQTT_COMMAND","E_MQTT_MESSAGE","E_MQTT_RESPONSE"};

enum LTE_Cmd_Resp_State state_init;
enum LTE_Cmd_Resp_Event event_init;

enum MQTT_Cmd_Resp_State state_mqtt;
enum MQTT_Cmd_Resp_Event event_mqtt;

enum SMS_Cmd_Resp_State state_sms;
enum SMS_Cmd_Resp_Event event_sms;
bool health_check = false;

//This task provides the health status of LTE, MQTT and SMS, executes every 50sec
 void task_Services( void * pvParameters )
 {
	 TickType_t xLastWakeTime;
	 const TickType_t xFrequency = 50000;

	 xLastWakeTime = xTaskGetTickCount ();
	 for( ;; )
	 {
		 vTaskDelayUntil( &xLastWakeTime, xFrequency );
		 
		 state_init = get_LTE_InitState();
		 event_init = get_LTE_InitEvent();
		 state_mqtt = get_MQTT_State();
		 state_sms = get_SMS_State();
		 
		 /*if ((state_init != S_Init_SUCCESS) && (state_mqtt != S_MQTT_SUCCESS))
		 {
			 SetDeviceToFailedMode();
		 }*/
		printf("\r\nInitialization Status: %s\r\n", LTE_Cmd_Resp_State_Names[state_init]);
		printf("MQTT Status: %s\r\n", MQTT_Cmd_Resp_State_Names[state_mqtt]);
		printf("SMS Status: %s\r\n\r\n", SMS_Cmd_Resp_State_Names[state_sms]);
		get_reset_reason();
		/*if( true == g_IsIgnitionON )
		{
			printf("Ignition is ON \n");
		}
		else
		{
			printf("Ignition is OFF \n");
		}*/
	}
 }