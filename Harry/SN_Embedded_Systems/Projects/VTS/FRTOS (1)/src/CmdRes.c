/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   CmdRes.c                                                                                                         */
/*                                                                                                                                  */
/*  Description:  Contains command response functionalities for different VTS device modes.											*/
/*																					                                                */
/*																																	*/
/*  -----------------------------------------------------------------------------------------------------------------------------   */
/*  |  Author              |    Date      | Rev  |									Comments								|		*/
/*  -----------------------------------------------------------------------------------------------------------------------------	*/
/*  | Sanjay S N           |  19-11-2020  | 1.0  |          Created                                                         |       */
/*------------------------------------------------------------------------------------------------------------------------------	*/
/*																																	*/
/*  Copyright (c) 2021, SNES																										*/
/*  All rights reserved.																											*/
/*																																    */
/************************************************************************************************************************************/

#include "CmdRes.h"

void CommandResponseUserMode()
{
	uint8_t command;
	command = GetReceivedCommand();
	CommandReceived = 0;
	if( command != 0 )
	{
		switch( command )
		{
			case	CHECK_COMMUNICATION :
											SendResponse(COMMUNICATION_ACK);
											break;
			case				IS_GPS_OK : 	CheckGPSStatus();
											SendResponse(GPS_STATUS);
											break;
			case		ENABLE_TRACKING :		EnableTraking();
											SendResponse(TRACKING_ENABLED);
											break;
			case		DISABLE_TRACKING :		DisableTraking();
											SendResponse(TRACKING_DISABLED);
											break;
			case		GET_CURRENT_LOCATION :
											SendResponse(WAIT_FOR_GPS_INFO);
											//GetLatestGPSInfo();
											SendLatestGPSInfo();
											break;
			case		GET_TRACKING_STATUS:
											SendResponse(g_ui_tracking_status);
											break;
			case		GET_DEVICE_ON_TIME:
											SendResponse(DEVICE_ON_TIME);
											break;
			case		GET_NO_OF_STORED_LOC:
											SendResponse(GET_NO_OF_STORED_LOC);
											break;
			case			GET_DEVICE_ID:
											SendResponse(GET_DEVICE_ID);
											break;

			case		GET_PREVIOUS_FIVE_LOC:
											SendResponse(SENDING_PREVIOUS_FIVE_LOC);
											IsfiveLocSentCompleted = SendPreviousFiveStoredLocations();
											break;
			case		GET_PREVIOUS_TEN_LOC:
											SendResponse(SENDING_PREVIOUS_TEN_LOC);
											IstenLocSentCompleted = SendPreviousTenStoredLocations();
											break;
			case		GET_ALL_PREVIOUS_LOC:
											SendResponse(SENDING_ALL_PREVIOUS_LOC);
											IsAllLocSentCompleted = SendAllPreviousStoredLocations();
											break;
			case		CHANGE_TO_FACTORY_MODE :
											SetDeviceToFactoryMode();
											SendResponse(MODE_CHANGED_TO_FACTORY);
											break;
			case	CHANGE_TO_MAINTENANCE_MODE :
											SetDeviceToMaintenanceMode();
											SendResponse(MODE_CHANGED_TO_MAINTENANCE);
											break;
			case		 EVERY_THING_FINE	:
												SOSDetected = false;
												//LED_Off(LED2);     To do
												//LED_Off(LED3);     To do
												break;
			
								default:
											printf("\nDevice is in User Mode\n");
											SendResponse(DEVICE_IN_NORMAL_MODE);
											break;
		}
	}
}

