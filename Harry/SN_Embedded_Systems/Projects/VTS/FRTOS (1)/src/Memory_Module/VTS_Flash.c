/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   VTS_Flash.c                                                                                                        */
/*                                                                                                                                  */
/*  Description:   Contains Read and Write into internal flash Functionalities.
/*																					                                                */
/*																																	*/
/*  -----------------------------------------------------------------------------------------------------------------------------   */
/*  |  Author              |    Date      | Rev  |									Comments								|		*/
/*  -----------------------------------------------------------------------------------------------------------------------------	*/
/*  | Sanjay S N           |  19-01-2020  | 1.0  |          Created                                                         |       */
/*  | Mamatha V 		   |  04-09-2022  | 1.0  |			Modified with new memory mapping for factory configuration.								|		*/
/*------------------------------------------------------------------------------------------------------------------------------	*/
/*																																	*/
/*  Copyright (c) 2021, SNES																										*/
/*  All rights reserved.																											*/
/*																																    */
/************************************************************************************************************************************/

#include "VTS_Flash.h"
#include "CmdRes.h"
#include "function.h"
#include "samv71j21b.h"

struct qspid_t g_qspid = { QSPI, 0, 0, 0 };
//struct qspi_config_t mode_config = {QSPI_MR_SMM_SPI, false, false, QSPI_LASTXFER, 0, 0, 0, 0, 0, 0, 0, false, false, 0};
struct qspi_config_t mode_config = {QSPI_MR_SMM_MEMORY, false, false, QSPI_LASTXFER, 0, 0, 0, 0, 0, 0, 0, false, false, 0};
	
// writing gps data into internal flash
Bool WriteLocationToFlash()
{
	Bool flashWriteStatus;
	uint8_t flashWriteAttempt;
	uint32_t ul_GpsData_page_addr = GPS_DATA_PAGE_ADDRESS;
	uint32_t GpspageSize = sizeof(GpsLocationData_t);
	uint32_t pul_GpsData_page =  ul_GpsData_page_addr;
	struct GPSLocationData *flashData2;
//	uint16_t configLength = sizeof(GpsLocationData_t);
	uint8_t *configpointer;

	memcpy(&(GpsLocationData_t[0]),flashData2,2400);
	flashWriteStatus = false;
	flashWriteAttempt = 0;

	g_ui_number_of_stored_Locations++;
	if(g_ui_number_of_stored_Locations > 100 ) g_ui_number_of_stored_Locations = 100;
	GpsLocationData_t[g_ui_GpsBufferIndex].fLatitude = GpsDataRecord.fLatitude;
	GpsLocationData_t[g_ui_GpsBufferIndex].fLongitude = GpsDataRecord.fLongitude;
	GpsLocationData_t[g_ui_GpsBufferIndex].fAltitude = GpsDataRecord.fAltitude;
	GpsLocationData_t[g_ui_GpsBufferIndex].iYear = GpsDataRecord.iYear;
	GpsLocationData_t[g_ui_GpsBufferIndex].u8Month = GpsDataRecord.u8Month;
	GpsLocationData_t[g_ui_GpsBufferIndex].u8Day = GpsDataRecord.u8Day;
	GpsLocationData_t[g_ui_GpsBufferIndex].u8Hour = GpsDataRecord.u8Hour;
	GpsLocationData_t[g_ui_GpsBufferIndex].u8Minute = GpsDataRecord.u8Minute;
	GpsLocationData_t[g_ui_GpsBufferIndex].u8Second = GpsDataRecord.u8Second;
	GpsLocationData_t[g_ui_GpsBufferIndex].iThousandthsSecond = GpsDataRecord.iThousandthsSecond;
	increment_GpsBufferIndex();
	

	do{
		////wdt_restart(WDT);
		flashWriteAttempt++;
		if( flashWriteStatus = WriteFlash(ul_GpsData_page_addr, &GpsLocationData_t[0], sizeof(GpsLocationData_t)) )
		printf("GPS Data Flash write successful  \n ");
	}while( (flashWriteStatus == false) && ( flashWriteAttempt < 3 ));

	if ( flashWriteAttempt >= 3 )
	{
		 decrement_GpsBufferIndex();
		 if(g_ui_number_of_stored_Locations <= 99 )
			 g_ui_number_of_stored_Locations--;
		printf("Problem with GPS Data Flash Writing");
	}
	else
	{

		////wdt_restart(WDT);
		UpdateFlash();
		//wdt_restart(WDT);
		UpdateFlashSecondary();
	}
	return(flashWriteStatus);
}

