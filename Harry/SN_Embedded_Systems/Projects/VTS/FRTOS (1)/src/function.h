#ifndef SYSTEM_FUNCTION_H_INCLUDED
#define SYSTEM_FUNCTION_H_INCLUDED
#include <stdio.h>
#include "asf.h"

#define BUFFER_LEN 1000
#define PAGE_ADDRESS_SECONDARY (IFLASH_ADDR + IFLASH_SIZE - (IFLASH_PAGE_SIZE * 250))
#define CONFIG_PAGE_ADDRESS (IFLASH_ADDR + IFLASH_SIZE - (IFLASH_PAGE_SIZE * 260))
#define GPS_DATA_PAGE_ADDRESS (IFLASH_ADDR + IFLASH_SIZE - (IFLASH_PAGE_SIZE * 560))
#define FACTORY_CONFIG_ADDRESS (IFLASH_ADDR + IFLASH_SIZE - (IFLASH_PAGE_SIZE * 8))		// STARTING FROM OX5FF000
#define RESET_COUNTER_PAGEADDRESS (IFLASH_ADDR + IFLASH_SIZE - (IFLASH_PAGE_SIZE * 8)) - 1
//#define USER_CONGIGDATA_QSPIADDRESS (QSPI_MEM_SIZE - (QSPI_PAGE_SIZE * 32)) //STARTING FROM 0X3FFE000
#define USER_CONFIGDATA_QSPIADDRESS (0X3FFE000u)
#define GPSDATA_QSPIADDRESS    (0X0000000u)
#define RSTCNTVALUE 0
#define MAX_RST_CNT 3
#define SET_APP_MOBILE_NUMBER (0x10)
#define APP_MOBILE_NUMBER_STORED (0x35)
#define GET_APP_NUMBER (0x36)
#define SET_USER_NAME (0x30)
#define CHECK_COMMUNICATION (0x31)
#define IS_GPS_OK (0x32)
#define PERFORM_SELF_TEST (0x66)
#define GET_TRACKING_STATUS (0x34)
#define EVERY_THING_FINE	(0x37)

#define COMMUNICATION_ACK (0xAA)
#define DEVICE_UNKNOWN_MODE (0x9B)
#define DEVICE_IN_FACTORY_MODE (0x11)
#define DEVICE_IN_MAINTENANCE_MODE (0x22)
#define DEVICE_IN_NORMAL_MODE (0x33)
#define DEVICE_IN_FAILED_MODE (0X77)
#define DEVICE_IN_HALT_MODE (0X88)
#define DEVICE_ON_TIME (0x12)
#define GET_DEVICE_ON_TIME (0x12)
#define GPS_STATUS (0x13)
#define GPS_IS_OK (0x13)
#define MODE_CHANGED_TO_FACTORY (0xEE)
#define CHANGE_TO_FACTORY_MODE (0xEE)
#define MODE_CHANGED_TO_MAINTENANCE (0xDD)
#define CHANGE_TO_MAINTENANCE_MODE (0xDD)
#define MODE_CHANGED_TO_USER (0xCC)
#define CHANGE_TO_USER_MODE (0xCC)
#define GET_NO_OF_STORED_LOC (0x14)

#define SELF_TEST_RESULT_STATUS (0x15)

#define SENDING_ALL_PREVIOUS_LOC  (0x16)
#define GET_ALL_PREVIOUS_LOC (0x16)
#define SENDING_PREVIOUS_FIVE_LOC (0x17)
#define GET_PREVIOUS_FIVE_LOC (0x17)
#define SENDING_PREVIOUS_TEN_LOC (0x18)
#define GET_PREVIOUS_TEN_LOC (0x18)
#define GET_SIM_STATUS (0x19)
#define SIM_STATUS_ACTIVE (0x49)
#define APP_NUMBER_STORED (0x1A)
#define DISABLE_TRACKING (0x44)
#define TRACKING_DISABLED (0x44)
#define ENABLE_TRACKING (0xBB)
#define TRACKING_ENABLED (0xBB)
#define TRACKING_STATUS (0x20)
#define WAIT_FOR_GPS_INFO (0x21)
#define GET_CURRENT_LOCATION (0xBA)
#define GET_DEVICE_ID (0x23)

#define END_OF_MESSAGE_LENGTH (3) 
#define END_OF_MESSAGE_DELAY (306)
#define MAX_GSM_COMMAND_DELAY (300)
#define SMS_COMMAND_DELAY (305)

typedef unsigned long UL;

struct FACTORYMODECONFIGDATA {
	uint8_t url[25];
	uint8_t phnumber[20];
	uint8_t checksum;
} factoryconfigdata;

struct User_Registration_data
{
	uint32_t First_name;
	uint32_t Last_name;
	uint32_t Mobile_no;
	uint32_t Email_ID;
	uint32_t checksum;
}Data;

struct UserRegistration_data
{
	uint32_t First_name;
	uint32_t Last_name;
	uint32_t Mobile_no;
	uint32_t Email_ID;
	uint8_t checksum;
}userd_ata;

