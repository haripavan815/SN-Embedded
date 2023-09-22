/********************************************************************************/
/*                                                                              */
/*  File Name:   MQTT.c													        */
/*                                                                              */
/*  Description:   contains Mqtt application, functions and responses	        */
/*                                                                              */
/*  -----------------------------------------------------------------------     */
/*  |  Author              |    Date      | Rev  |       Comments          |    */
/*  -----------------------------------------------------------------------     */
/*  | Harish N			   |  01-01-2022  | 1.0  |			created	       |    */
/*  -----------------------------------------------------------------------     */
/*                                                                              */
/*  Copyright (c) 2021, SNES                                                    */
/*  All rights reserved.                                                        */
/*                                                                              */
/********************************************************************************/
#include "LTE_Module/MQTT_Comm.h"

#define MQTT_BUFFER_LENGTH 500
#define clients_LEN 6
#define MAX 6

QueueHandle_t xQueue_MQTT_command = NULL;
QueueHandle_t xQueue_MQTT_response = NULL;
QueueHandle_t xQueue_MQTT_Async_response = NULL;

bool IsMQTTInitialised = false;
bool IsMQTTSubcribE_MQTT_COMMANDed = false;
bool IsMQTTSubcribed = false;
bool IsMQTTConnect_Commanded = false;
bool IsMQTTConnected = false;
bool IsMQTTOpen_Commanded = false;
bool IsMQTTOpened = false;
bool IsMQTTPub_Commanded = false;
bool Start_Pub = true;
bool Set_ver_flag = true;	
bool Set_pdp_flag = true;
bool Set_keepalive_flag = true;
bool Set_session_flag = true;
bool Set_ssl_flag = true;
bool Set_timeout_flag = true;
bool Set_will_flag = true;
bool Set_recv_flag = true;
bool clientIdarr[clients_LEN]={0};
		 	
int matt_next_state = S_MQTT_IDLE;
int matt_next_event = E_MQTT_COMMAND;

struct Mqtt_Cfg_version {
	int ClientIDx;
	int version1 ;
};

struct Mqtt_cfg_pdp {
	int pdpcid;
	int ClientIDx;
};

struct Mqtt_cfg_ssl{
	int ClientIDx;
	int ssl_enable;
	int sslctx_idx;
};

struct Mqtt_cfg_will{
	int ClientIDx;
	int will_flag;
	int will_qos;
	int will_retain;
};

struct Mqtt_cfg_keepalive{
	int ClientIDx;
	int keep_alive_time;
};

struct Mqtt_cfg_session{
	int ClientIDx;
	int clean_session;
};

struct Mqtt_cfg_timeout{
	int ClientIDx;
	int pkt_timeout;
	int retry_times;
	int timeout_notice;
};

struct Mqtt_cfg_recv_mode{
	int ClientIDx;
	int msg_recv_mode;
	int msg_len_enable;
};

//Queue of 6 to store the received data(Rx queue)		
 struct MessagE_MQTT_RESPONSE{
	int client;
	int msg_id;
	char topic[50];
	char payload[200];
} RX_response[MAX];
	
int rear = 6;
int front = -1;

bool InitialiseMqttQueue()
{
	xQueue_MQTT_command = xQueueCreate( 4, MQTT_BUFFER_LENGTH);
	if( xQueue_MQTT_command == 0 )
	{
		printf("Failed to create the xQueue_MQTT_command\n");
	}
	
	xQueue_MQTT_response = xQueueCreate( 4, MQTT_BUFFER_LENGTH );
	if( xQueue_MQTT_response == 0 )
	{
		printf("Failed to create the xQueue_MQTT_response\n");
	}
	
	xQueue_MQTT_Async_response = xQueueCreate( 4, MQTT_BUFFER_LENGTH );
	if( xQueue_MQTT_Async_response == 0 )
	{
		printf("Failed to create the xQueue_MQTT_Async_response\n");
	}	
		
}

bool InitialiseMQTT(void)
{
	//clientIdarr[0]=true;                //Using Client Id: 1 for testing purpose
	struct server_information snes;
	strcpy(SNES_Mqtt.server_name, "\"www.snembeddedsystems.com\"");
	strcpy(SNES_Mqtt.port, "1883");
	strcpy(SNES_Mqtt.client_id, "\"SNES111\"");
	strcpy(SNES_Mqtt.username, "\"snes_tester\"");
	strcpy(SNES_Mqtt.password, "\"tester@snes\"");
	strcpy(SNES_Mqtt.sub_topic, "\"SNES/VTD/POS\"");
	
	if( true == IsMQTTInitialised )
	{
		//Printing active client's open status, connection and and subscription status
		for(int i=0; i<MAX; i++)
		{
			//printf("Is client %d Opened:= %d\r\n",i, open_clients[i].active);
			//printf("Client %d's Open Status:= %s\r\n",i, open_clients[i].status);
			//printf("Is client %d Connected:= %d\r\n",i,Conn_clients[i].active);
			//printf("Client %d's Connection Status:= %s\r\n",i,Conn_clients[i].status);
			//printf("Is client %d Subscribed:= %d\r\n",i,Sub_clients[i].active);
			//printf("Client %d's Subscription topic:= %s\r\n",i,Sub_clients[i].topic);
			//printf("\r\n");
		}
		return ( true );
	}
	
	if(false == IsMQTTOpen(0))
	{
		MQTTOpen(SNES_Mqtt.server_name, SNES_Mqtt.port);
	}
	if ((false == IsMQTTConnect(0)) && (true == IsMQTTOpen(0)))
	{
		MQTTConnect(SNES_Mqtt.client_id, SNES_Mqtt.username, SNES_Mqtt.password);
	}
	if ((false == IsMQTTSubcribe(SNES_Mqtt.sub_topic)) && (true == IsMQTTConnect(0)))
	{
		MQTTSubcribe(1,SNES_Mqtt.sub_topic,1);
		IsMQTTInitialised = true;
	}
	if (true == IsMQTTSubcribe(SNES_Mqtt.sub_topic))
	{
		IsMQTTInitialised = true;
		return true;
	}
	//Close_Nw_Connection(Idx_value);
	return ( false );
}

bool MQTTOpen(uint8_t host[], uint8_t port[])
{
	uint8_t name[50];
	uint8_t Port[6];
	memset(name, 0, 50);
	memset(Port, 0, 6);
	strcpy(name,host);
	strcpy(Port, port);
	xQueueReset( xQueue_MQTT_command );
	Open_Mqtt_Server(name, Port);
	return ( false );
 }
 
