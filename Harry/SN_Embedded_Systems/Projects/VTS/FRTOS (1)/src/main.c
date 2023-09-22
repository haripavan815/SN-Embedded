/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   main_application.c																								    */
/*																																	*/
/*  Description:   contains main application, tasks and handlers																	*/
/*																																	*/
/*  -----------------------------------------------------------------------------------------------------------------------------   */
/*  |  Author              |    Date      | Rev  |									Comments								|		*/
/*  -----------------------------------------------------------------------------------------------------------------------------	*/
/*  | Naveen G S		   |  01-12-2021  | 1.0  |			Created													        |	    */
/*  | Harish N  		   |  30-06-2022  | 1.1  |			Added SMS,Services and AIS-140 module							|		*/
/*	| Harish N 		       |  26-09-2021  | 1.2	 |          Added I2C module												|		*/
/*------------------------------------------------------------------------------------------------------------------------------	*/
/*																																	*/
/*  Copyright (c) 2021, SNES																										*/
/*  All rights reserved.																											*/
/*																																    */
/************************************************************************************************************************************/

#include <asf.h>
#include <conf_board.h>
#include <string.h>
#include "CmdRes.h"
#include "conf_board.h"
#include "function.h"
#include "VTS_Project_Config.H"
extern uint8_t tx_buffer_usart0[TRANSFER_SIZE];
extern uint8_t rx_buffer_usart0[TRANSFER_SIZE];

extern uint8_t tx_buffer_usart1[TRANSFER_SIZE];
extern uint8_t rx_buffer_usart1[TRANSFER_SIZE];

extern bool tx_completed_usart0;
extern bool rx_completed_usart0;

extern bool tx_completed_usart1;
extern bool rx_completed_usart1;
SemaphoreHandle_t xSemaphoreLTE = NULL;
int count = 0;
int intPin = 12;  
double Accel_Z_corrector = 14418.0;
//g_ul_ms_ticks = 0;
Gscale = GFS_250DPS;
Ascale = AFS_2G; 
volatile uint8_t g_ui_device_mode = DEVICE_UNKNOWN_MODE;
volatile uint8_t g_ui_device_status = DEVICE_UNKNOWN_MODE;
bool IsSystemBooted, IsUART0Configured, IsI2CConfigured, IsSPIConfigured,
IsQSPIConfigured, IsUSARTConfigured, IsLTEConfigured, IsIMUInitalised, IsBatterySafe = false;
struct FACTORYMODECONFIGDATA FactorySettings;
struct User_Registration_data writedata, readinfo;
struct LocationData writegpsdata, readgpsdata;
bool result, testdata;
volatile int g_ui_GpsBufferIndex = 0;


/**
 *  \brief Application entry point.
 *
 *  \return Unused (ANSI-C compatibility).
*/

