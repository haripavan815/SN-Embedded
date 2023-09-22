/*
* Created: 19-11-2020 16:01:22
*  Author: Sanjay S N
*/

#ifndef VTS_FLASH_H_
#define VTS_FLASH_H_
//#include "function.h"
#include <asf.h>
#include <conf_board.h>
#include "flash_efc.h"
#include "ASF/common/components/memory/qspi_flash/s25fl1xx/s25fl1xx.h"

// struct User_Registration_data
// {
// 	uint32_t First_name;
// 	uint32_t Last_name;
// 	uint32_t Mobile_no;
// 	uint32_t Email_ID;
// }Data;
	
	
Bool WriteLocationToFlash(void);
void decrement_GpsBufferIndex(void);
void increment_GpsBufferIndex(void);
Bool ReadConfigDataFromFlashSecondary(void);
Bool ReadConfigDataFromFlash(void);
void WriteDefaultDataToFlash(void);
Bool InitializeFlash(void);
Bool ReadDeviceID( uint8_t *str );
Bool WriteFlash( uint32_t ul_page_addr, uint8_t uc_buffer[], uint32_t ul_buffer_length);
Bool UpdateFlashSecondary(void);
Bool UpdateFlash(void);
Bool ReadGpsDataFromFlash(void);
Bool WriteFactoryConfigdatatoFlash(void);
bool ReadFactoryConfigDataFromFlash(void);
void WriteDefaultDataToFlash(void);
bool writeresetcountintoflash( uint8_t counter);
uint8_t ReadcounterFromFlash();
struct FACTORYMODECONFIGDATA GetFactoryDetails();
bool InitilizeQSPIFlash(void);
Bool CheckQSPIFlash();
uint8_t writeuserdetailsintoqspi(uint32_t adress,struct User_Registration_data Write_Data);
bool Readuserdetailsfromqspi(uint32_t address, struct User_Registration_data *readpointer);
uint8_t writegpsdataintoqspi(uint32_t address,struct LocationData Write_Data);
bool Readgpsdatafromqspi(uint32_t address, struct LocationData *readpointer);

volatile int reset_time;
#endif /* VTS_FLASH_H_ */