bool IsMQTTOpen(int ClientID_num)
{
	if (open_clients[ClientID_num].active == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool IsMQTTConnect(int ClientID_num)
{
	if (Conn_clients[ClientID_num].active == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}
 
bool MQTTConnect(uint8_t client_ID[], uint8_t username[], uint8_t password[])
{
	uint8_t ID[20];
	uint8_t name[20];
	uint8_t pwd[50];
	memset(ID, 0, 2);
	memset(name, 0, 20);
	memset(pwd, 0, 50);
	strcpy(ID,client_ID);
	strcpy(name,username);
	strcpy(pwd, password);
	xQueueReset( xQueue_MQTT_command );
	Connect_To_Mqtt_Broker(ID,name,pwd);
	return ( false );
}
 
bool MQTTSubcribe(int8_t msg_Id, uint8_t topic_name[], int8_t qos)
{
	uint8_t name[50];
	int8_t msg;
	int8_t Qos;
	memset(name,0,50);
	strcpy(name, topic_name);
	msg = msg_Id;
	Qos = qos;
	xQueueReset( xQueue_MQTT_command );
	Subscribe_To_Mqtt_Server(msg,name,Qos);	
	return ( false );
} 

bool IsMQTTSubcribe(uint8_t topic_name)
{
	if(true == (strstr(Sub_clients[Idx_value].topic, topic_name)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Handles MQTT responses
void MQTTReceiveHandler(void)
{	
	//Echo Turned Off
	// Need to Handle Stat and subscriptions
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};		
	memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
	if( true == ReadMQTTAsyncResponce(MQTTReceiveBuffer, 5 ) )
	{
		if(strstr(MQTTReceiveBuffer, "QMTSTAT" ) != NULL )
		{
			int err;
			memset(stat_msg.error_msg,0,100);
			sscanf(MQTTReceiveBuffer, "\r\n+QMTSTAT: %d,%d\r\n", NULL, &err);
			//printf("Error code = %d\n", err);
			switch(err)
			{
				case 1 : strcpy(stat_msg.error_msg,"Connection is closed or reset by peer\r\n");
							break;
				case 2 : strcpy(stat_msg.error_msg,"Sending PINGREQ packet timed out or failed\r\n");
							break;
				case 3 : strcpy(stat_msg.error_msg,"Sending CONNECT packet timed out or failed\r\n");
							break;
				case 4 : strcpy(stat_msg.error_msg,"Receiving CONNECK packet timed out or failed\r\n");
							break;
				case 5 : strcpy(stat_msg.error_msg,"The client sends DISCONNECT packet to sever and the server is initiative to close MQTT connection\r\n");
							break;
				case 6 : strcpy(stat_msg.error_msg,"The client is initiative to close MQTT connection due to packet sending failure all the time.\r\n");
							break;
				case 7 : strcpy(stat_msg.error_msg,"The link is not alive or the server is unavailable\r\n");
							break;	
				default: strcpy(stat_msg.error_msg, "Unknown error\r\n");						
			}
			printf(stat_msg.error_msg);
		}
			//Holds (saves) last 6 messages inside the buffer
		if(strstr(MQTTReceiveBuffer, "QMTRECV" ) != NULL )
		{
			int CLI_ID = NULL;
			int msgID = NULL;
			uint8_t topic[100] = {0};
			char payload[100] = {0};
				
			memset(topic,0,100);
			memset(payload,0,100);
			sscanf(MQTTReceiveBuffer, "\r\n+QMTRECV: %d,%d,%s,%s\r\n\r\n\r\n", &CLI_ID, &msgID, &topic, NULL);
			//printf("Topic = %s\n", topic);
			char *token;
			token = strtok(topic, ",");
			memcpy(topic,token,strlen(token));
			while(token!=NULL)
			{ 
				memset(payload,0,100);
				memcpy(payload,token,strlen(token));
				token = strtok(NULL, "");
			}
		
			if(rear == 0)
			{
				//printf("Queue overflow\n"); 
				// reset to 6
				rear = 6;
			}
					 
			if(front = -1)																			
			{
				front = 0;
			}
			rear--;
			memset(RX_response[rear].topic,0,100);
			memset(RX_response[rear].payload,0,100);
			RX_response[rear].client =  CLI_ID;
			RX_response[rear].msg_id =  msgID;
			memcpy(RX_response[rear].topic,topic,strlen(topic));
			memcpy(RX_response[rear].payload,payload,strlen(payload));
				
				//Printing Rx queue on the console
			
				/*for(int j=0;j<MAX;j++)
				{
					printf("Client Id = %d\n", RX_response[j].client);
					printf("Msg Id = %d\n", RX_response[j].msg_id);
					printf("Topic = %s\n", RX_response[j].topic);
					printf("Payload = %s\n", RX_response[j].payload); 
					printf("\r\n");
				}*/
				//Compare the packet string from the server for registration details
				if (strcmp(RX_response[0].payload,"**"))
				{
					//Start user registration by storing user data onto QSPI flash area
					for(int j=0;j<MAX;j++)
					{
						printf("Client Id = %d\n", RX_response[j].client);
						printf("Msg Id = %d\n", RX_response[j].msg_id);
						printf("Topic = %s\n", RX_response[j].topic);
						printf("Payload = %s\n", RX_response[j].payload);
						printf("\r\n");
					}
					
				}
				
				//Write last 5 GPS values on to the flash
			}
	
		}

	}

void WaitforSpaceInMqttCommandQueue(void)
{
	while( uxQueueMessagesWaiting(xQueue_MQTT_command) == 4 )
	{
		vTaskDelay(0);
	}
}

//Reads MQTT responses from the buffer
bool ReadMQTTResponce( uint8_t *data, uint32_t Timeout )
{
	uint8_t ReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	uint8_t rx_buffer_mqtt[MQTT_BUFFER_LENGTH] = {0};
	uint8_t ReceiveBufferIndex = 0;
	memset(rx_buffer_mqtt, 0, sizeof(rx_buffer_mqtt));
	do
	{
		if( xQueueReceive( xQueue_MQTT_response, rx_buffer_mqtt, Timeout ) == pdPASS )
		{
			memcpy( &ReceiveBuffer[ReceiveBufferIndex], rx_buffer_mqtt,strlen(rx_buffer_mqtt));
			ReceiveBufferIndex += strlen(rx_buffer_mqtt);
			memset(rx_buffer_mqtt, 0, strlen(rx_buffer_mqtt));
			if( ReceiveBufferIndex >= MQTT_BUFFER_LENGTH )
			{
				memset(ReceiveBuffer, 0, sizeof(ReceiveBuffer));
				ReceiveBufferIndex = 0;
			}
		}
		else
		{
			;
		}
		
	}while( (uxQueueMessagesWaiting(xQueue_MQTT_response) != 0) );
	
	if( ReceiveBufferIndex != 0 )
	{
		memcpy( data , ReceiveBuffer, ReceiveBufferIndex );
		return( true );
	}
	else
	{
		return( false );
	}	
}

//Asynchronous responses are read from the buffer
bool ReadMQTTAsyncResponce( uint8_t *data, uint32_t Timeout )
{
	uint8_t ReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	uint8_t rx_buffer_mqtt[MQTT_BUFFER_LENGTH] = {0};
	uint8_t ReceiveBufferIndex = 0;
	memset(rx_buffer_mqtt, 0, sizeof(rx_buffer_mqtt));
	do
	{
		if( xQueueReceive( xQueue_MQTT_Async_response, rx_buffer_mqtt, Timeout ) == pdPASS )
		{
			memcpy( &ReceiveBuffer[ReceiveBufferIndex], rx_buffer_mqtt,strlen(rx_buffer_mqtt));
			ReceiveBufferIndex += strlen(rx_buffer_mqtt);
			memset(rx_buffer_mqtt, 0, strlen(rx_buffer_mqtt));
			if( ReceiveBufferIndex >= MQTT_BUFFER_LENGTH )
			{
				memset(ReceiveBuffer, 0, sizeof(ReceiveBuffer));
				ReceiveBufferIndex = 0;
			}
		}
		else
		{
			;
		}
		
	}while( (uxQueueMessagesWaiting(xQueue_MQTT_Async_response) != 0) );
	
	if( ReceiveBufferIndex != 0 )
	{
		memcpy( data , ReceiveBuffer, ReceiveBufferIndex );
		return( true );
	}
	else
	{
		return( false );
	}	
}

int Open_Mqtt_Server(uint8_t host[], uint8_t port[])
{
	
	uint8_t	atqmtopenconn[100];
	uint8_t c_ID[6];
	uint8_t port_c[2];
	
	memset(atqmtopenconn,0,100);
	strcpy(atqmtopenconn,"AT+QMTOPEN=");

	strcpy(c_ID,clientID);
	Idx_value = atoi(clientID);
	
	strcat(atqmtopenconn,c_ID);
	strcat(atqmtopenconn, ",");
	strcat(atqmtopenconn,host);
	strcat(atqmtopenconn, ",");
	strcat(atqmtopenconn,port);
	strcat(atqmtopenconn, "\r\n");

	//printf(atqmtopenconn);
	xQueueReset( xQueue_MQTT_command );
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, atqmtopenconn, 0);
	return Idx_value;
}

uint8_t Publish_To_Mqtt_Server(int8_t clientIDx, int8_t msgID,int8_t Qos, int8_t retain,  uint8_t pub_topic[],  uint8_t *message[])
{
	static TickType_t MqttCurrentTime=0, PrevMqttStartTime=0;
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	uint8_t atqmtpub[MQTT_BUFFER_LENGTH] = {0};
	char message1[200];
	uint8_t c_ID[2];
	uint8_t M_id[2];
	uint8_t Qos_d[2];
	uint8_t retain_d[2];
	uint8_t EndofMessage[] = {0x1A};
					
	//if (false == IsMQTTPub_Commanded)
	{	
		PrevMqttStartTime = xTaskGetTickCount();
		sprintf(c_ID, "%d", clientIDx);
		sprintf(M_id, "%d", msgID);
		sprintf(Qos_d, "%d", Qos);
		sprintf(retain_d, "%d", retain);
	
		memset(atqmtpub,0,250);
		memset(message1,0,200);
		strcpy(atqmtpub,"AT+QMTPUB=");
		strcat(atqmtpub,c_ID);
		strcat(atqmtpub,",");
		strcat(atqmtpub,M_id);
		strcat(atqmtpub,",");
		strcat(atqmtpub,Qos_d);
		strcat(atqmtpub,",");
		strcat(atqmtpub,retain_d);
		strcat(atqmtpub,",");
		strcat(atqmtpub,pub_topic);
		strcat(atqmtpub,"\r\n");
	
		int temp = strlen(message)+1;
		memcpy(message1,message,temp);
		strcat(message1,"\r\n");

		//printf("%s", atqmtpub);
		//printf("%s", message1);
	
		strcat(atqmtpub,EndofMessage);
		strcat(message1,EndofMessage);
	
	    //strcat(atqmtpub, message1);
		//printf("%s", atqmtpub);
		
		xQueueReset( xQueue_MQTT_command );
		WaitforSpaceInMqttCommandQueue();
		xQueueSend( xQueue_MQTT_command, atqmtpub, 0);
		IsMQTTPub_Commanded = true;
		vTaskDelay(500);//delay_ms(100);// Allowing LTE Task to excecute command first before message
		xQueueReset( xQueue_MQTT_command );
		WaitforSpaceInMqttCommandQueue();
		xQueueSend( xQueue_MQTT_command, message1, 0);
	} 

#if 1
	//WaitforSpaceInMqttCommandQueue();
	memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
	uint32_t timeout_tries = 0;
	//printf("out: %d", timeout_tries);
	do
	{
		//printf("in: %d", timeout_tries);
		if( true == ReadMQTTResponce(MQTTReceiveBuffer, 5 ) )
		{
			//printf("Res: %s\n", MQTTReceiveBuffer);
			IsMQTTPub_Commanded = false;
			if(strstr(MQTTReceiveBuffer, "+QMTPUB:" ))
			{
				//delay_ms(100);
				if((strstr(MQTTReceiveBuffer,"+QMTPUB: 0,1,0"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 1,1,0"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 2,1,0"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 3,1,0"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 4,1,0"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 5,1,0")))
				{
					printf("Sent packet successfully and received ACK from server\r\n");
					return true;
				}
				else if ((strstr(MQTTReceiveBuffer,"+QMTPUB: 0,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 1,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 2,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 3,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 4,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 5,1,1")))
				{
					printf("Packet retransmission\r\n");
					return false;
				}
				else if ((strstr(MQTTReceiveBuffer,"+QMTPUB: 0,1,2"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 1,1,2"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 2,1,2"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 3,1,2"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 4,1,2"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 5,1,2")))
				{
					printf("Failed to send packet\r\n");
					return false;
				}
				else
				{
					//IsMQTTPub_Commanded = true;
					printf("Failed to send packet\r\n");
					return false;
				}
			}
		}
	} while (timeout_tries++ <= (37500/50));
#else	
		if( true == IsMQTTPub_Commanded )
		{
			
			WaitforSpaceInMqttCommandQueue();
			memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
			if( true == ReadMQTTResponce(MQTTReceiveBuffer, 300 ) )
			//delay_ms(5000);
			{
				IsMQTTPub_Commanded = false;
				if(strstr(MQTTReceiveBuffer, "+QMTPUB:" ))
				{
					//delay_ms(100);
					if((strstr(MQTTReceiveBuffer,"+QMTPUB: 0,1,0"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 1,1,0"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 2,1,0"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 3,1,0"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 4,1,0"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 5,1,0")))
					{
						printf("Sent packet successfully and received ACK from server\r\n");
						return true;
					}
					else if ((strstr(MQTTReceiveBuffer,"+QMTPUB: 0,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 1,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 2,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 3,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 4,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 5,1,1")))
					{
						printf("Packet retransmission\r\n");
						return false;
					}
					else if ((strstr(MQTTReceiveBuffer,"+QMTPUB: 0,1,2"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 1,1,2"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 2,1,2"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 3,1,2"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 4,1,2"))||(strstr(MQTTReceiveBuffer,"+QMTPUB: 5,1,2")))
					{
						printf("Failed to send packet\r\n");
						return false;
					}
					else
					{
						//IsMQTTPub_Commanded = true;
						printf("Failed to send packet\r\n");
						return false;
					}

				}
			}
			else
			{
				MqttCurrentTime = xTaskGetTickCount();
				if( (MqttCurrentTime - PrevMqttStartTime) > 37500 )
				{
					IsMQTTPub_Commanded = false;
				}
			}
	}
#endif	
	return false;
}

//Responses from the open message command are being monitored and controlled
int8_t Fetch_open_msg(uint8_t msg[])
{
		
	memset(open_clients[Idx_value].active,0,5);
	memset(open_clients[Idx_value].status,0,50);
	
	if(((strstr(msg,"+QMTOPEN: 0,0"))||(strstr(msg,"+QMTOPEN: 1,0")))||(strstr(msg,"+QMTOPEN: 2,0"))||(strstr(msg,"+QMTOPEN: 3,0"))||(strstr(msg,"+QMTOPEN: 4,0"))||(strstr(msg,"+QMTOPEN: 5,0")))
	{
		printf( "Network opened successfully\n");
		open_clients[Idx_value].active = true;
		strcpy(open_clients[Idx_value].status,"Network opened successfully");
		clientIdarr[Idx_value] = true;
		matt_next_state = S_MQTT_CONNECT;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
		//break;
	}
	else if(((strstr(msg,"+QMTOPEN: 0,4"))||(strstr(msg,"+QMTOPEN: 1,4")))||(strstr(msg,"+QMTOPEN: 2,4"))||(strstr(msg,"+QMTOPEN: 3,4"))||(strstr(msg,"+QMTOPEN: 4,4"))||(strstr(msg,"+QMTOPEN: 5,4")))
	{
		printf("Failed to parse domain name\n");
		open_clients[Idx_value].active = false;
		strcpy(open_clients[Idx_value].status ,"Failed to parse domain name\r\n");
		matt_next_state = S_MQTT_OPEN;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else if(((strstr(msg,"+QMTOPEN: 0,-1"))||(strstr(msg,"+QMTOPEN: 1,-1")))||(strstr(msg,"+QMTOPEN: 2,-1"))||(strstr(msg,"+QMTOPEN: 3,-1"))||(strstr(msg,"+QMTOPEN: 4,-1"))||(strstr(msg,"+QMTOPEN: 5,-1")))
	{
		printf("Failed to open network\n");
		
		open_clients[Idx_value].active = false;
		strcpy(open_clients[Idx_value].status,"Failed to open network\r\n");
		matt_next_state = S_MQTT_OPEN;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else if(((strstr(msg,"+QMTOPEN: 0,1"))||(strstr(msg,"+QMTOPEN: 1,1")))||(strstr(msg,"+QMTOPEN: 2,1"))||(strstr(msg,"+QMTOPEN: 3,1"))||(strstr(msg,"+QMTOPEN: 4,1"))||(strstr(msg,"+QMTOPEN: 5,1")))
	{
		printf("Wrong parameter\n");
		matt_next_state = S_MQTT_OPEN;
		matt_next_event = E_MQTT_COMMAND;
		open_clients[Idx_value].active = false;
		strcpy(open_clients[Idx_value].status,"Wrong parameter\r\n");
		
		return Idx_value;
	}
	else if(((strstr(msg,"+QMTOPEN: 0,2"))||(strstr(msg,"+QMTOPEN: 1,2")))||(strstr(msg,"+QMTOPEN: 2,2"))||(strstr(msg,"+QMTOPEN: 3,2"))||(strstr(msg,"+QMTOPEN: 4,2"))||(strstr(msg,"+QMTOPEN: 5,2")))
	{
		printf("MQTT identifier is occupied\n");
		matt_next_state = S_MQTT_SET_CLIENT_ID;
		matt_next_event = E_MQTT_COMMAND;
		open_clients[Idx_value].active = true;
		strcpy(open_clients[Idx_value].status,"MQTT identifier is occupied");
		clientIdarr[Idx_value] = true;
		return Idx_value;
	}
	else if(strstr(msg,"QMTSTAT"))
	{
		printf("ERROR\n");
		matt_next_state = S_MQTT_OPEN;
		matt_next_event = E_MQTT_COMMAND;
		open_clients[Idx_value].active = false;
		strcpy(open_clients[Idx_value].status,"ERROR\r\n");
		return Idx_value;
	}
	else
	{
		printf(msg);
		open_clients[Idx_value].active = false;
		strcpy(open_clients[Idx_value].status,"Timeout\r\n");
		return Idx_value;
	}
}

//Automatically sets the client ID to the next one, if previous is occupied
int8_t Set_Mqtt_clientIDx()
{
	int i;
	//When all the client ID's are occupied, resetting all the clientID. so that connection restarts
	if (open_clients[Idx_value].active==false)
	{
		for (i=0;i<clients_LEN;i++)
		{
			clientIdarr[clients_LEN] = false;
		}	
	}
	//clientIdarr[0] = true;
	//printf(clientIdarr);
	
	if (clientIdarr[0] == 0)
	{
		strcpy(clientID,"0");
		Idx_value = 0;
		return 1;
	}
	else if (clientIdarr[1] == 0)
	{
		strcpy(clientID,"1");
		Idx_value = 1;
		return 1;
	}
	else if (clientIdarr[2] == 0)
	{
		strcpy(clientID,"2");
		Idx_value = 2;
		return 1;
	}
	else if (clientIdarr[3] == 0)
	{
		strcpy(clientID,"3");
		Idx_value = 3;
		return 1;
	}
	else if (clientIdarr[4] == 0)
	{
		strcpy(clientID,"4");
		Idx_value = 4;
		return 1;
	}
	else if (clientIdarr[5] == 0)
	{
		strcpy(clientID,"5");
		Idx_value = 5;
		return 1;
	}
	else
	{
		strcpy(clientID,"n");
		open_clients[Idx_value].active=false;
		 strcpy(open_clients[Idx_value].status, "All Client ID's are occupied'\r\n");
		 return 0;
	}	
}

uint8_t Connect_To_Mqtt_Broker(uint8_t client_ID[], uint8_t username[], uint8_t password[])
{
	uint8_t atqmtconn[100];
	memset(atqmtconn,0,100);
	strcpy(atqmtconn,"AT+QMTCONN=");
	uint8_t c_ID[2];
	strcpy(c_ID,clientID);
	Idx_value = atoi(clientID);
	strcat(atqmtconn, c_ID);
	strcat(atqmtconn,",");
	strcat(atqmtconn,client_ID);
	strcat(atqmtconn,",");
	strcat(atqmtconn,username);
	strcat(atqmtconn,",");
	strcat(atqmtconn,password);
	strcat(atqmtconn,"\r\n");
	
	//printf(atqmtconn);
	xQueueReset( xQueue_MQTT_command );
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, atqmtconn, 0);
	return Idx_value;
}

uint8_t fetch_conn_msg(uint8_t msg[])
{
	if((strstr(msg,"+QMTCONN: 0,0,0"))||(strstr(msg,"+QMTCONN: 1,0,0"))||(strstr(msg,"+QMTCONN: 2,0,0"))||(strstr(msg,"+QMTCONN: 3,0,0"))||(strstr(msg,"+QMTCONN: 4,0,0"))||(strstr(msg,"+QMTCONN: 5,0,0")))
	{
		printf( "Connection Accepted\n");
		Conn_clients[Idx_value].active = true;
		IsMQTTConnected = true;
		strcpy(Conn_clients[Idx_value].status,"Connection Accepted");
		matt_next_state = S_MQTT_SUBSCRIBE;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else if((strstr(msg,"+QMTCONN: 0,0,1"))||(strstr(msg,"+QMTCONN: 1,0,1"))||(strstr(msg,"+QMTCONN: 2,0,1"))||(strstr(msg,"+QMTCONN: 3,0,1"))||(strstr(msg,"+QMTCONN: 4,0,1"))||(strstr(msg,"+QMTCONN: 5,0,1")))
	{
		printf( "Connection Refused: Unacceptable Protocol Version\n");
		Conn_clients[Idx_value].active = false;
		strcpy(Conn_clients[Idx_value].status,"Connection Refused: Unacceptable Protocol Version\r\n");
		matt_next_state = S_MQTT_CONNECT;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else if((strstr(msg,"+QMTCONN: 0,0,2"))||(strstr(msg,"+QMTCONN: 1,0,2"))||(strstr(msg,"+QMTCONN: 2,0,2"))||(strstr(msg,"+QMTCONN: 3,0,2"))||(strstr(msg,"+QMTCONN: 4,0,2"))||(strstr(msg,"+QMTCONN: 5,0,2")))
	{
		printf( "Connection Refused: Identifier Rejected\n");
		Conn_clients[Idx_value].active = false;
		strcpy(Conn_clients[Idx_value].status,"Connection Refused: Identifier Rejected\r\n");
		matt_next_state = S_MQTT_CONNECT;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else if((strstr(msg,"+QMTCONN: 0,0,3"))||(strstr(msg,"+QMTCONN: 1,0,3"))||(strstr(msg,"+QMTCONN: 2,0,3"))||(strstr(msg,"+QMTCONN: 3,0,3"))||(strstr(msg,"+QMTCONN: 4,0,3"))||(strstr(msg,"+QMTCONN: 5,0,3")))
	{
		printf( "Connection Refused: Server Unavailable\n");
		Conn_clients[Idx_value].active = false;
		strcpy(Conn_clients[Idx_value].status,"Connection Refused: Server Unavailable\r\n");
		matt_next_state = S_MQTT_CONNECT;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else if((strstr(msg,"+QMTCONN: 0,0,4"))||(strstr(msg,"+QMTCONN: 1,0,4"))||(strstr(msg,"+QMTCONN: 2,0,4"))||(strstr(msg,"+QMTCONN: 3,0,4"))||(strstr(msg,"+QMTCONN: 4,0,4"))||(strstr(msg,"+QMTCONN: 5,0,4")))
	{
		printf( "Connection Refused: Bad User Name or Password\n");
		Conn_clients[Idx_value].active = false;
		strcpy(Conn_clients[Idx_value].status,"Connection Refused: Bad User Name or Password\r\n");
		matt_next_state = S_MQTT_CONNECT;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else if((strstr(msg,"+QMTCONN: 0,0,5"))||(strstr(msg,"+QMTCONN: 1,0,5"))||(strstr(msg,"+QMTCONN: 2,0,5"))||(strstr(msg,"+QMTCONN: 3,0,5"))||(strstr(msg,"+QMTCONN: 4,0,5"))||(strstr(msg,"+QMTCONN: 5,0,5")))
	{
		printf( "Connection Refused: Not Authorized\n");
		Conn_clients[Idx_value].active = false;
		strcpy(Conn_clients[Idx_value].status,"Connection Refused: Not Authorized\r\n");
		matt_next_state = S_MQTT_CONNECT;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else if((strstr(msg,"+QMTCONN: 0,1"))||(strstr(msg,"+QMTCONN: 1,1"))||(strstr(msg,"+QMTCONN: 2,1"))||(strstr(msg,"+QMTCONN: 3,1"))||(strstr(msg,"+QMTCONN: 4,1"))||(strstr(msg,"+QMTCONN: 5,1")))
	{
		printf( "Packet retransmission\n");
		Conn_clients[Idx_value].active = true;
		strcpy(Conn_clients[Idx_value].status,"Packet retransmission\r\n");
		matt_next_state = S_MQTT_CONNECT;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else if((strstr(msg,"+QMTCONN: 0,2"))||(strstr(msg,"+QMTCONN: 1,2"))||(strstr(msg,"+QMTCONN: 2,2"))||(strstr(msg,"+QMTCONN: 3,2"))||(strstr(msg,"+QMTCONN: 4,2"))||(strstr(msg,"+QMTCONN: 5,2")))
	{
		printf( "Failed to send packet\n");
		Conn_clients[Idx_value].active = false;
		strcpy(Conn_clients[Idx_value].status,"Failed to send packet\r\n");
		matt_next_state = S_MQTT_CONNECT;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else if(strstr(msg,"ERROR"))
	{
		IsMQTTConnected=true;
		Conn_clients[Idx_value].active = true;
		strcpy(Conn_clients[Idx_value].status,"Connection Accepted");
		matt_next_state = S_MQTT_SUBSCRIBE;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
	else
	{
		printf( "Timeout\n");
		Conn_clients[Idx_value].active = true;
		strcpy(Conn_clients[Idx_value].status,"Timeout\r\n");
		matt_next_state = S_MQTT_CONNECT;
		matt_next_event = E_MQTT_COMMAND;
		return Idx_value;
	}
}

uint8_t Subscribe_To_Mqtt_Server(int8_t MsgID,uint8_t sub_topic[],int8_t Qos)
{	
	uint8_t	atqmtsub[100]= "AT+QMTSUB=";
	uint8_t c_ID[2];
	uint8_t M_id[2];
	uint8_t Qos_d[2];
	strcpy(c_ID,clientID);
	Idx_value = atoi(clientID);
	sprintf(M_id, "%d", MsgID);
	sprintf(Qos_d, "%d", Qos);	
	
	strcat(atqmtsub, c_ID);
	strcat(atqmtsub,",");
	strcat(atqmtsub,M_id);
	strcat(atqmtsub,",");
	strcat(atqmtsub,sub_topic);
	strcat(atqmtsub,",");
	strcat(atqmtsub,Qos_d);
	strcat(atqmtsub,"\r\n");

	//printf(atqmtsub);
	Sub_clients[Idx_value].active = true;
	strcpy(Sub_clients[Idx_value].topic,sub_topic);
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, atqmtsub, 0);
	return Idx_value;	
}

bool Set_ver_cfg(int ClientIDx, int version1)
{
	uint8_t	cfgver[50] = "AT+QMTCFG=\"version\",";
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	struct Mqtt_Cfg_version version;
	version.ClientIDx = ClientIDx;
	version.version1= version1;
	uint8_t c_ID[2];
	uint8_t ver[2];
		
	if( Set_ver_flag == false)
	return false;
	else
	{
	sprintf(c_ID, "%d", version.ClientIDx);
	strcat(cfgver, c_ID);
	strcat(cfgver, ",");
	sprintf(ver, "%d", version.version1);
	strcat(cfgver, ver);
	strcat(cfgver,"\r\n");
	xQueueReset( xQueue_MQTT_command );
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, cfgver, 0);
	//delay_ms(50);
	memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
		if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
		{
			if(strstr(MQTTReceiveBuffer, "OK" ) != NULL )
			{	
			printf("Version configuration Successful\n");
			return true;
			}
			else
			{
			printf("Version configuration Failed\n");
			return false;
			}
		}
		return false;
	}
}

bool Set_pdp_cfg(int ClientIDx, int pdpcid)
{
	uint8_t	cfgpdp[50] = "AT+QMTCFG=\"pdpcid\",";
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	struct Mqtt_cfg_pdp pdp;
	pdp.ClientIDx = ClientIDx;
	pdp.pdpcid = pdpcid;
	uint8_t c_ID1[2];
	uint8_t pdp_id[2];
	
	if (Set_pdp_flag == false)
	return false;
	else
	{
		sprintf(c_ID1, "%d", pdp.ClientIDx);
		strcat(cfgpdp, c_ID1);
		strcat(cfgpdp, ",");
		sprintf(pdp_id, "%d", pdp.pdpcid);
		strcat(cfgpdp, pdp_id);
		strcat(cfgpdp,"\r\n");
		xQueueReset( xQueue_MQTT_command );
		WaitforSpaceInMqttCommandQueue();
		xQueueSend( xQueue_MQTT_command, cfgpdp, 0);
		//delay_ms(50);
		memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
		if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
		{
			if(strstr(MQTTReceiveBuffer, "OK" ) != NULL )
			{
				printf("PDP configuration Successful\n");
				return true;
			}
			else
			{
				printf("PDP configuration Failed\n");
				return false;
			}
		}
		return false;
	}
}
		
bool Set_ssl_cfg(int ClientIDx,int ssl_enable,int sslctx_idx)
{
	uint8_t	cfgssl[50] = "AT+QMTCFG=\"ssl\",";
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	struct Mqtt_cfg_ssl ssl;
	ssl.ClientIDx = ClientIDx;
	ssl.ssl_enable = ssl_enable;
	ssl.sslctx_idx = sslctx_idx;
	uint8_t c_ID2[2];
	uint8_t ssl_en[2];
	uint8_t ssl_id[2];
	if( Set_ssl_flag == false)
	return false;
	else
	{
	sprintf(c_ID2, "%d", ssl.ClientIDx);
	strcat(cfgssl, c_ID2);
	strcat(cfgssl, ",");
	sprintf(ssl_en, "%d", ssl.ssl_enable);
	strcat(cfgssl, ssl_en);
	strcat(cfgssl, ",");
	sprintf(ssl_id, "%d", ssl.sslctx_idx);
	strcat(cfgssl, ssl_id);
	strcat(cfgssl,"\r\n");
	
	xQueueReset( xQueue_MQTT_command );
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, cfgssl, 0);
	//delay_ms(50);
	memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
	if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
	{
		if(strstr(MQTTReceiveBuffer, "OK" ) != NULL )
		{
			printf("SSL configuration Successful\n");
			return true;
		}
		else
		{
			printf("SSL configuration Failed\n");
			return false;
		}
	}
	return false;
}
}

bool Set_keepalive_cfg(int ClientIDx, int keep_alive_time )
{
	uint8_t	cfgalive[50] = "AT+QMTCFG=\"keepalive\",";
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	struct Mqtt_cfg_keepalive keepalive;
	keepalive.ClientIDx = ClientIDx;
	keepalive.keep_alive_time = keep_alive_time;
	uint8_t c_ID3[2];
	uint8_t alive_time[5];
	if( Set_keepalive_flag == false)
	return false;
	else
	{
	sprintf(c_ID3, "%d", keepalive.ClientIDx);
	strcat(cfgalive, c_ID3);
	strcat(cfgalive, ",");
	sprintf(alive_time, "%d", keepalive.keep_alive_time);
	strcat(cfgalive, alive_time);
	strcat(cfgalive,"\r\n");
	
	xQueueReset( xQueue_MQTT_command );
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, cfgalive, 0);
	//delay_ms(50);
	memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
	if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
	{
		if(strstr(MQTTReceiveBuffer, "OK" ) != NULL )
		{
			printf("Keepalive configuration Successful\n");
			return true;
		}
		else
		{
			printf("Keepalive configuration Failed\n");
			return false;
		}
	}
	return false;
}
}

bool Set_session_cfg(int ClientIDx,int clean_session)
{
	uint8_t	cfgsession[50] = "AT+QMTCFG=\"session\",";
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	struct Mqtt_cfg_session session;
	session.ClientIDx = ClientIDx;
	session.clean_session = clean_session;
	uint8_t c_ID4[2];
	uint8_t clean[2];
	
	if( Set_session_flag == false)
	return false;
	else
	{
	sprintf(c_ID4, "%d", session.ClientIDx);
	strcat(cfgsession, c_ID4);
	strcat(cfgsession, ",");
	sprintf(clean, "%d", session.clean_session);
	strcat(cfgsession,clean);
	strcat(cfgsession,"\r\n");
	xQueueReset( xQueue_MQTT_command );
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, cfgsession, 0);
	//delay_ms(50);
	memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
	if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
	{
		if(strstr(MQTTReceiveBuffer, "OK" ) != NULL )
		{
			printf("Session configuration Successful\n");
			return true;
		}
		else
		{
			printf("Session configuration Failed\n");
			return false;
		}
	}
	return false;
}
}

bool Set_will_cfg(int ClientIDx,int will_flag,int will_qos, int will_retain, uint8_t will_topic[], uint8_t will_message[])
{
	uint8_t	cfgwill[100] = "AT+QMTCFG=\"will\",";
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	struct Mqtt_cfg_will will;
	will.ClientIDx = ClientIDx;
	will.will_flag = will_flag;
	will.will_qos= will_qos;
	will.will_retain = will_retain;
	uint8_t c_ID5[2];
	uint8_t flag[2];
	uint8_t qos[2];
	uint8_t retain[2];
	uint8_t will_Topic[10];
	uint8_t will_Msg[50];

	if( Set_will_flag == false)
	return false;
	else
	{
	sprintf(c_ID5, "%d", will.ClientIDx);
	strcat(cfgwill,c_ID5);
	strcat(cfgwill, ",");
	sprintf(flag, "%d", will.will_flag);
	strcat(cfgwill, flag);
	strcat(cfgwill, ",");
	sprintf(qos, "%d", will.will_qos);
	strcat(cfgwill, qos);
	strcat(cfgwill, ",");
	sprintf(retain, "%d", will.will_retain);
	strcat(cfgwill, retain);
	strcat(cfgwill, ",");
	strcat(cfgwill, will_topic);
	strcat(cfgwill, ",");
	strcat(cfgwill, will_message);
	strcat(cfgwill,"\r\n");
	xQueueReset( xQueue_MQTT_command );
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, cfgwill, 0);
	//delay_ms(50);
	memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
	if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
	{
		if(strstr(MQTTReceiveBuffer, "OK" ) != NULL )
		{
			printf("Will configuration Successful\n");
			return true;
		}
		else
		{
			printf("Will configuration Failed\n");
			return false;
		}
	}
	return false;
}
}

bool Set_timeout_cfg(int ClientIDx,int pkt_timeout,int retry_times, int timeout_notice)
{
	uint8_t	cfgtimeout[50] = "AT+QMTCFG=\"timeout\",";
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	struct Mqtt_cfg_timeout timeout;
	timeout.ClientIDx = ClientIDx;
	timeout.pkt_timeout = pkt_timeout;
	timeout.retry_times = retry_times;
	timeout.timeout_notice = timeout_notice;
	uint8_t c_ID6[2];
	uint8_t pkt_time[2];
	uint8_t retry[2];
	uint8_t notice[2];
	
	if( Set_timeout_flag == false)
	return false;
	else
	{
	sprintf(c_ID6, "%d", timeout.ClientIDx);
	strcat(cfgtimeout, c_ID6);
	strcat(cfgtimeout, ",");
	sprintf(pkt_time, "%d", timeout.pkt_timeout);
	strcat(cfgtimeout, pkt_time);
	strcat(cfgtimeout, ",");
	sprintf(retry, "%d", timeout.retry_times);
	strcat(cfgtimeout,retry);
	strcat(cfgtimeout, ",");
	sprintf(notice, "%d", timeout.timeout_notice);
	strcat(cfgtimeout, notice);
	strcat(cfgtimeout,"\r\n");
	
	xQueueReset( xQueue_MQTT_command );
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, cfgtimeout, 0);
	//delay_ms(50);
	memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
	if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
	{
		if(strstr(MQTTReceiveBuffer, "OK" ) != NULL )
		{
			printf("Timeout configuration Successful\n");
			return true;
		}
		else
		{
			printf("Timeout configuration Failed\n");
			return false;
		}
	}
	return false;
}
}

bool Set_recv_mode_cfg(int ClientIDx,int msg_len_enable,int msg_recv_mode)
{
	uint8_t	cfgrecvm[50] = "AT+QMTCFG=\"recv/mode\",";
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	struct Mqtt_cfg_recv_mode recv_mode;
	recv_mode.ClientIDx = ClientIDx;
	recv_mode.msg_len_enable = msg_len_enable;
	recv_mode.msg_recv_mode = msg_recv_mode;
	uint8_t c_ID7[2];
	uint8_t enable[2];
	uint8_t mode[2];
	if( Set_recv_flag == false)
	return false;
	else
	{
	sprintf(c_ID7, "%d", recv_mode.ClientIDx);
	strcat(cfgrecvm, c_ID7);
	strcat(cfgrecvm, ",");
	sprintf(enable, "%d", recv_mode.msg_len_enable);
	strcat(cfgrecvm, enable);
	strcat(cfgrecvm, ",");
	sprintf(mode, "%d", recv_mode.msg_recv_mode);
	strcat(cfgrecvm, mode);
	strcat(cfgrecvm,"\r\n");
	
	xQueueReset( xQueue_MQTT_command );
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, cfgrecvm, 0);
	//delay_ms(50);
	memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
	if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
	{
		if(strstr(MQTTReceiveBuffer, "OK" ) != NULL )
		{
			printf("Recv/mode configuration Successful\n");
			return true;
		}
		else
		{
			printf("Recv/mode configuration Failed\n");
			return false;
		}
	}
	return false;
}
}

//Closes the recently connected clientID
bool Close_Nw_Connection(int8_t clientIDx)
{
	uint8_t	atqmtclose[15]= "AT+QMTCLOSE=";
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	uint8_t c_ID[2];
	sprintf(c_ID, "%d", clientIDx);
	strcat(atqmtclose, c_ID);
	strcat(atqmtclose,"\r\n");
	
	xQueueReset( xQueue_MQTT_command );
	WaitforSpaceInMqttCommandQueue();
	xQueueSend( xQueue_MQTT_command, atqmtclose, 0);
	//delay_ms(50);
	/*memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
	if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
	{
 		
	 if ((strstr(MQTTReceiveBuffer,"0,0"))||(strstr(MQTTReceiveBuffer,"1,0"))||(strstr(MQTTReceiveBuffer,"2,0"))||(strstr(MQTTReceiveBuffer,"3,0"))||(strstr(MQTTReceiveBuffer,"4,0"))||(strstr(MQTTReceiveBuffer,"5,0")))
	
	 {*/
		if (clientIDx == 0){clientIdarr[0] = false;}
		else if (clientIDx == 1){clientIdarr[1] = false;}
		else if (clientIDx == 2){clientIdarr[2] = false;}
		else if (clientIDx == 3){clientIdarr[3] = false;}
		else if (clientIDx == 4){clientIdarr[4] = false;}
		else if (clientIDx == 5){clientIdarr[5] = false;}

		/*printf(clientIdarr);
		printf("Network closed successfully\r\n");
		return true;
	}
	else
	{
		printf( "Failed to close network\r\n");
		return false;
	}
	return false;
   }*/
}

bool IsSpaceAvailableInMqttCommandQueue()
{
	if( uxQueueMessagesWaiting(xQueue_MQTT_command) == 4 )
	return false;
	
	return true;
}

enum MQTT_Cmd_Resp_State get_MQTT_State()
{
	return matt_next_state;
}

enum MQTT_Cmd_Resp_Event get_MQTT_Event()
{
	return matt_next_event;
}
	
static int time_out = 0;		
static TimeOut_t xTimeOut;
TickType_t matt_xTicksToWait = 0;

bool SM_MQTT_Init()
{
	uint8_t MQTTReceiveBuffer[MQTT_BUFFER_LENGTH] = {0};
	uint8_t EndofMessage[] = { 0x1A,0x1A,0x0D}; 
	strcpy(SNES_Mqtt.server_name, "\"3.7.68.45\"");
	strcpy(SNES_Mqtt.port, "1883");
	strcpy(SNES_Mqtt.client_id, "\"SNES111\"");
	strcpy(SNES_Mqtt.username, "\"admin\"");
	strcpy(SNES_Mqtt.password, "\"password\"");
	strcpy(SNES_Mqtt.sub_topic, "\"snes/reg\"");

	switch(matt_next_state)
	{
		case S_MQTT_IDLE:
		{
			matt_next_state = S_MQTT_SET_CLIENT_ID;
		}break;
		
		case S_MQTT_SET_CLIENT_ID:
		{
				if(Set_Mqtt_clientIDx())
				{
					printf("Client_ID = %d\r\ n", Idx_value);
					matt_next_state = S_MQTT_OPEN;
				}
				else
				matt_next_state = S_MQTT_SET_CLIENT_ID;		
		}break;

		case S_MQTT_OPEN:
		{
				switch(matt_next_event)
				{										
					case E_MQTT_COMMAND:
					{
						if(IsSpaceAvailableInMqttCommandQueue())
						{
							vTaskSetTimeOutState( &xTimeOut );
							matt_xTicksToWait = 10000;
							xQueueReset( xQueue_MQTT_command);
							MQTTOpen(SNES_Mqtt.server_name, SNES_Mqtt.port);
							matt_next_event = E_MQTT_RESPONSE;
						}
						else
						{
							matt_next_event = E_MQTT_COMMAND;
						}
	
					}break;
					
					case E_MQTT_RESPONSE:
					{
						memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
						if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
						{
							Fetch_open_msg(MQTTReceiveBuffer);
						}
						if( xTaskCheckForTimeOut( &xTimeOut, &matt_xTicksToWait ) != pdFALSE )
						{
							matt_next_event = E_MQTT_COMMAND;
							break;
						}
						
					}break;
										
					default: break;
				}
		}break;

		case S_MQTT_CONNECT:
		{
			switch(matt_next_event)
			{
				case E_MQTT_COMMAND:
				{
					if(IsSpaceAvailableInMqttCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						matt_xTicksToWait = 10000;
						xQueueReset( xQueue_MQTT_command);
						MQTTConnect(SNES_Mqtt.client_id,SNES_Mqtt.username,SNES_Mqtt.password);
						matt_next_event = E_MQTT_RESPONSE;
					}
					else
					{
						matt_next_event = E_MQTT_COMMAND;
					}
			
				}break;
		
				case E_MQTT_RESPONSE:
				{
					memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
					if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_MQTT_response );
						fetch_conn_msg(MQTTReceiveBuffer);
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &matt_xTicksToWait ) != pdFALSE )
					{
						matt_next_event = E_MQTT_COMMAND;
						break;
					}
				}break;
		
				default: break;
			}
		}break;
		
		case S_MQTT_SUBSCRIBE:
		{
			switch(matt_next_event)
			{
				case E_MQTT_COMMAND:
				{
					if(IsSpaceAvailableInMqttCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						matt_xTicksToWait = 10000;
						xQueueReset( xQueue_MQTT_command);
						MQTTSubcribe(1,SNES_Mqtt.sub_topic,1);
						matt_next_event = E_MQTT_RESPONSE;
					}
					else
					{
						matt_next_event = E_MQTT_COMMAND;
					}
			
				}break;
		
				case E_MQTT_RESPONSE:
				{
					memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
					if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_MQTT_response );
						if((strstr(MQTTReceiveBuffer,"+QMTSUB: 0,1,0,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 1,1,0,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 2,1,0,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 3,1,0,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 4,1,0,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 5,1,0,1")))
						{
							printf("Sent packet successfully and received ACK from server\r\n");
							Sub_clients[Idx_value].active = 1;
							matt_next_state = S_MQTT_SUCCESS;
							matt_next_event = E_MQTT_COMMAND;
						}
						else if ((strstr(MQTTReceiveBuffer,"+QMTSUB: 0,1,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 1,1,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 2,1,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 3,1,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 4,1,1,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 5,1,1,1")))
						{
							printf("Packet retransmission\r\n");
							Sub_clients[Idx_value].active = 0;
							matt_next_state = S_MQTT_SUBSCRIBE;
							matt_next_event = E_MQTT_COMMAND;
						}
						else if ((strstr(MQTTReceiveBuffer,"+QMTSUB: 0,1,2,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 1,1,2,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 2,1,2,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 3,1,2,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 4,1,2,1"))||(strstr(MQTTReceiveBuffer,"+QMTSUB: 5,1,2,1")))
						{
							printf("Failed to send packet\r\n");
							Sub_clients[Idx_value].active = 0;
							matt_next_state = S_MQTT_SUBSCRIBE;
							matt_next_event = E_MQTT_COMMAND;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &matt_xTicksToWait ) != pdFALSE )
					{
						matt_next_event = E_MQTT_COMMAND;
						break;
					}
				}break;
		
				default: break;
			}
		}break;
		
		/*case S_MQTT_SUCCESS:
		{
			matt_next_state = S_MQTT_PUBLISH;
		}
		break;
		*/
		case S_MQTT_PUBLISH:
		{
			switch(matt_next_event)
			{
				case E_MQTT_COMMAND:
				{
					if(IsSpaceAvailableInMqttCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						matt_xTicksToWait = 10000;
						xQueueReset( xQueue_MQTT_command);
						xQueueSend( xQueue_MQTT_command, "AT+QMTPUB=0,1,1,0,\"SNES/VTS/POS\"\r\n", 0);
						matt_next_event = E_MQTT_RESPONSE;
					}
					else
					{
						matt_next_event = E_MQTT_COMMAND;
					}
					
				}break;

				case E_MQTT_MESSAGE:
				{
					if(IsSpaceAvailableInMqttCommandQueue())
					{
						vTaskSetTimeOutState( &xTimeOut );
						xQueueReset( xQueue_MQTT_command);
						xQueueSend( xQueue_MQTT_command, "{\"latitude\":\"12.726,34.54\",\"longitude\":\"35.7474,87.33\"\}\x1A", 0);
						matt_next_event = E_MQTT_RESPONSE;
					}
					else
					{
						matt_next_event = E_MQTT_COMMAND;
					}
					
				}break;
								
				case E_MQTT_RESPONSE:
				{
					memset(MQTTReceiveBuffer,0,sizeof(MQTTReceiveBuffer));
					if( true == ReadMQTTResponce(MQTTReceiveBuffer, 0 ) )
					{
						xQueueReset( xQueue_MQTT_response );
						if(strstr(MQTTReceiveBuffer, ">" ) != NULL )
						{
							matt_next_event = E_MQTT_MESSAGE;
							break;
						}
						if(strstr(MQTTReceiveBuffer, "QMTPUB" ) != NULL )
						{
							//break; // This is just for testing intentional Failure Case---Remove this to ensure message is published successfully!!
							matt_next_state = S_MQTT_SUCCESS;
							matt_next_event = E_MQTT_COMMAND;
							break;
						}
					}
					if( xTaskCheckForTimeOut( &xTimeOut, &matt_xTicksToWait ) != pdFALSE )
					{
						matt_next_event = E_MQTT_COMMAND;
						break;
					}
				}break;
								
				default: break;
			}
		}
		break;
				
	default: break;
	}	
	return (matt_next_state == S_MQTT_SUCCESS);				
}

bool SM_Registration()
{
	
}