void decrement_GpsBufferIndex()
{
	g_ui_GpsBufferIndex--;
	if( g_ui_GpsBufferIndex < 0 )
		g_ui_GpsBufferIndex = 99;
}

void increment_GpsBufferIndex()
{
	ConfigData_t.g_ui_GpsBufferIndex++;
	UpdateFlash();
	if( ConfigData_t.g_ui_GpsBufferIndex > 4 )
	{
	ConfigData_t.g_ui_GpsBufferIndex = 0;
	//flash_lock(ul_test_page_addr, ul_test_page_addr + (IFLASH_PAGE_SIZE*260) - 1, 0, 0);
	NVIC_SystemReset();
	}
}

Bool ReadConfigDataFromFlashSecondary()
{
	struct ConfigData *flashData1;
	uint8_t configLength = sizeof(ConfigData_t)-2;
	uint8_t *configpointer1;
	uint32_t ul_test_page_addr1 = PAGE_ADDRESS_SECONDARY;
	uint32_t pul_test_page1 =  ul_test_page_addr1;
	flashData1 = (struct ConfigData*) pul_test_page1;

//	flash_unlock(ul_test_page_addr1,  ul_test_page_addr1 + (IFLASH_PAGE_SIZE*1) - 1, 0, 0);
	configpointer1 = (uint8_t*)flashData1;
	uint8_t calculatedChecksum1 =0;
	for( int i = 0 ; i < configLength; i++)
	{
		calculatedChecksum1 = calculatedChecksum1 ^ configpointer1[i];
		//		printf("\n%d = %0X",i,configpointer[i]);
	}
//	printf( "ReadConfigDataFromFlashSecondary flashData = %X \n",pul_test_page1);
	if( flashData1->g_ui_Known_value != 0x5A )
	{
		printf("No Valid config data in flash \n");
		return false;
	}
	if( calculatedChecksum1 == flashData1->Checksum )
	{
		printf( "Name = %s \n",flashData1->UserName);
		memcpy(&ConfigData_t.UserName[0],&flashData1->UserName[0],40);
		printf("Number = %s \n",flashData1->UserNumber);
		memcpy(&ConfigData_t.UserNumber[0],&flashData1->UserNumber[0],20);
		printf("Default Number = %s \n",flashData1->DefaultNumber);
		memcpy(&ConfigData_t.DefaultNumber[0],&flashData1->DefaultNumber[0],20);

		ConfigData_t.g_ui_device_mode = flashData1->g_ui_device_mode;
		g_ui_device_mode = flashData1->g_ui_device_mode;

		ConfigData_t.g_ui_device_status = flashData1->g_ui_device_status;
		g_ui_device_status = flashData1->g_ui_device_status;

		ConfigData_t.g_ui_number_of_stored_Locations = flashData1->g_ui_number_of_stored_Locations;
		g_ui_number_of_stored_Locations = flashData1->g_ui_number_of_stored_Locations;

		ConfigData_t.g_ui_GpsBufferIndex = flashData1->g_ui_GpsBufferIndex;
		g_ui_GpsBufferIndex = flashData1->g_ui_GpsBufferIndex;
		switch(flashData1->g_ui_device_mode)
		{
			case DEVICE_IN_FACTORY_MODE: printf("Device Mode : Factory Mode\n"); break;
			case DEVICE_IN_MAINTENANCE_MODE: printf("Device Mode : Maintenance Mode\n"); break;
			case DEVICE_IN_NORMAL_MODE: printf("Device Mode : User Mode\n"); break;
			case DEVICE_UNKNOWN_MODE: printf("Device Mode : Unknown Mode\n"); break;
			default: printf("Device Mode : Mode not set \n");
		}

		switch(flashData1->g_ui_device_status)
		{
			case DEVICE_IN_FACTORY_MODE: printf("Device Status : Factory Mode\n"); break;
			case DEVICE_IN_MAINTENANCE_MODE: printf("Device Status : Maintenance Mode\n"); break;
			case DEVICE_IN_NORMAL_MODE: printf("Device Status : User Mode\n"); break;
			case DEVICE_UNKNOWN_MODE: printf("Device Status : Unknown Mode\n"); break;
			default: printf("Device Status : Status not set \n");
		}
		switch(flashData1->g_ui_tracking_status)
		{
			case TRACKING_ENABLED: printf("Tracking Status : Enabled\n"); break;
			case TRACKING_DISABLED: printf("Tracking Status : Disabled\n"); break;
			default: printf("Tracking Status : Status not set \n");
		}
		printf("Number of Stored GPS Locations = %d, GPS Buffe rIndex = %d \n",g_ui_number_of_stored_Locations, g_ui_GpsBufferIndex);
		printf(" Secondary Checksum:  Read = %0X, Calculated = %0X",flashData1->Checksum,calculatedChecksum1);
		//wdt_restart(WDT);
		UpdateFlash();
		return(true);
	}
	else
	{
		printf(" Secondary Checksum:  Read = %0X, Calculated = %0X",flashData1->Checksum,calculatedChecksum1);
		printf("Checksum Failed secondary");
		return(false);
	}
}
Bool ReadConfigDataFromFlash()
{
	struct ConfigData *flashData;
	uint8_t configLength = sizeof(ConfigData_t)-2;
	//uint8_t *configpointer;

	//configpointer = (uint8_t*)flashData;

		ConfigData_t.g_ui_GpsBufferIndex = flashData->g_ui_GpsBufferIndex;
		printf("Name is %s\n", flashData->UserName);
		printf("Number is %s\n", flashData->UserNumber);
		printf("Index ix %d\n", flashData->g_ui_GpsBufferIndex);
		printf("Counter = %d", ConfigData_t.g_ui_GpsBufferIndex);
		
		reset_time = ConfigData_t.g_ui_GpsBufferIndex;
}