int main(void)
{	
	/* Initialize the SAM system */
	sysclk_init();
	
	board_init(); 
	
	/* Initialize LTE Board */
	LTE_BoardInit();
	
	/* Initialize Free-RTOS */
	RTOS_Init();
	
	/* Initialize the console uart */
	configure_console();
	
	/* Output demo information. */
	printf("-- SNES Free-RTOS based VTD Project V1.2 --\n\r");
	printf("-- %s\n\r", BOARD_NAME);
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
	printf("\nBooting LTE Module....\n\r");
	
	// Initialize USART0 
	configure_usart(ID_USART0, 115200);	
	
	//Initialize ignition recognition
	IntialiseIgnitionRecognition();
	
	// Testing IMU sensor
	SelfTest_MPU6050();
	
	//Check weather battery voltage is proper
	RecogniseIgnition();
	
	
	//Initialize flash 
	if( InitializeFlash() == false)
	{
		printf( "flash initializations failed\n");
	}
	WriteFactoryConfigdatatoFlash();
	FactorySettings = GetFactoryDetails();
	printf("phone number is  %s\n", FactorySettings.phnumber);
	printf("URL is  %s\n", FactorySettings.url);
	while (false == ReadFactoryConfigDataFromFlash())
	{
		//Sets mode to failed mode
		SetDeviceToFailedMode();
	}
	
	if (true == InitilizeQSPIFlash())
	{
		// **************************storing the user registration details into qspi****************************** //
		memset(&writedata,0,sizeof(writedata));
		memset(&readinfo,0,sizeof(readinfo));
		writedata.First_name = "Harry";
		writedata.Last_name = "N";
		writedata.Mobile_no = "7676890629";
		writedata.Email_ID = "harish.mbhv@gmail.com";
		writedata.checksum=0;

		if(false == writeuserdetailsintoqspi(USER_CONFIGDATA_QSPIADDRESS,writedata))
		{
			result = Readuserdetailsfromqspi(USER_CONFIGDATA_QSPIADDRESS,&readinfo);
			printf("Result is %d\n",result);
			printf("username is %s\n",readinfo.First_name);
			printf("username is %s\n",readinfo.Last_name);
			printf("username is %s\n",readinfo.Mobile_no);
			printf("username is %s\n",readinfo.Email_ID);
		}
		

		// ***************************storing loaction details into qspi******************************* //
		memset(&writegpsdata,0,sizeof(writegpsdata));
		memset(&readgpsdata,0,sizeof(readgpsdata));
		writegpsdata.fLatitude =  12.547592;
		writegpsdata.fLongitude = -56.897970;
		writegpsdata.fAltitude = -1020;
		writegpsdata.iYear	= 2028;
		writegpsdata.u8Month = 8;
		writegpsdata.u8Day = 10;
		writegpsdata.u8Hour = 22;
		writegpsdata.u8Minute = 39;
		writegpsdata.u8Second = 30;
		writegpsdata.iThousandthsSecond = 1 + g_ui_GpsBufferIndex;
		
		if (false == writegpsdataintoqspi(GPSDATA_QSPIADDRESS,writegpsdata))
		{
			testdata = Readgpsdatafromqspi(GPSDATA_QSPIADDRESS, &readgpsdata);
			printf("Result is %d\n",testdata);
			printf(" LAT %+013.8f %+013.8f %+06d %02d-%02d-%04d %02d:%02d:%02d.%03d\n" ,readgpsdata.fLatitude,
			readgpsdata.fLongitude,
			readgpsdata.fAltitude,
			readgpsdata.u8Day,
			readgpsdata.u8Month,
			readgpsdata.iYear,
			readgpsdata.u8Hour,
			readgpsdata.u8Minute,
			readgpsdata.u8Second,
			readgpsdata.iThousandthsSecond);
		}
	}
	
	if(!IsBoardHealthSuccess)
	{
		switch(CheckBoardHealth())
		{
			case DEVICE_IN_FAILED_MODE :	SetDeviceToFailedMode();
											printf("Failed to initialize board\r\n");
											break;
			case DEVICE_IN_FACTORY_MODE :	SetDeviceToFactoryMode();
											break;
			case DEVICE_IN_HALT_MODE :		SetDeviceToHaltMode();
											break;
			case DEVICE_UNKNOWN_MODE :		SetDeviceToUnknownMode();
											printf("Board Initialization Success!!\r\n");
											IsBoardHealthSuccess = true;
											break;
			default:SetDeviceToUnknownMode();
		}		
	}
	
	if ((g_ui_device_mode == DEVICE_UNKNOWN_MODE) || (g_ui_device_mode == DEVICE_IN_FAILED_MODE))
	{
		//Read factory config details from flash i.e URL and Company's mobile number
		if(true == ReadFactoryConfigDataFromFlash())
		{
			SetDeviceToNormalMode();
			printf("/*******Normal mode******/\n");
	
		}
		else
		SetDeviceToFailedMode();	
	}
	
	//WriteFactoryConfigdatatoFlash();
	/*if (g_ui_device_mode == DEVICE_IN_FACTORY_MODE)
	{
		//Update phone number and server URL on to flash
		WriteFactoryConfigdatatoFlash();
	}*/
	
	//Enters maintenance mode from USB Interrupt
	if (g_ui_device_mode == DEVICE_IN_MAINTENANCE_MODE)
	{
		printf("---Maintenance mode---\r\n");
	}
	
	// Run Free-RTOS application	
	Run_RTOS_Application();
	
	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}

//Checks the board health(UART, I2C, SPI, QSPI, IMU, BATTERY) and identifies the mode 
uint8_t CheckBoardHealth()
{
	IsBatterySafe = true;							
	if ((IsUART0Configured && IsI2CConfigured && IsSPIConfigured && 
	IsQSPIConfigured && IsUSARTConfigured && IsLTEConfigured && 
	IsIMUInitalised && IsBatterySafe) != true)
	{
		if ((IsBatterySafe) == false)
		{
			SetDeviceToHaltMode();
			return DEVICE_IN_HALT_MODE;
		}
		else
		{
			SetDeviceToFailedMode();
			return DEVICE_IN_FAILED_MODE;
		}	
	}
	else
	return DEVICE_UNKNOWN_MODE;	
}

//Gets the reason for reset and prinit it on the console
void get_reset_reason(void)
{
	char msg[80];
	
	strcpy(&msg[0], "\r\nReset info : ");
	
	//! [reset_get_status]
	uint32_t info = rstc_get_status(RSTC);
	//! [reset_get_status]
	
	/* Decode the reset reason. */
	switch (info & RSTC_SR_RSTTYP_Msk)
	 {
		case RSTC_GENERAL_RESET:
		strcat(&msg[0], "General Reset,");
		break;
		
		case RSTC_BACKUP_RESET:
		strcat(&msg[0], "Backup Reset,");
		break;
		
		case RSTC_WATCHDOG_RESET:
		strcat(&msg[0], "Watchdog Reset,");
		break;
		
		case RSTC_SOFTWARE_RESET:
		strcat(&msg[0], "Software Reset,");
		break;
		
		case RSTC_USER_RESET:
		strcat(&msg[0], "User Reset,");
		break;
		
		default:
		strcat(&msg[0], "Invalid reset reason!,");
	}
	
	/* NRST level. */
	if (info & RSTC_SR_NRSTL) 
	{
		strcat(&msg[0], " NRST=1,");
	}
	else
	{
		strcat(&msg[0], " NRST=0,");
	}

	/* User reset status. */
	if (info & RSTC_SR_URSTS) 
	{
		strcat(&msg[0], " User Reset=1\r");
	}
	else 
	{
		strcat(&msg[0], " User Reset=0\r");
	}

	puts(&msg[0]);
	
}

