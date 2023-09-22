/*
 * SMS_Comm.h
 *
 * Created: 27-06-2022 15:40:41
 *  Author: HarishN
 */ 
#ifndef SMS_COMM_H_
#define SMS_COMM_H_
#define SMS_Buffer_size 30
#include <asf.h>
#include <conf_board.h>
#include <stdio.h>
#include <string.h>
#define Max_sms_memorysize 9
#define MIN_MSGID_INDEX 12
#define MAX_MSGID_INDEX 13
#define MIN_MOBILENUM_INDEX 22
#define MAX_MOBILENUM_INDEX 34

enum SMS_Cmd_Resp_State {S_SMS_IDLE = 0,
	 S_EnterNumber, S_EnterText, S_SMS_Send_SUCCESS, S_SMS_ERROR, S_SMS_Delete_SUCCESS, S_SMS_Recieve_Read_SUCCESS,
	 S_ReadAllMsg ,S_Turnonpdumode, S_deleteallmessages, S_Read_sms_data, S_Delete_all_msgs
};

enum SMS_Cmd_Resp_Event {E_SMS_COMMAND = 0, E_SMS_MESSAGE,
	E_SMS_RESPONSE, E_SMS_MESSAGE_RESPONSE
};

struct Sms_Commands{
	uint8_t Set_op_mode_cmd[SMS_Buffer_size];
	uint8_t Set_mobile_num_cmd[SMS_Buffer_size];
	uint8_t SMS_text_cmd[SMS_Buffer_size];
	uint8_t Read_sms_cmd[SMS_Buffer_size];
	uint8_t Receive_sms_cmd[SMS_Buffer_size];
	uint8_t Read, write, Send, _sms_cmd[SMS_Buffer_size];
	uint8_t msgID;
	};

enum SMS_Cmd_Resp_State get_SMS_State();
enum SMS_Cmd_Resp_Event get_SMS_Event();
bool SM_SMS_Send();
bool ReadSMSResponce( uint8_t *data, uint32_t Timeout );
bool IsSpaceAvailableInSmsCommandQueue();
bool InitialiseSmsQueue();
bool ReadSMSAsyncResponce( uint8_t *data, uint32_t Timeout );
extern Bool g_b_IsGSMModemOkay;
Bool SMSReceiveHandler();
char* substr(const char *src, int m, int n);
bool SMS_Recieve();

struct Sms_Commands SNES_SMS;
#endif /* SMS_COMM_H_ */  