void WriteDefaultDataToFlash()
{
	uint8_t Name[] = "Sanjay S N"; 
	uint8_t Number[] = "+919886548455";
	int reset_counter = 5;
	
	memcpy(&(ConfigData_t.UserName[0]),  Name,20);
	memcpy(&(ConfigData_t.UserNumber[0]), Number,15);
	memcpy(&(ConfigData_t.DefaultNumber[0]), Number,15);
	ConfigData_t.g_ui_GpsBufferIndex = reset_counter;
	printf("\nWriting Default data to flash\n");
	//wdt_restart(WDT);
	UpdateFlash();
	//mdelay(500);
	//wdt_restart(WDT);
//	printf("\nWriting Secondary flash!!");

	UpdateFlashSecondary();
	//wdt_restart(WDT);
}

/* Initialize flash: 6 wait states for flash writing. */
Bool InitializeFlash(void)
{
	uint32_t ul_rc;
	ul_rc = flash_init(FLASH_ACCESS_MODE_128, 6);
	if (ul_rc != FLASH_RC_OK) {
		printf("-F- Initialization error %lu\n\r", (UL)ul_rc);
		return 0;
	}
	return true;
}

Bool ReadDeviceID( uint8_t *str )
{
	uint32_t unique_id[4];
	uint32_t ul_rc;
	memset(str,0,40);
	/* Read the unique ID */
	//puts("-I- Reading 128 bits Unique Identifier\r");
	ul_rc = flash_read_unique_id(unique_id, 4);
	if (ul_rc != FLASH_RC_OK) {
		printf("-F- Read the Device Identifier error %lu\n\r", (UL)ul_rc);
		return 0;
	}
	sprintf(str,"%08X-%08X-%08X-%08X",
	(unsigned int)unique_id[0], (unsigned int)unique_id[1],
	(unsigned int)unique_id[2], (unsigned int)unique_id[3]);

	//printf(str);
	return true;
}

Bool WriteFlash( uint32_t ul_page_addr, uint8_t uc_buffer[], uint32_t ul_buffer_length)
{
	uint32_t ul_rc;
	uint32_t ul_idx;
	uint32_t *pul_page = (uint32_t *) ul_page_addr;
	//uint8_t *temp;
/* Unlock page locked pages are not allowed to write*/
//	printf("WriteFlash -I- Unlocking page: 0x%08lx\r\n", (UL)ul_page_addr);
	ul_rc = flash_unlock(ul_page_addr,
			ul_page_addr + ul_buffer_length - 1, 0, 0);
	if (ul_rc != FLASH_RC_OK) {
		printf("-F- Unlock error %lX\n\r", (UL)ul_rc);
		return 0;
	}

	/* The EWP command is not supported for non-8KByte sectors in all devices
	 *  SAM4 series, so an erase command is required before the write operation.
	 */
	ul_rc = flash_erase_sector(ul_page_addr);
	if (ul_rc != FLASH_RC_OK) {
		printf("-F- flash_erase_sector: Flash programming error 0x%lX\n\r", (UL)ul_rc);
		return 0;
	}

	ul_rc = flash_write(ul_page_addr, uc_buffer,
			ul_buffer_length, 0);

	if (ul_rc != FLASH_RC_OK) {
		printf("-F- flash_write: Flash programming error 0x%lX\n\r", (UL)ul_rc);
		return 0;
	}
		

	return true;
}