//Initializes RTOS queues and tasks
void RTOS_Init()
{
	InitialiseUsart0Queue();
	InitialiseUsart1Queue();
	InitializeInitQueue();
	InitialiseGpsQueue();
	InitialiseMqttQueue();
	InitialiseSmsQueue();
	

	// Create Semaphore and use it if necessary! (on shared resource between tasks)
	xSemaphoreLTE = xSemaphoreCreateMutex();
	if( xSemaphoreLTE == NULL )
	{
		printf("Failed to create xSemaphoreLTE\r\n");
	}

	// Create Tasks here
	if (xTaskCreate(task_Health_monitor, "Health_Monitor", task_health_monitor_STACK_SIZE, NULL,
	task_health_monitor_PRIORITY, NULL) != pdPASS)
	 {
		printf("Failed to create task_Health_monitor\r\n");
	}
	
	if (xTaskCreate(task_Main_App, "MainApp", task_Main_App_STACK_SIZE, NULL,
	task_Main_App_PRIORITY, NULL) != pdPASS) 
	{
		printf("Failed to create task_Main_App\r\n");
	}

	if (xTaskCreate(task_LTE_Commander, "LTE_IO", task_LTE_Comm_STACK_SIZE, NULL,
	task_LTE_Comm_PRIORITY, NULL) != pdPASS)
	{
		printf("Failed to create task_LTE_Commander\r\n");
	}
	if (xTaskCreate(task_LTE_Parser, "LTE_Parser", task_LTE_Comm_STACK_SIZE, NULL,
	task_LTE_Comm_PRIORITY, NULL) != pdPASS) 
	{
		printf("Failed to create task_LTE_Parser\r\n");
	}

	if (xTaskCreate(task_LTE_Gps_Parser, "LTE_Gps_Parser", task_LTE_Comm_STACK_SIZE, NULL,
	task_LTE_Comm_PRIORITY, NULL) != pdPASS) 
	{
		printf("Failed to create task_LTE_Gps_Parser\r\n");
	}
	
	if (xTaskCreate(task_Services, "Services", task_Main_App_STACK_SIZE, NULL,
	task_Main_App_PRIORITY, NULL) != pdPASS) 
	{
		printf("Failed to create task_Services\r\n");
	}

	if (xTaskCreate(task_AIS, "AIS_140", task_Main_App_STACK_SIZE, NULL,
	task_Main_App_PRIORITY, NULL) != pdPASS) 
	{
		printf("Failed to create task_AIS\r\n");
	}
	
	// Configure RX for Asynchronous receptions, TX will be configured during transmissions
	usart0_xdmac_configure_rx(USART0, rx_buffer_usart0, TRANSFER_SIZE);
	usart1_xdmac_configure_rx(USART1, rx_buffer_usart1, TRANSFER_SIZE);
	
}

void Run_RTOS_Application()
{
	IsSystemBooted = true;
	/* Start the scheduler. */
	vTaskStartScheduler();
}

/**
 * \brief XDMAC interrupt handler.
 */
void XDMAC_Handler(void)
{
	uint32_t dma_status_tx_usart0, dma_status_rx_usart0;
	
	dma_status_tx_usart0 = xdmac_channel_get_interrupt_status(XDMAC, XDMAC_TX_CH);
	if (dma_status_tx_usart0 & XDMAC_CIS_BIS)
	{
		NVIC_ClearPendingIRQ(XDMAC_IRQn);
		NVIC_DisableIRQ(XDMAC_IRQn);
		tx_completed_usart0 = true;
	}
	
	dma_status_rx_usart0 = xdmac_channel_get_interrupt_status(XDMAC, XDMAC_RX_CH);
	if(dma_status_rx_usart0 & XDMAC_CIS_BIS)
	{
		NVIC_ClearPendingIRQ(XDMAC_IRQn);
		NVIC_DisableIRQ(XDMAC_IRQn);
		rx_completed_usart0 = true;
	}
	
	uint32_t dma_status_tx_usart1, dma_status_rx_usart1;
	
	dma_status_tx_usart1 = xdmac_channel_get_interrupt_status(XDMAC, XDMAC_TX_CH1);
	if (dma_status_tx_usart1 & XDMAC_CIS_BIS)
	{
		NVIC_ClearPendingIRQ(XDMAC_IRQn);
		NVIC_DisableIRQ(XDMAC_IRQn);
		tx_completed_usart1 = true;
	}
	
	dma_status_rx_usart1 = xdmac_channel_get_interrupt_status(XDMAC, XDMAC_RX_CH1);
	if(dma_status_rx_usart1 & XDMAC_CIS_BIS)
	{
		NVIC_ClearPendingIRQ(XDMAC_IRQn);
		NVIC_DisableIRQ(XDMAC_IRQn);
		rx_completed_usart0 = true;
	}
}


	