void CommandResponseMaintenanceMode()
{
	uint8_t command;
	command = GetReceivedCommand();
	CommandReceived = 0;
	if( command != 0 )
	{
		switch( command )
		{
			case	CHECK_COMMUNICATION :
											SendResponse(COMMUNICATION_ACK);
						  					break;
			case				IS_GPS_OK : 	CheckGPSStatus();
											SendResponse(GPS_STATUS);
											break;
			case		PERFORM_SELF_TEST :
											PerformSelfTest();
											SendResponse(SELF_TEST_RESULT_STATUS);
											break;
			case		GET_CURRENT_LOCATION:
											SendResponse(WAIT_FOR_GPS_INFO);
											//GetLatestGPSInfo();
											SendLatestGPSInfo();
											break;
			case		GET_TRACKING_STATUS:
											SendResponse(g_ui_tracking_status);
											break;
			case			GET_APP_NUMBER:
											SendResponse(GET_APP_NUMBER);
											break;
			case			GET_DEVICE_ID:
											SendResponse(GET_DEVICE_ID);
											break;
			case			GET_SIM_STATUS:
											 if ( g_b_sim_status == true )
												SendResponse(SIM_STATUS_ACTIVE);
											else
												SendResponse(GET_SIM_STATUS);

											break;
			case			ENABLE_TRACKING :
												EnableTraking();
												SendResponse(TRACKING_ENABLED);
												break;
			case			DISABLE_TRACKING :
												DisableTraking();
												SendResponse(TRACKING_DISABLED);
												break;

			case		GET_DEVICE_ON_TIME:
											SendResponse(DEVICE_ON_TIME);
											break;
			case		GET_NO_OF_STORED_LOC:
											SendResponse(GET_NO_OF_STORED_LOC);
											break;
			case		GET_PREVIOUS_FIVE_LOC:
											SendResponse(SENDING_PREVIOUS_FIVE_LOC);
											IsfiveLocSentCompleted = SendPreviousFiveStoredLocations();
											break;
			case		GET_PREVIOUS_TEN_LOC:
											SendResponse(SENDING_PREVIOUS_TEN_LOC);
											IstenLocSentCompleted = SendPreviousTenStoredLocations();
											break;
			case		GET_ALL_PREVIOUS_LOC:
											SendResponse(SENDING_ALL_PREVIOUS_LOC);
											IsAllLocSentCompleted = SendAllPreviousStoredLocations();
											break;
			case		CHANGE_TO_FACTORY_MODE :
											SetDeviceToFactoryMode();
											SendResponse(MODE_CHANGED_TO_FACTORY);
											break;
			case		CHANGE_TO_USER_MODE:
											SetDeviceToNormalMode();
											printf( "CommandResponseMaintenanceMode CHANGE_TO_USER_MODE %02X ",command);
											SendResponse(MODE_CHANGED_TO_USER);
											break;
			case		 EVERY_THING_FINE	:
											SOSDetected = false;
											//LED_Off(LED2);  To do
											//LED_Off(LED3);  To do
											break;
			
								default:
											printf("\nDevice is in Maintenance Mode\n");
											SendResponse(DEVICE_IN_MAINTENANCE_MODE);
											break;
		}
	}
}

void CommandResponseFactoryMode()
{
	uint8_t command;
	uint8_t response;
	command = GetReceivedCommand();
//	printf( "FactoryMode Received Command is %02X ",command);
	CommandReceived = 0;
	if( command != 0 )
	{
		switch( command )
		{
			case		SET_APP_MOBILE_NUMBER :  printf( "Received Command is %02X ",command);
										response = StoreMobileAppNumber();
										if ( response == true )
						 	 				SendResponse(APP_MOBILE_NUMBER_STORED);
										else
											SendResponse(SET_APP_MOBILE_NUMBER);
						  				break;
			case SET_USER_NAME			:	printf( "Received Command is %02X ",command);
											StoreUserName();
											SendResponse(SET_USER_NAME);
											break;
			case			GET_DEVICE_ID:
											SendResponse(GET_DEVICE_ID);
											break;
			case CHECK_COMMUNICATION :
											printf( "Received Command is %02X ",command);
											SendResponse(CHECK_COMMUNICATION);
										//SendResponse(COMMUNICATION_ACK);
						  					break;
			case			 IS_GPS_OK:
										response = CheckGPSStatus();
										SendResponse(response);
										break;
			case	PERFORM_SELF_TEST :
										response = PerformSelfTest();
										SendResponse(response);
										break;
			case CHANGE_TO_MAINTENANCE_MODE :
										SetDeviceToMaintenanceMode();
										printf( "CommandResponseFactoryMode CHANGE_TO_MAINTENANCE_MODE %02X ",command);
										SendResponse(MODE_CHANGED_TO_MAINTENANCE);
										break;
			case	CHANGE_TO_USER_MODE:
										SetDeviceToNormalMode();
										SendResponse(MODE_CHANGED_TO_USER);
										break;
			case EVERY_THING_FINE	:
											SOSDetected = false;
											//LED_Off(LED2);    To do
											//LED_Off(LED3);	To do
											break;						
										
							default:
										printf("\nDevice is in Factory Mode\n");
										SendResponse(DEVICE_IN_FACTORY_MODE);
										break;
		}
	}
}