Bool UpdateFlashSecondary()
{
	Bool flashWriteStatus;
	uint8_t flashWriteAttempt;
	uint32_t ul_test_page_addr1 = PAGE_ADDRESS_SECONDARY;
	uint32_t pageSize = sizeof(ConfigData_t);
	uint32_t pul_test_page =  ul_test_page_addr1;

	flash_unlock(ul_test_page_addr1,  ul_test_page_addr1 + (IFLASH_PAGE_SIZE*1) - 1, 0, 0);

	flashWriteStatus = false;
	flashWriteAttempt = 0;
	uint8_t calchecksum = 0;
	uint8_t configLength = sizeof(ConfigData_t)-2;
	ConfigData_t.g_ui_Known_value = g_ui_Known_value;
	ConfigData_t.g_ui_device_mode = g_ui_device_mode;
	ConfigData_t.g_ui_tracking_status = g_ui_tracking_status;
	ConfigData_t.g_ui_device_status = g_ui_device_status;
	ConfigData_t.g_ui_number_of_stored_Locations = g_ui_number_of_stored_Locations;
	ConfigData_t.g_ui_GpsBufferIndex = g_ui_GpsBufferIndex;
	uint8_t *configpointer;
	configpointer = (uint8_t*) &ConfigData_t;
	for( int i = 0 ; i < configLength; i++)
	{
		calchecksum = calchecksum ^ configpointer[i];
	}
	ConfigData_t.Checksum = calchecksum;
//	printf("\n Secondary :g_ui_number_of_stored_Locations = %d, g_ui_GpsBufferIndex = %d \n",ConfigData_t.g_ui_number_of_stored_Locations,ConfigData_t.g_ui_GpsBufferIndex);

	do{
		//wdt_restart(WDT);
		flashWriteAttempt++;
		if( flashWriteStatus = WriteFlash(ul_test_page_addr1, &ConfigData_t, pageSize) )
		printf("secondary Flash write success\n");
	}while( (flashWriteStatus == false) && ( flashWriteAttempt < 3 ));

	if ( flashWriteAttempt >= 3 )
	{
		printf("Problem with Flash Writing\n");
	}

	//flash_lock(ul_test_page_addr1, ul_test_page_addr1 + (IFLASH_PAGE_SIZE*64) - 1, 0, 0);
	return(flashWriteStatus);
}

Bool UpdateFlash()
{
		Bool flashWriteStatus;
		uint8_t flashWriteAttempt;
		uint32_t ul_test_page_addr = CONFIG_PAGE_ADDRESS;
		uint32_t pageSize = sizeof(ConfigData_t);
		uint32_t pul_test_page =  ul_test_page_addr;

//		flash_unlock(IFLASH_ADDR,  ul_test_page_addr + IFLASH_PAGE_SIZE - 1, 0, 0);
		flash_unlock(ul_test_page_addr,  ul_test_page_addr + (IFLASH_PAGE_SIZE*1) - 1, 0, 0);

		flashWriteStatus = false;
		flashWriteAttempt = 0;
		uint8_t calchecksum = 0;
		uint8_t configLength = sizeof(ConfigData_t)-2;
		ConfigData_t.g_ui_GpsBufferIndex = ConfigData_t.g_ui_GpsBufferIndex;
		uint8_t *configpointer;

		do{
			//wdt_restart(WDT);
			flashWriteAttempt++;
			if( flashWriteStatus = WriteFlash(ul_test_page_addr, &ConfigData_t, pageSize) )
				printf("Flash write success\n");
		}while( (flashWriteStatus == false) && ( flashWriteAttempt < 3 ));

		if ( flashWriteAttempt >= 3 )
		{
			printf("Problem with Flash Writing\n");
		}
//		flash_lock(ul_test_page_addr, ul_test_page_addr + (IFLASH_PAGE_SIZE*260) - 1, 0, 0);
	return(flashWriteStatus);
}

