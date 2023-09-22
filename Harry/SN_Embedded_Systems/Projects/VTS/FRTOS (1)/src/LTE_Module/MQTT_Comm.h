/* MQTT Communication */
#ifndef MQTT_COMM_H_
#define MQTT_COMM_H_
#define MQTT_Buffer_size 50
#include <asf.h>
#include <conf_board.h>

int Open_Mqtt_Server(uint8_t host[], uint8_t port[]);
int8_t Set_Mqtt_clientIDx();
int8_t Fetch_open_msg(uint8_t msg[]);
uint8_t Connect_To_Mqtt_Broker(uint8_t client_ID[], uint8_t username[], uint8_t password[]);
uint8_t fetch_conn_msg(uint8_t msg[]);
uint8_t Subscribe_To_Mqtt_Server(int8_t MsgID,uint8_t sub_topic[],int8_t Qos);
uint8_t Publish_To_Mqtt_Server(int8_t clientIDx, int8_t msgID,int8_t Qos, int8_t retain,  uint8_t pub_topic[],  uint8_t *message[]);

struct Error_stat_message{
	char error_msg[100];
}stat_msg;

uint8_t clientID[6];
int Idx_value;

struct server_information
{
	uint32_t server_name[MQTT_Buffer_size];
	uint8_t port[MQTT_Buffer_size];
	uint8_t client_id[MQTT_Buffer_size];
	uint8_t username[MQTT_Buffer_size];
	uint8_t password[MQTT_Buffer_size];
	uint8_t sub_topic[MQTT_Buffer_size];
};
	
struct server_information SNES_Mqtt;

struct open_Idx
{
	Bool active;
	uint8_t status[50];
}open_clients[5];

struct Conn_Idx
{
	Bool active;
	uint8_t status[100];
}Conn_clients[5];

struct Sub_Idx
{
	Bool active;
	uint8_t topic[100];
}Sub_clients[5];



bool InitialiseMqttQueue();
bool InitialiseMQTT(void);
void WaitforSpaceInMqttCommandQueue(void);
bool ReadMQTTResponce(uint8_t *data, uint32_t Timeout);
bool ReadMQTTAsyncResponce( uint8_t *data, uint32_t Timeout );
bool MQTTOpen(uint8_t host[], uint8_t port[]);
bool MQTTConnect(uint8_t client_ID[], uint8_t username[], uint8_t password[]);
bool MQTTSubcribe(int8_t msg_Id, uint8_t topic_name[], int8_t qos); 
bool IsMQTTOpen(int ClientID_num);
bool IsMQTTConnect(int ClientID_num);
bool IsMQTTSubcribe(uint8_t topic_name); 
void MQTTReceiveHandler(void);

InitialiseMQTTCommunication();

bool Set_ver_cfg(int ClientIDx, int version1);
bool Set_pdp_cfg(int ClientIDx, int pdpcid);
bool Set_ssl_cfg(int ClientIDx,int ssl_enable,int sslctx_idx);
bool Set_keepalive_cfg(int ClientIDx, int keep_alive_time);
bool Set_session_cfg(int ClientIDx,int clean_session);
bool Set_will_cfg(int ClientIDx,int will_flag,int will_qos, int will_retain, uint8_t will_topic[], uint8_t will_message[]);
bool Set_timeout_cfg(int ClientIDx,int pkt_timeout,int retry_times, int timeout_notice);
bool Set_recv_mode_cfg(int ClientIDx,int msg_len_enable,int msg_recv_mode);

bool Close_Nw_Connection(int8_t clientIDx);
enum MQTT_Cmd_Resp_State { S_MQTT_IDLE = 0, S_MQTT_SET_CLIENT_ID, S_MQTT_OPEN, S_MQTT_CONNECT, S_MQTT_SUBSCRIBE, S_MQTT_PUBLISH, S_MQTT_SUCCESS, S_MQTT_ERROR,
};
enum MQTT_Cmd_Resp_Event { E_MQTT_COMMAND = 0, E_MQTT_MESSAGE, E_MQTT_RESPONSE};

enum MQTT_Cmd_Resp_State get_MQTT_State();
enum MQTT_Cmd_Resp_Event get_MQTT_Event();

bool IsSpaceAvailableInMqttCommandQueue();
bool SM_MQTT_Init();
bool SM_Registration();

#endif /* MQTT_COMM_H_ */  