struct ConfigData {
	uint8_t g_ui_Known_value;
	uint8_t g_ui_device_mode;
	uint8_t g_ui_device_status;
	uint8_t g_ui_tracking_status;
	uint8_t UserName[40];
	uint8_t UserNumber[20];
	uint8_t DefaultNumber[20];
	uint8_t g_ui_number_of_stored_Locations;
	uint8_t g_ui_GpsBufferIndex;
	int Checksum;
} ConfigData_t;

typedef struct GPSLocationData {
	double fLatitude;
	double fLongitude;
	uint32_t fAltitude;
	uint16_t iYear;
	uint8_t u8Month;
	uint8_t u8Day;
	uint8_t u8Hour;
	uint8_t u8Minute;
	uint8_t u8Second;
	uint16_t	iThousandthsSecond;
} GpsLocationData_type;

typedef struct LocationData {
	double fLatitude;
	//uint8_t d1;
	double fLongitude;
	//uint8_t d2;
	uint32_t fAltitude;
	//uint8_t d3;
	uint16_t iYear;
	uint8_t u8Month;
	uint8_t u8Day;
	uint8_t u8Hour;
	uint8_t u8Minute;
	uint8_t u8Second;
	uint16_t	iThousandthsSecond;
	uint8_t checksum;
} LocationData_type;


struct FACTORY_MODECONFIGDATA {
	uint8_t url[25];
	uint8_t phnumber[20];
	uint8_t checksum;
} factorydata;
//enum COMMAND
//{
//	SET_APP_MOBILE=1,
//	CHECKING_COMMUNICATION,
//	IS_GPS_OK,
//	PERFORM_SELF_TEST,
//	UPDATE_GPS_INFO,
//	GET_TRACKING_STATUS,
//	GET_APP_NUMBER,
//	GET_SIM_STATUS,
//	GET_DEVICE_ON_TIME,
//	GET_NO_OF_STORED_LOC,
//	GET_PREVIOUS_FIVE_LOC,
//	GET_PREVIOUS_TEN_LOC,
//	GET_ALL_PREVIOUS_LOC,
//	ENABLE_TRACKING,
//	DISABLE_TRACKING,
//	CHANGE_TO_FACTORY_MODE,
//	CHANGE_TO_MAINTENANCE_MODE,
//	CHANGE_TO_USER_MODE
//};

	uint32_t GetTime(void);
	//void mdelay(uint32_t ul_dly_ticks);
	void RTOS_Init();
	void Run_RTOS_Application();
	uint8_t CheckBoardHealth();
// 	uint8_t GetDeviceMode(void);
// 	Bool GsmCommand(uint8_t* Cmd, uint8_t Length, uint32_t delayMs);
// 	Bool IsGsmModemOkay(void);
// 	void decrement_GpsBufferIndex(void);
// 	void increment_GpsBufferIndex(void);
// 	
// 	Bool CheckGPSStatus(void);
// 	void DisableTraking(void);
// 	void EnableTraking(void);
// 	void GetLatestGPSInfo(void);
// 	Bool PerformSelfTest(void);
// 	Bool SendAllPreviousStoredLocations(void);
// 	void SendLatestGPSInfo(void);
// 	Bool SendPreviousFiveStoredLocations(void);
// 	Bool SendPreviousTenStoredLocations(void);
// 
// 	void SendResponse(uint8_t res );
// 	void SetDeviceToFactoryMode(void);
// 	void SetDeviceToMaintenanceMode(void);
// 	void SetDeviceToNormalMode(void);
// 	
// 	void CommandResponseUserMode(void);
// 	void CommandResponseMaintenanceMode(void);
// 	void CommandResponseFactoryMode(void);
// 	
// 	void IntialiseNVRAMforStorage(void);
// 	void IntialiseCommandResponse(void);
// 	Bool IntialiseGPScommunication(void);
// 	void CommandResponseUSBMode(void);
// 	
// 	void ReadCommandAndSendResponse(void);
// 	void UpdateTrackingStatus(void);
// 	void ReadGPSLocation(void);
// 	void StoreGPSLocation(void);
// 	void SendGPSLocation(void);
// 	
// 	uint8_t GetReceivedCommand(void);
 	void get_reset_reason(void);
// 	Bool ReadSMS(uint8_t *str);
// 	Bool PrepareForSendSMS(void);
// 	Bool SendSMS(uint8_t *Message, uint8_t Length);
// 	uint8_t CheckSMSAvailable(void);
// 	void ReplaceSpaceWithDollar(uint8_t *InMessage , uint8_t *OutMessage);
// 	Bool UpdateFlash(void);
// 	Bool UpdateFlashSecondary(void);
// //	void UpdateTrackingStatus(void);
// //	void UpdateDeviceMode(void);
// 	Bool StoreMobileAppNumber(void);
// 	Bool StoreUserName(void);
//// 	Bool WriteLocationToFlash(void);	
 //	Bool ReadConfigDataFromFlashSecondary(void);
 ///	Bool ReadConfigDataFromFlash(void);
 //	Bool ReadGpsDataFromFlash(void);
// 	Bool IntialiseGSMModem(void);

// 	void SendHeartbeat(void);
// 	Bool ReadDeviceID( uint8_t *str );
// 	void DecodeReceivedUSBMessage(void);
// 	uint8_t GetUSBReceivedCommand(void);
// 	void printGpsStoreddata(int count);
#endif /* SYSTEM_FUNCTION_H_INCLUDED */