Bool ReadGpsDataFromFlash()
{
	uint32_t ul_rc;
	GpsLocationData_type *flashLocDataPtr;
	Bool flashWriteStatus;
	uint8_t flashWriteAttempt, i;
	uint32_t ul_GpsData_page_addr = GPS_DATA_PAGE_ADDRESS;
	uint32_t GpspageSize = sizeof(GpsLocationData_t);
	uint32_t pul_GpsData_page =  ul_GpsData_page_addr;
	if( g_ui_number_of_stored_Locations < 1 )
	return false;
	ul_rc = flash_unlock(ul_GpsData_page_addr,
	ul_GpsData_page_addr + (IFLASH_PAGE_SIZE*6) - 1, 0, 0);
	if (ul_rc != FLASH_RC_OK) {
		printf("ReadGpsDataFromFlash: -F- Unlock error %lX\n\r", (UL)ul_rc);
		return 0;
	}

	flashLocDataPtr =  (GpsLocationData_type*) pul_GpsData_page;
	
	memcpy(GpsLocationData_t,flashLocDataPtr,sizeof(GpsLocationData_t));

	return(true);
}

//Writes factory details(URl, Phone num) into the flash
Bool WriteFactoryConfigdatatoFlash(){

	uint8_t Number[] = "+9663947730";
	uint8_t url[]="www.snes.com";
	Bool flashWriteStatus;
	uint8_t flashWriteAttempt;
	uint32_t ul_test_page_addr = FACTORY_CONFIG_ADDRESS;
	uint32_t Datasize = sizeof(factoryconfigdata);
	uint32_t pul_test_page =  ul_test_page_addr;
	/*printf("ul test page address %x\n", ul_test_page_addr);*/
	flash_unlock(ul_test_page_addr,  ul_test_page_addr + (IFLASH_PAGE_SIZE*1) - 1, 0, 0);
	flashWriteStatus = false;
	flashWriteAttempt = 0;
	memcpy(&(factoryconfigdata.url[0]),  url,30);
	memcpy(&(factoryconfigdata.phnumber[0]), Number,20);
	/*	factoryconfigdata.checksum=0;*/
	uint8_t *factoryconfigpointer;
	uint8_t factconfiglength=sizeof(factoryconfigdata)-2;
	factoryconfigpointer = (uint8_t*) &factoryconfigdata;
	uint8_t crcinwrite=0;
	for(int i=0;i<factconfiglength;i++)
	{
		crcinwrite=crcinwrite ^ factoryconfigpointer[i];
			//printf( "url = %s \n",crcinwrite);
	}
	
	factoryconfigdata.checksum=crcinwrite;
	do{
		//wdt_restart(WDT);
		flashWriteAttempt++;
		if( flashWriteStatus = WriteFlash(ul_test_page_addr,&factoryconfigdata, Datasize) )
		printf("Flash write success\n");
		//SetDeviceToNormalMode();
	}while( (flashWriteStatus == false) && ( flashWriteAttempt < 3 ));

	if ( flashWriteAttempt >= 3 )
	{
		printf("Problem with Flash Writing\n");
		SetDeviceToFactoryMode();
	}
	//		flash_lock(ul_test_page_addr, ul_test_page_addr + (IFLASH_PAGE_SIZE*260) - 1, 0, 0);
	return(flashWriteStatus);
}

//Reads factory details(URL & Phone num) from the flash
bool ReadFactoryConfigDataFromFlash()
{
	struct FACTORYMODECONFIGDATA *flashData, RetData;
	struct FACTORYMODECONFIGDATA s1;
	uint8_t configrecivedLength = sizeof(factoryconfigdata)-1;
	uint8_t *factoryconfigrecievepointer;
	uint32_t ul_test_page_addr = FACTORY_CONFIG_ADDRESS;
	uint32_t pul_test_page =  ul_test_page_addr;

	flashData=(struct FACTORYMODECONFIGDATA*)pul_test_page;
	factoryconfigrecievepointer = (uint8_t*) flashData;
	flash_unlock(ul_test_page_addr,  ul_test_page_addr + (IFLASH_PAGE_SIZE*1) - 1, 0, 0);
	uint8_t crcinread = 0;
	for(int i=0;i<configrecivedLength;i++)
	{
		crcinread=crcinread ^ factoryconfigrecievepointer[i];
		delay_ms(100);
		
	}
		//printf( "crc in read mode  = %d \n",crcinread);
	if(crcinread==flashData->checksum)
	{
		//printf( "url = %s \n",flashData->url);
		memcpy(&factoryconfigdata.url[0],&flashData->url[0],30);
		memcpy(&RetData.url[0],&flashData->url[0],30);
		memcpy(&s1.url[0],&flashData->url[0],30);
		//printf("url in ret = %s \n",RetData.url);
		//printf("Number = %s \n",flashData->phnumber);
		memcpy(&factoryconfigdata.phnumber[0],&flashData->phnumber[0],20);
		memcpy(&s1.phnumber[0],&flashData->phnumber[0],20);
		memcpy(&RetData.phnumber[0],&flashData->phnumber[0],20);
		//printf("num in ret = %s \n",RetData.phnumber);
		memcpy(&RetData.checksum, &flashData->checksum,10);
		return 1;
	}
	else
	{
		
		return 0;
	}

}

