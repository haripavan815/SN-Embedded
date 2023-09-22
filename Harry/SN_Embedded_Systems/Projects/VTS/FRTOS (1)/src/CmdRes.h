
#ifndef VTS_CMD_RES_H_
#define VTS_CMD_RES_H_
#include "function.h"
#include "asf.h"
#include "led.h"
#include "IMU_Module/VTS_IMU.h"
extern uint8_t CommandReceived;
extern uint8_t CommandParamater[40];
extern Bool IsfiveLocSentCompleted;
extern Bool IstenLocSentCompleted;
extern Bool IsAllLocSentCompleted;
extern volatile Bool SOSDetected;
extern bool IsGpsUartInitialized;
extern bool IsBoardHealthSuccess; 
//extern volatile uint32_t g_ul_ms_ticks;
extern volatile uint8_t g_ui_device_mode;
extern volatile uint8_t g_ui_device_status;
extern volatile uint8_t g_ui_tracking_status;
extern volatile Bool g_b_sim_status;
extern volatile uint8_t g_ui_number_of_stored_Locations;
extern uint8_t string1[BUFFER_LEN];
extern uint8_t string2[BUFFER_LEN];
extern uint8_t USBMessageReceived[BUFFER_LEN];
extern uint8_t USBMessageLength;
extern Bool IsUSBMessageReceived;
extern uint8_t USBCommandReceived;
extern uint8_t USBCommandParamater[40];
extern uint8_t g_ui_Known_value;
extern uint8_t SNSDeviceID[40];
extern volatile uint8_t UsbVbusDetected;
extern volatile int g_ui_GpsBufferIndex;
extern Bool g_b_IsGSMModemOkay;
extern GpsLocationData_type GpsLocationData_t[100];
extern GpsLocationData_type LatestGpsData,GpsDataRecord;
extern  struct factorydata;

void CommandResponseUserMode(void);
void CommandResponseMaintenanceMode(void);
void CommandResponseFactoryMode(void);
void CommandResponseUSBMode(void);
void ReadCommandAndSendResponse(void);
uint8_t GetDeviceMode(void);
uint8_t GetReceivedCommand(void);
uint8_t GetUSBReceivedCommand(void);
void SendResponse(uint8_t Response);
Bool CheckGPSStatus(void);
void DisableTraking(void);
void EnableTraking(void);
void SetDeviceToUnknownMode(void);
void SetDeviceToFactoryMode(void);
void SetDeviceToMaintenanceMode(void);
void SetDeviceToNormalMode(void);
void SetDeviceToFailedMode(void);
void SetDeviceToHaltMode(void);
Bool PerformSelfTest(void);
Bool SendAllPreviousStoredLocations(void);
void SendLatestGPSInfo(void);
Bool SendPreviousFiveStoredLocations(void);
Bool SendPreviousTenStoredLocations(void);
Bool StoreUserName(void);
Bool StoreMobileAppNumber(void);

#endif /* VTS_CMD_RES_H_ */
