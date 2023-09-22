/* GPS Communication */
#ifndef GPS_COMM_H_
#define GPS_COMM_H_

#include <asf.h>
#include <conf_board.h>
#define LTE_buffer_size 50
#define GPS_BUFFER_LENGTH 250
uint8_t fault_indicator_status[100];
struct LTE_commands{
	uint8_t Is_LTE_OK[LTE_buffer_size];
	uint8_t Error_message_format[LTE_buffer_size];
	uint8_t Is_SIM_Ready[LTE_buffer_size];
	uint8_t Is_Nw_Reg_Ready[LTE_buffer_size];
	uint8_t Is_GPS_Nw_Reg_Ready[LTE_buffer_size];
	uint8_t Turn_On_GNSS_Engine[LTE_buffer_size];
	uint8_t Turn_Off_GNSS_Engine[LTE_buffer_size];
	uint8_t Configure_Gps_debug_Port[LTE_buffer_size];
	uint8_t Config_Gps_Nmea[LTE_buffer_size];
	uint8_t Config_GPS_AutoOutput[LTE_buffer_size];
	uint8_t Save_AT_Settings_To_Nvm[LTE_buffer_size];
	uint8_t Turn_Off_Echo[LTE_buffer_size];
	uint8_t GPS_Read[GPS_BUFFER_LENGTH];
	uint8_t Enable_Incoming_Messages[LTE_buffer_size];
	uint8_t msgautodelete_aftermaxmemorystorage[LTE_buffer_size];
	uint8_t Set_op_mode_cmd[LTE_buffer_size];
	};
enum LTE_Cmd_Resp_State {S_IDLE = 0, S_IsLTE_OK, 
	S_ErrorMessageFormat,
	S_IsSimReady, S_IsNwRegReady, S_IsGPSNwRegReady, S_Init_SUCCESS, S_ERROR,
	S_TurnOffGNSSEngine, S_TurnOnGNSSEngine, S_ConfigGpsDebugPort, S_ConfigGPSNmea, S_ConfigGPSAutoOutput,
	S_SaveATSettingsToNvm, S_TurnOffEcho, S_EnableIncomingMessages,
	S_GPSRead, S_MsgAutoDelete, S_TurnOnTextMode
};

enum LTE_Cmd_Resp_Event {E_COMMAND = 0,
	E_RESPONSE
};

enum LTE_Cmd_Resp_State get_LTE_InitState();
enum LTE_Cmd_Resp_Event get_LTE_InitEvent();

bool InitializeInitQueue();
bool InitialiseGpsQueue();

bool IsSpaceAvailableInInitializeCommandQueue();
bool IsSpaceAvailableInGpsCommandQueue();

bool ReadInitializeResponce( uint8_t *data, uint32_t Timeout );
bool ReadGPSResponce( uint8_t *data, uint32_t Timeout );

bool SM_LTE_Init();
struct LTE_commands SNES_LTE;
int parse_comma_delimited_GPS_str(char *string, char **fields, int max_fields);
int Gpsnmea0183_checksum(char *nmea_data);
bool GetGPSLocation( uint8_t** gpsLoc );
#endif /* GPS_COMM_H_ */  