//Writes counter value for reset inside flash (Used in failed mode)
bool writeresetcountintoflash( uint8_t counter){
	bool writestatus;
	uint8_t flashwriteattempt;
	uint32_t ul_pageaddr= RESET_COUNTER_PAGEADDRESS;
	//uint8_t counter=2;
	uint8_t  datasize=sizeof(counter);
	uint32_t pul_test_page =  ul_pageaddr;
	
	flash_unlock(ul_pageaddr , ul_pageaddr + (IFLASH_PAGE_SIZE*1)-1 ,0 ,0);
	
	writestatus=false;
	flashwriteattempt=0;
	
	do{
		//wdt_restart(WDT);
		flashwriteattempt++;
		if( writestatus = WriteFlash(ul_pageaddr,&counter, datasize) )
		printf(" \r");
	}while( (writestatus == false) && ( flashwriteattempt < 3 ));

	if ( flashwriteattempt >= 3 )
	{
		printf("Problem with Flash Writing\n");
		//SetDeviceToFactoryMode();
	}
	//		flash_lock(ul_test_page_addr, ul_test_page_addr + (IFLASH_PAGE_SIZE*260) - 1, 0, 0);
	return(writestatus);
}

//Reads counter value for reset from flash
uint8_t ReadcounterFromFlash()
{
	
	uint8_t * flashData;
	uint32_t ul_test_page_addr = RESET_COUNTER_PAGEADDRESS;
	uint32_t pul_test_page =  ul_test_page_addr;
	flash_unlock(ul_test_page_addr , ul_test_page_addr + (IFLASH_PAGE_SIZE*1)-1 ,0 ,0);
	//flashData = ( int *) ul_test_page_addr;
	flashData =  ul_test_page_addr;
	printf("\tthe value at that address is %d\n", *flashData);
	
	return *flashData;
}

//Fetches the factory details(URL & phone num) from flash

struct FACTORYMODECONFIGDATA GetFactoryDetails()
{
	struct FACTORYMODECONFIGDATA *flashData;
	struct FACTORYMODECONFIGDATA factory_config_data;
	uint8_t configrecivedLength = sizeof(factoryconfigdata)-1;
	uint8_t *factoryconfigrecievepointer;
	uint32_t ul_test_page_addr = FACTORY_CONFIG_ADDRESS;
	uint32_t pul_test_page =  ul_test_page_addr;

	flashData=(struct FACTORYMODECONFIGDATA*)pul_test_page;
	factoryconfigrecievepointer = (uint8_t*) flashData;
	flash_unlock(ul_test_page_addr,  ul_test_page_addr + (IFLASH_PAGE_SIZE*1) - 1, 0, 0);
	uint8_t crcinread = 0;
	for(int i=0;i<configrecivedLength;i++)
	{
		crcinread=crcinread ^ factoryconfigrecievepointer[i];
		delay_ms(100);
		
	}
		//printf( "crc in read mode  = %d \n",crcinread);
	if(crcinread==flashData->checksum)
	{
		memcpy(&factory_config_data.url, &flashData->url,25);
		memcpy(&factory_config_data.phnumber, &flashData->phnumber,20);
		memcpy(&factory_config_data.checksum, &flashData->checksum,10);
		return factory_config_data;
	}
	else
	{
		//printf("recieved data is not valid, recive data crc and transmitdata crc are %d, %d\n",crcinread,flashData->checksum);
	}
}