void CommandResponseUSBMode()
{
	uint8_t command;
	char str[100];
	command = GetUSBReceivedCommand();
	USBCommandReceived = 0;
	if( command != 0 )
	{
		memset(str,0,sizeof(str));
		switch( command )
		{
			case	CHECK_COMMUNICATION :
												SendResponse(COMMUNICATION_ACK);
												break;
			case				IS_GPS_OK : 	CheckGPSStatus();
												SendResponse(GPS_STATUS);
												break;
			case		PERFORM_SELF_TEST :
												PerformSelfTest();
												SendResponse(SELF_TEST_RESULT_STATUS);
												break;
			case		GET_CURRENT_LOCATION:
												SendResponse(WAIT_FOR_GPS_INFO);
												//GetLatestGPSInfo();
												SendLatestGPSInfo();
												break;
			case		GET_TRACKING_STATUS:
												SendResponse(g_ui_tracking_status);
												break;
			case			GET_APP_NUMBER:
												SendResponse(GET_APP_NUMBER);
												break;
			case			GET_DEVICE_ID:
												SendResponse(GET_DEVICE_ID);
												break;
			case			GET_SIM_STATUS:
												if ( g_b_sim_status == true )
													SendResponse(SIM_STATUS_ACTIVE);
												else
													SendResponse(GET_SIM_STATUS);

												break;
			case			ENABLE_TRACKING :
												EnableTraking();
												SendResponse(TRACKING_ENABLED);
												break;
			case			DISABLE_TRACKING :
												DisableTraking();
												SendResponse(TRACKING_DISABLED);
												break;

			case		GET_DEVICE_ON_TIME:
												SendResponse(DEVICE_ON_TIME);
												break;
			case		GET_NO_OF_STORED_LOC:
												SendResponse(GET_NO_OF_STORED_LOC);
												break;
			case		GET_PREVIOUS_FIVE_LOC:
												SendResponse(SENDING_PREVIOUS_FIVE_LOC);
												IsfiveLocSentCompleted = SendPreviousFiveStoredLocations();
												break;
			case		GET_PREVIOUS_TEN_LOC:
												SendResponse(SENDING_PREVIOUS_TEN_LOC);
												IstenLocSentCompleted = SendPreviousTenStoredLocations();
												break;
			case		GET_ALL_PREVIOUS_LOC:
												SendResponse(SENDING_ALL_PREVIOUS_LOC);
												IsAllLocSentCompleted = SendAllPreviousStoredLocations();
												break;
			case		CHANGE_TO_FACTORY_MODE :
												SetDeviceToFactoryMode();
												sprintf( str,"CommandResponseMaintenanceMode CHANGE_TO_FACTORY_MODE %02X \n",command);
												printf(str);
												if( true == UsbVbusDetected)
													//udi_cdc_write_buf(str, strlen(str));
												SendResponse(MODE_CHANGED_TO_FACTORY);
												break;
			case		CHANGE_TO_USER_MODE:
												SetDeviceToNormalMode();
												sprintf( str, "CommandResponseMaintenanceMode CHANGE_TO_USER_MODE %02X \n",command);
												printf(str);
												if( true == UsbVbusDetected)
													//udi_cdc_write_buf(str, strlen(str));
												SendResponse(MODE_CHANGED_TO_USER);
												break;
			case CHANGE_TO_MAINTENANCE_MODE :
												SetDeviceToMaintenanceMode();
												sprintf( str,"CommandResponseFactoryMode CHANGE_TO_MAINTENANCE_MODE %02X \n",command);
												printf(str);
												if( true == UsbVbusDetected)
													//udi_cdc_write_buf(str, strlen(str));
												SendResponse(MODE_CHANGED_TO_MAINTENANCE);
												break;
			case EVERY_THING_FINE			:
												SOSDetected = false;
			default:
												sprintf(str,"Device is communicating through USB\n");
												printf(str);
												if( true == UsbVbusDetected)
													//udi_cdc_write_buf(str, strlen(str));
												//SendResponse(DEVICE_IN_MAINTENANCE_MODE);
												break;
		}
	}
}

void ReadCommandAndSendResponse(void)
{
	if ( DEVICE_IN_FACTORY_MODE == GetDeviceMode() || (DEVICE_UNKNOWN_MODE== GetDeviceMode()))
		CommandResponseFactoryMode();
	else
	{
		if( DEVICE_IN_MAINTENANCE_MODE == GetDeviceMode() )
			CommandResponseMaintenanceMode();
		else
		{
			if( DEVICE_IN_NORMAL_MODE == GetDeviceMode() )
				CommandResponseUserMode();
		}
	}
}

uint8_t GetDeviceMode()
{
	static uint8_t PreviousDeviceStatus = DEVICE_UNKNOWN_MODE;

	if ( g_ui_device_mode != PreviousDeviceStatus )
	{
		switch( g_ui_device_mode )
		{
			case DEVICE_IN_FACTORY_MODE :     g_ui_device_status = DEVICE_IN_FACTORY_MODE; break;
			case DEVICE_IN_MAINTENANCE_MODE : g_ui_device_status = DEVICE_IN_MAINTENANCE_MODE; break;
			case DEVICE_IN_NORMAL_MODE :        g_ui_device_status = DEVICE_IN_NORMAL_MODE; break;
			default: 					   g_ui_device_status = DEVICE_UNKNOWN_MODE;

		}
		PreviousDeviceStatus = g_ui_device_mode;
	}
	return(g_ui_device_status);
}

uint8_t GetReceivedCommand(void)
{
	//MessageReceived
	return(CommandReceived);
}

uint8_t GetUSBReceivedCommand(void)
{
	//MessageReceived
	return(USBCommandReceived);
}

void SendResponse(uint8_t Response)
{
	char res[50] ;
	uint8_t length = 21;
	memset(res,0,sizeof(res));
//	sprintf( res, "RESPONSE 0X%02X ", Response);
//	printf( "Response %02X Sent\n", Response);
	switch(Response)
	{
		case GET_APP_NUMBER:
							sprintf( res, "App Number = %s \n", &ConfigData_t.UserNumber[0] );
							length = 27;
							break;
		case GET_NO_OF_STORED_LOC:
							sprintf( res, "No of Stored Locations = %03d \n", g_ui_number_of_stored_Locations );
							length = 28;
							break;
		case DEVICE_IN_FACTORY_MODE:
								sprintf( res, "Device in Factory Mode!! \n" );
								length = 27;
								break;
		case GET_DEVICE_ID:
								sprintf( res, "Device ID : %s\n", SNSDeviceID );
								length = 48;
								break;

		 default:			sprintf( res, "RESPONSE 0X%02X \n", Response);
							length = 21;
	}
	printf("Response = %s, length = %d Response= %d ",res,length,Response);
//	if( true == UsbVbusDetected)
//		udi_cdc_write_buf(res, strlen(res));
	//SendSMS(res,strlen(res));
}

Bool CheckGPSStatus()
{

	return(true);
}

void DisableTraking()
{
	g_ui_tracking_status = TRACKING_DISABLED;
	ConfigData_t.g_ui_tracking_status = g_ui_tracking_status;
	//UpdateFlash();
	//UpdateFlashSecondary();
}
void EnableTraking()
{
	g_ui_tracking_status = TRACKING_ENABLED;
	ConfigData_t.g_ui_tracking_status = g_ui_tracking_status;
	//UpdateFlash();
	//UpdateFlashSecondary();
}

void SetDeviceToFactoryMode()
{
	g_ui_device_mode = DEVICE_IN_FACTORY_MODE;
	//ConfigData_t.g_ui_device_mode = g_ui_device_mode;
	g_ui_device_status = DEVICE_IN_FACTORY_MODE;
	//ConfigData_t.g_ui_device_status = g_ui_device_status;
	//UpdateFlash();
	//UpdateFlashSecondary();
}