bool InitilizeQSPIFlash()
{
	
	//	uint8_t *memory = (uint8_t *)QSPIMEM_ADDR;
	enum status_code status = STATUS_OK;
	pmc_enable_periph_clk(ID_QSPI);
	/* QSPI memory mode configure */
	status = s25fl1xx_initialize(g_qspid.qspi_hw, &mode_config, 1);
	/* Enable quad mode */
	s25fl1xx_set_quad_mode(&g_qspid, 0);
	s25fl1xx_enter_4byte_Address_mode(&g_qspid);
	if (status == STATUS_OK)
	{
		return 1;
	}
	else
	{
		return 0;
	}
	
}
	

 Bool CheckQSPIFlash()
{
	uint32_t QspiAddress1,QspiAddress2,QspiAddress3,sta;
	Bool Status = true;
	uint8_t res = 0;
	uint32_t id;
	uint32_t x;
	uint8_t rx_data;
	Bool returnstatus=false;
	s25fl1xx_read(&g_qspid, rx_data, 20, 0);
	sta = s25fl1xx_read_jedec_id(&g_qspid);
	//printf("\n JEDEC_ID = 0x%08X \n",sta);
	QspiAddress1 =  0x000100;
	s25fl1xx_read(&g_qspid, rx_data, 64, QspiAddress1);
	res = s25fl1xx_erase_64k_block(&g_qspid, QspiAddress1);
	if (res != 0 ) 
	{
		printf("\n Erase block Failure!!!!!\r\n");
	}
	
	struct User_Registration_data Write_Data, Read_data;
	Write_Data.First_name = "Harry";
	Write_Data.Last_name = "HP";
	Write_Data.Mobile_no = "7676890629";
	Write_Data.Email_ID = "harish.mbhv@gmail.com";
	Write_Data.checksum=0;
	
	//printf("\rWriting to the beginning of memory 0x8%07X\r\n", QspiAddress1 );
	s25fl1xx_write(&g_qspid, (uint32_t *)&Write_Data,  sizeof(Write_Data), QspiAddress1, 0);
	printf(" Loc = 0x8%07X\n First Name: %s\n Last Name:%s\n Mobile Number:%s\n Email_ID:%s\n\n", QspiAddress1, Write_Data.First_name, Write_Data.Last_name, Write_Data.Mobile_no, Write_Data.Email_ID);
	memset(&Write_Data,0,sizeof(Write_Data));
	delay_ms(1000);
	//printf("\rReading from the beginning of the memory 0x8%07X\n", QspiAddress1 );
	memset(&Read_data,0,sizeof(Read_data));
	s25fl1xx_read(&g_qspid, &Read_data, sizeof(Read_data), QspiAddress1);
	printf(" Loc = 0x8%07X\n  First Name: %s\n Last Name:%s\n Mobile Number:%s\n Email_ID:%s\n\n", QspiAddress1, Read_data.First_name, Read_data.Last_name, Read_data.Mobile_no, Read_data.Email_ID);

			
		
}
//************wrinting userdata into qspi flash***************//
//Returns 0 if successful, otherwise returns ERROR_PROGRAM if unsuccessful

uint8_t writeuserdetailsintoqspi(uint32_t adress,struct User_Registration_data Data){
	uint8_t rx_data;
	uint32_t sta;
	uint8_t res = 0;
	uint8_t writecrc=0;
	uint8_t status;
	//uint32_t *pointer;
	uint32_t datalength=(sizeof(&Data));
	//uint32_t pointer=(uint32_t*)Write_Data;
	struct User_Registration_data *pointer;
	uint32_t *factoryconfigrecievepointer;
	factoryconfigrecievepointer = &Data;

	sta = s25fl1xx_read_jedec_id(&g_qspid);
	s25fl1xx_read(&g_qspid, rx_data, 64, adress);
	res = s25fl1xx_erase_64k_block(&g_qspid, adress);
	if (res != 0 )
	{
		printf("\n Erase block Failure!!!!!\r\n");
	}
	for (int i=0;i<=datalength;i++){
		writecrc=writecrc ^ factoryconfigrecievepointer[i];

	}
	
	Data.checksum=writecrc;
	
	
	status = s25fl1xx_write(&g_qspid, (uint32_t *)&Data,  sizeof(Data), adress, 0);
	 return status;
}

//*******************reading userdata from qspi**********************************//
// Returns 0 if Success else return 1
bool Readuserdetailsfromqspi(uint32_t address, struct User_Registration_data *readpointer)
{
	struct UserRegistration_data Read_data;
	uint8_t readcrc=0;
	uint32_t validcrc;
	uint32_t *factoryconfigrecievereadpointer;
	
	memset(&Read_data,0,sizeof(Read_data));
	s25fl1xx_read(&g_qspid, &Read_data, sizeof(Read_data), address);
	//copy the data to returning structure and returns only on successful crc validation.
	memcpy(readpointer,&Read_data,sizeof(Read_data));
	validcrc=Read_data.checksum;
	//making zero to calculate only data crc
	Read_data.checksum = 0;
	factoryconfigrecievereadpointer = &Read_data;
	uint8_t readdatalength=sizeof(&Read_data);
	for (int i=0;i<=readdatalength;i++){
		readcrc=readcrc^factoryconfigrecievereadpointer[i];
	}
	delay_ms(1000);
	if(validcrc==readcrc){
		//printf(" Loc = 0x8%07X\n  First Name: %s\n Last Name:%s\n Mobile Number:%s\n Email_ID:%s\n\n", address, Read_data.First_name, Read_data.Last_name, Read_data.Mobile_no, Read_data.Email_ID);
		return 0;
	}
	else
	{
		//printf("read data is not valid");
		return 1;
	}
	
}

//***************writing gps data into flash*********************//
//Returns 0 if successful, otherwise returns ERROR_PROGRAM if unsuccessful
uint8_t writegpsdataintoqspi(uint32_t address,struct LocationData Write_Data){
	uint8_t rx_data, status;
	uint32_t sta;
	uint8_t res = 0;
	uint8_t *writegpspointer;
	writegpspointer=(uint8_t*)&Write_Data;
	uint8_t writegpsdatacrc=0;
	uint32_t datasize =sizeof(Write_Data);
	//uint32_t datasize=sizeof(&Write_Data);
	sta = s25fl1xx_read_jedec_id(&g_qspid);
	printf("\n JEDEC_ID = 0x%08X \n",sta);
	s25fl1xx_read(&g_qspid, rx_data, 64, address);
	res = s25fl1xx_erase_64k_block(&g_qspid, address);
	if (res != 0 )
	{
		printf("\n Erase block Failure!!!!!\r\n");
	}
	
	for (int i=0;i<datasize;i++){
		writegpsdatacrc=writegpsdatacrc ^ writegpspointer[i];
		printf("writegps %d,%d\n",writegpspointer[i],writegpsdatacrc);
	}

	delay_ms(1000);
	printf(" writecrc is %d\n",writegpsdatacrc);
	Write_Data.checksum=writegpsdatacrc;
	
	
	printf("crc value in writing gps data into qspi is %d\n",writegpsdatacrc);
	printf("\rWriting gps data into qspi memory 0x8%07X\r\n", address );
	status = s25fl1xx_write(&g_qspid, (uint32_t *)&Write_Data,  sizeof(Write_Data), address, 0);
	printf(" Loc = 0x8%07X  LAT %+013.8f %+013.8f %+06d %02d-%02d-%04d %02d:%02d:%02d.%03d\n",address ,Write_Data.fLatitude,
	Write_Data.fLongitude, Write_Data.fAltitude,
	Write_Data.u8Day,
	Write_Data.u8Month,
	Write_Data.iYear,
	Write_Data.u8Hour,Write_Data.u8Minute,Write_Data.u8Second,
	Write_Data.iThousandthsSecond);
	
	return status;
}
//*************************readgpsdata from qspi*************************//

bool Readgpsdatafromqspi(uint32_t address, struct LocationData *readpointer){
	struct LocationData Read_data;
	uint8_t *readgpspointer;
	uint8_t readgpsdatacrc=0;
	uint8_t validwritecrc=0;
	memset(&Read_data,0,sizeof(Read_data));
	
	s25fl1xx_read(&g_qspid, &Read_data, sizeof(Read_data), address);
	
	memcpy(readpointer,&Read_data,sizeof(Read_data));
	validwritecrc=Read_data.checksum;

	Read_data.checksum=0;
	uint32_t datasize=sizeof(Read_data);
	readgpspointer=(uint8_t*)&Read_data;
	for (int i=0;i<datasize;i++)
	{
		readgpsdatacrc=readgpsdatacrc^readgpspointer[i];
		printf("readgps %d,%d\n",readgpspointer[i],readgpsdatacrc);
	}
	delay_ms(1000);
	printf("crc value while reading gps data into qspi is %d\n",readgpsdatacrc);
	

	if(validwritecrc==readgpsdatacrc){
		return 0;
	}
	else{
		return 1;
	}

}

	
	