void SetDeviceToMaintenanceMode()
{
	printf("SetDeviceToMaintenanceMode");
	g_ui_device_mode = DEVICE_IN_MAINTENANCE_MODE;
	//ConfigData_t.g_ui_device_mode = g_ui_device_mode;
	writeresetcountintoflash(RSTCNTVALUE);
	g_ui_device_status = DEVICE_IN_MAINTENANCE_MODE;
	//ConfigData_t.g_ui_device_status = g_ui_device_status;
	//UpdateFlash();
	//UpdateFlashSecondary();
}

void SetDeviceToNormalMode()
{
	g_ui_device_mode = DEVICE_IN_NORMAL_MODE;
	//ConfigData_t.g_ui_device_mode = g_ui_device_mode;
	writeresetcountintoflash(RSTCNTVALUE);
	g_ui_device_status = DEVICE_IN_NORMAL_MODE;
	//ConfigData_t.g_ui_device_status = g_ui_device_status;
	//UpdateFlash();
	//UpdateFlashSecondary();
}

void SetDeviceToFailedMode()
{
	int Reset_count;
	g_ui_device_mode = DEVICE_IN_FAILED_MODE;
	g_ui_device_status = DEVICE_IN_FAILED_MODE;
	delay_ms(10000);
	Reset_count = (int)ReadcounterFromFlash();
	if (Reset_count < MAX_RST_CNT)
	{
		writeresetcountintoflash(++Reset_count);
		rstc_start_software_reset(RSTC);
	}
	else
	{
		writeresetcountintoflash(RSTCNTVALUE);
		SetDeviceToHaltMode();
	}
	
	
}
void SetDeviceToUnknownMode()
{
	g_ui_device_mode = DEVICE_UNKNOWN_MODE;
	g_ui_device_status = DEVICE_UNKNOWN_MODE;
	
}
void SetDeviceToHaltMode()
{
	g_ui_device_mode = DEVICE_IN_HALT_MODE;
	g_ui_device_status = DEVICE_IN_HALT_MODE;
	printf("/********HALT MODE********/");
	while (1)
	{
		//Blinks Red LED infinitely 
		LED_Toggle(LED1);
		delay_ms(500);		
	}
	
		//Infinite loop because of IMU failure or battery failure 
	
}
Bool PerformSelfTest(void)
{
	return(true);
}
	
Bool SendAllPreviousStoredLocations(void)
{
	char strGps[100];
	static int count = 0;
	int index;
	memset(strGps,0,100);
	count++;
	index = g_ui_GpsBufferIndex - count;
	if( index < 0 ) index = index + 100;
	if ( count > g_ui_number_of_stored_Locations )
	{
		count = 0;
		return true;
	}
	else
	{
		sprintf(strGps ,"LAT %+013.8f %+013.8f %+06d %02d-%02d-%04d %02d:%02d:%02d.%03d\n",
		GpsLocationData_t[index].fLatitude,GpsLocationData_t[index].fLongitude,GpsLocationData_t[index].fAltitude,
		GpsLocationData_t[index].u8Day,GpsLocationData_t[index].u8Month,GpsLocationData_t[index].iYear,
		GpsLocationData_t[index].u8Hour,GpsLocationData_t[index].u8Minute,GpsLocationData_t[index].u8Second,GpsLocationData_t[index].iThousandthsSecond);
		printf(strGps);
		if( true == UsbVbusDetected)
		//udi_cdc_write_buf(strGps, strlen(strGps));
		if( g_b_IsGSMModemOkay == true )
		{
			//if( false == SendSMS(strGps,64))
			//{
				//printf("SMS Sending Failed\n");
				//g_b_IsGSMModemOkay = false;
			//}
		}
	}
	return false;
}

void SendLatestGPSInfo(void)
{
	char strGps[100];
	memset(strGps,0,100);
	if( LatestGpsData.iYear == 0  )
	{
		memcpy(&LatestGpsData, &GpsLocationData_t[g_ui_GpsBufferIndex-1], sizeof(LatestGpsData));
	}
	sprintf(strGps ,"LAT %+013.8f %+013.8f %+06d %02d-%02d-%04d %02d:%02d:%02d.%03d\n  ",
	LatestGpsData.fLatitude,LatestGpsData.fLongitude,LatestGpsData.fAltitude,LatestGpsData.u8Day,LatestGpsData.u8Month,LatestGpsData.iYear,
	LatestGpsData.u8Hour,LatestGpsData.u8Minute,LatestGpsData.u8Second,LatestGpsData.iThousandthsSecond);
	printf(strGps);
//	if( true == UsbVbusDetected)
//		udi_cdc_write_buf(strGps, strlen(strGps));
	if( g_b_IsGSMModemOkay == true )
	{
		//if( false == SendSMS(strGps,64))
		//{
			//printf("SMS Sending Failed\n");
			//g_b_IsGSMModemOkay = false;
		//}
	}
}

Bool SendPreviousFiveStoredLocations(void)
{
	char strGps[100];
	static int count = 0;
	int index;
	memset(strGps,0,100);

	count++;
	index = g_ui_GpsBufferIndex - count;
	if( index < 0 ) index = index + 100;
	if ( count > 5 )
	{
			count = 0;
			return true;
	}
	else
	{
		sprintf(strGps ,"LAT %+013.8f %+013.8f %+06d %02d-%02d-%04d %02d:%02d:%02d.%03d\n  ",
		GpsLocationData_t[index].fLatitude,GpsLocationData_t[index].fLongitude,GpsLocationData_t[index].fAltitude,
		GpsLocationData_t[index].u8Day,GpsLocationData_t[index].u8Month,GpsLocationData_t[index].iYear,
		GpsLocationData_t[index].u8Hour,GpsLocationData_t[index].u8Minute,GpsLocationData_t[index].u8Second,GpsLocationData_t[index].iThousandthsSecond);
		
		if( true == UsbVbusDetected)
			//udi_cdc_write_buf(strGps, strlen(strGps));
		if( g_b_IsGSMModemOkay == true )
		{
			printf(strGps);
			//if( false == SendSMS(strGps,64))
			//{
				//printf("SMS Sending Failed\n");
				//g_b_IsGSMModemOkay = false;
			//}
		}
		else
			count--;

	}
	return false;

}
Bool SendPreviousTenStoredLocations(void)
{
		char strGps[100];
		static int count = 0;
		int index;
		memset(strGps,0,100);

		count++;
		index = g_ui_GpsBufferIndex - count;
		if( index < 0 ) index = index+ 100;
		if ( count > 10 )
		{
			count = 0;
			return true;
		}
		else
		{
			sprintf(strGps ,"LAT %+013.8f %+013.8f %+06d %02d-%02d-%04d %02d:%02d:%02d.%03d\n  ",
			GpsLocationData_t[index].fLatitude,GpsLocationData_t[index].fLongitude,GpsLocationData_t[index].fAltitude,
			GpsLocationData_t[index].u8Day,GpsLocationData_t[index].u8Month,GpsLocationData_t[index].iYear,
			GpsLocationData_t[index].u8Hour,GpsLocationData_t[index].u8Minute,GpsLocationData_t[index].u8Second,GpsLocationData_t[index].iThousandthsSecond);
			printf(strGps);
			if( true == UsbVbusDetected)
				//udi_cdc_write_buf(strGps, strlen(strGps));
			if( g_b_IsGSMModemOkay == true )
			{
				//if( false == SendSMS(strGps,64))
				//{
					//printf("SMS Sending Failed\n");
					//g_b_IsGSMModemOkay = false;
				//}
			}
		}
		return false;

}

Bool StoreUserName()
{
	memcpy(&(ConfigData_t.UserName[0]), CommandParamater,40);
//	printf("StoreUserName number = %s ", ConfigData_t.UserNumber);
	//UpdateFlashSecondary();
	return 0; //(UpdateFlash());
}

Bool StoreMobileAppNumber()
{
	memcpy(&(ConfigData_t.UserNumber[0]), CommandParamater,15);
	memcpy(&(ConfigData_t.DefaultNumber[0]), CommandParamater,15);
	//UpdateFlashSecondary();
	return 0; //(UpdateFlash());
}




