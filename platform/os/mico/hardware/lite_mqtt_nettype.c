/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */



#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lite_gprs_network.h"
#include "iot_import.h"
#include "utils_timer.h"
#include "mico.h"

#define os_uart_mqtt_log(format, ...)  custom_log("uart_test1", format, ##__VA_ARGS__)

#ifndef EMG3062

#else
	//AT+CPIN?
	#define AT_GPRS_CHECK_SIM_CARD_FMT "AT+CPIN?\r\n"

	//AT+CREG?
	#define AT_GPRS_CHECK_REG_NET_FMT "AT+CREG?\r\n"

	//AT+CREG=0
	#define AT_GPRS_DISABLE_REG_NET_FMT "AT+CREG=0\r\n"

	//AT+CGACT?
	#define AT_GPRS_CHECK_GPRS_NET_NORMAL_FMT "AT+CGACT?\r\n"

	//AT+CGATT=1
	#define AT_GPRS_SET_GPRS_NET_NORMAL_FMT "AT+CGACT=1,1\r\n"

	//AT+CGDCONT?
	#define AT_GPRS_QUERY_PDP_CONTEXT_FMT  "AT+CDCONT?\r\n"

	//AT+CGDCONT
	#define AT_GPRS_DEFINE_PDP_CONTEXT_FMT "AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n"

	//AT+CFUN=1 set fully functionality.
	#define AT_GPRS_RESTART_RF_MODULE_FMT "AT+CFUN=1\r\n"        	//has problem.

	//ATE0 close echo function
	#define AT_GPRS_CLOSE_ECHO_FUNC_FMT "ATE0\r\n"

	//AT+CIFSR get local ip address
	#define AT_GPRS_LOOPUP_IP_ADDR_FMT "AT+CIFSR\r\n"

	//AT+RESET
	#define AT_GPRS_RESET "AT+RESET\r\n"

	/*********network manager***********/
	// tcp connect
	// AT+ CIPMUX=<mode>
	// AT+CIPSTART= <type>,<addr>,<port>
	//#define AT_TCP_SOCKET_INIT_FMT "AT+CIPMUX=1\r\n"
	#define AT_TCP_SOCKET_CONNECT_FMT "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n"
	//#define AT_SSL_SOCKET_CONNECT_FMT "AT+CIPSTART=%d,\"SSL\",\"%s\",%d\r\n"

	// tcp fast close
	// AT+CIPCLOSE=<Socket id>
	#define AT_TCP_SOCKET_CLOSE_FMT "AT+CIPCLOSE\r\n"

	//AT+CIPSHUT
	#define AT_TCP_SOCKET_SHUTDOWN_FMT "AT+CIPSHUT\r\n"

	// tcp send data
	// AT+CIPSEND=<link id>,<length>
	#define AT_TCP_SOCKET_SEND_DATA_FMT "AT+CIPSEND\r\n"

	// tcp recv data
	// +IPD,<id>,<len>:<data>
	#define AT_TCP_SOCKET_RECV_DATA_FMT "+IPD,%d,%d:%s"

	//set ip header AT+CIPHEAD=1
	#define AT_TCP_SOCKET_IP_HEADER_SET_FMT "AT+CIPHEAD=1\r\n"

#endif

typedef struct
{
	uint32_t size;
	uint32_t head;
	uint32_t tail;
	uint8_t *buffer;
} lite_ring_buffer_t;

#define IOT_MIN(x,y)  ((x) < (y) ? (x) : (y))
#define MQTT_RING_BUFFER_LEN 512

static lite_ring_buffer_t  mqtt_ring_buffer, recv_ring_buffer;
static uint8_t mqtt_ring_data[MQTT_RING_BUFFER_LEN] = {0};
static int8_t  recv_ring_data[MQTT_RING_BUFFER_LEN] = {0};
static mico_thread_t recv_thread;
static mico_semaphore_t uart_sem = NULL;


/*
	Initializes the size of the ring buffer
*/
int32_t lite_ring_buffer_init( lite_ring_buffer_t *ring_buffer, uint8_t *buffer, uint32_t size )
{
    ring_buffer->buffer     = buffer;
    ring_buffer->size       = size;
    ring_buffer->head       = 0;
    ring_buffer->tail       = 0;
    return 0;
}

/*
	the size of ring buffer is already in use
*/
uint32_t lite_ring_buffer_used_space( lite_ring_buffer_t *ring_buffer )
{
	uint32_t head_to_end = ring_buffer->size - ring_buffer->head;
	return ((head_to_end + ring_buffer->tail) % ring_buffer->size);
}


/*
	read mqtt data from ring buffer
*/
uint32_t lite_ring_buffer_read(lite_ring_buffer_t *ring_buffer, char *data, uint32_t bytes_consume)
{
	uint32_t head_to_end = ring_buffer->size - ring_buffer->head;
	if(bytes_consume < head_to_end)
	{
		memcpy(data, &(ring_buffer->buffer[ring_buffer->head]), bytes_consume);
	}
	else
	{
		memcpy(data, &(ring_buffer->buffer[ring_buffer->head]), head_to_end);
		memcpy((data + head_to_end), ring_buffer->buffer, (bytes_consume - head_to_end));
	}
	ring_buffer->head = (ring_buffer->head + bytes_consume) % ring_buffer->size;
	return 0;
}

/*
	store mqtt data into global ring buffer
*/
uint32_t lite_ring_buffer_write( lite_ring_buffer_t *ring_buffer, const uint8_t *data, uint32_t data_length )
{
	uint32_t tail_to_end = ring_buffer->size - ring_buffer->tail;

	/* Calculate the maximum amount we can copy */
	uint32_t amount_to_copy = IOT_MIN(data_length, (ring_buffer->tail == ring_buffer->head) ? ring_buffer->size : (tail_to_end + ring_buffer->head) % ring_buffer->size);

	/* Copy as much as we can until we fall off the end of the buffer */
	memcpy(&ring_buffer->buffer[ring_buffer->tail], data, IOT_MIN(amount_to_copy, tail_to_end));

	/* Check if we have more to copy to the front of the buffer */
	if (tail_to_end < amount_to_copy)
	{
		memcpy(ring_buffer->buffer, data + tail_to_end, amount_to_copy - tail_to_end);
	}

	/* Update the tail */
	ring_buffer->tail = (ring_buffer->tail + amount_to_copy) % ring_buffer->size;

	return amount_to_copy;
}


//Check AT command return value
int32_t lite_wait_at_ack(char *pAck, int32_t timeOutMs)
{
	int8_t recvBuf[64] = {0};
	int32_t dataLen = 0;

	iotx_time_t timer;
	iotx_time_init(&timer);
	utils_time_countdown_ms(&timer, timeOutMs);

	if(!pAck)
	{
		os_uart_mqtt_log("ack buffer is null");
		return -1;
	}

	do
	{
		//read whole uart buffer data
		dataLen = lite_uart_buffer_len();
		if(dataLen)
		{
			lite_uart_recv(recvBuf, dataLen, timeOutMs);
			os_uart_mqtt_log("ack %s \n", recvBuf);

			if(strstr(recvBuf, pAck))
			{
				os_uart_mqtt_log("match success \r\n");
				return 0;
			}
			else
			{
				HAL_SleepMs(100);
				memset(recvBuf, 0, sizeof(recvBuf));
			}
		}
		else
		{
			HAL_SleepMs(100);
		}
	}while(!utils_time_is_expired(&timer));

	return -1;
}



/*
	send at cmd by uart and check ack value
*/
int32_t lite_send_at_cmd(char *pCmd, uint32_t size, char *pAck, int32_t timeOutMs)
{
	if(!pCmd || !pAck)
	{
		os_uart_mqtt_log("param error, cmd or ack value is null");
		return -1;
	}

	if(lite_uart_send(pCmd, size))
	{
		os_uart_mqtt_log("send at cmd failed %s", pCmd);
		return -1;
	}

	if(lite_wait_at_ack(pAck, timeOutMs))
	{
		os_uart_mqtt_log("wait %s ack failed", pAck);
		return -1;
	}
	return 0;
}

void recv_uart_data_thread( mico_thread_arg_t arg )
{
    UNUSED_PARAMETER( arg );
    OSStatus err = kNoErr;
    uint32_t msgLen = 0;
 	char  uartMsg;
 	char uartMsgBuffer[MQTT_RING_BUFFER_LEN] = {0};
 	int8_t tmp[8] = {0};
 	int32_t pos = 0;
	printf("create thread successed...\r\n");
	//HAL_SleepMs(1000);
    while(1){
    	while(!lite_uart_buffer_len());
	labe:
		lite_uart_recv(&uartMsg, 1, 200);
		//printf("recv data :%x\r\n", uartMsg);
		if(uartMsg != '+')
			goto labe;
		lite_uart_recv(uartMsgBuffer, 3, 200);
		if( !(uartMsgBuffer[0] == 'O' && uartMsgBuffer[1] == 'K' && uartMsgBuffer[2] == '=' ) )
			goto labe;

		lite_uart_recv(&uartMsg, 1, 200);
		do{
				tmp[pos] = uartMsg;
				pos ++;
				lite_uart_recv(&uartMsg, 1, 200);
		}while(uartMsg != ',');

		msgLen = atoi(tmp);

		for(int i=0; i<pos; i++){
			tmp[i] = 0x00;
		}
		pos = 0;
printf("msgLen dataLength:%d\r\n", msgLen);

		err = lite_uart_recv(uartMsgBuffer, msgLen, 200);
		require_noerr_string( err, exit, "can't recv require data");
		lite_ring_buffer_write(&recv_ring_buffer, (const unsigned char*)uartMsgBuffer, msgLen);
		lite_uart_recv(uartMsgBuffer, msgLen+12, 200);
		memset( uartMsgBuffer, 0, msgLen+12 );
		mico_rtos_set_semaphore( &uart_sem );
printf("after recv data.................\r\n");
    }

exit:
	mico_rtos_deinit_semaphore(&uart_sem);
	mico_rtos_delete_thread(NULL);
}

/*
+CGATT: 1 // STATE: IP INITIAL()
AT+CSTT // STATE: IP START()
AT+CIICR // STATE: IP GPRSACT()
AT+CIFSR // STATE: IP STATUS?()
AT+CIPSTART="TCP","116.236.221.75",7015
OK
CONNECT OK // STATE: CONNECT OK()
AT+CIPCLOSE // STATE: TCP CLOSED()
AT+CIPSHUT // STATE: IP INITIAL()

sign up for mobile network and initialize uart serial port
the network is normal when device get ip address
*/
int32_t lite_phy_net_connect()
{
	int8_t cmd[64] = {0};
	int32_t loop = 0;
	OSStatus err =kNoErr;

	mico_rtos_init_semaphore( &uart_sem, 1 );

	//mqtt ring buffer init
	lite_ring_buffer_init(&mqtt_ring_buffer, (uint8_t *)mqtt_ring_data, MQTT_RING_BUFFER_LEN);

	//recv ring buffer init
	lite_ring_buffer_init(&recv_ring_buffer, (uint8_t *)recv_ring_data, MQTT_RING_BUFFER_LEN);

	//uart com port init
	if(lite_uart_init())
	{
		os_uart_mqtt_log("uart init failed");
		return -1;
	}

	//create receive data thread
	err = mico_rtos_create_thread( &recv_thread, MICO_APPLICATION_PRIORITY, "recv_uart_data_thread", recv_uart_data_thread, 0x800, 0 );
	require_noerr_string( err, exit, "ERROR: Unable to start the recv_uart_data thread." );

	mico_rtos_suspend_thread( &recv_thread );
	/* reset module */
//	if(lite_send_at_cmd(AT_GPRS_RESET, 10, "OK", 1000))	//return +CREG:1,1
//	{
//		os_uart_mqtt_log("please register mobile network");
//		return -1;
//	}
//	HAL_SleepMs(3000);
	//check sim card is insert or not
	os_uart_mqtt_log("cmd:%s", AT_GPRS_CHECK_SIM_CARD_FMT);

	while(1);
	loop = 0;
	do
	{
		if(lite_send_at_cmd(AT_GPRS_CHECK_SIM_CARD_FMT, 10, "READY", 5000))  //return +CPIN:READY
		{
			loop++;
			if(loop == 3)
			{
				os_uart_mqtt_log("please check sim card");
				return -1;
			}
		}
		else
		{
			os_uart_mqtt_log("sim card ready");
			break;
		}
	}while(loop < 3);

	//close at echo function
	os_uart_mqtt_log("cmd:%s", AT_GPRS_CLOSE_ECHO_FUNC_FMT);
	if(lite_send_at_cmd("ATE0\r\n", 6, "OK", 1000)){ //return OK
		os_uart_mqtt_log("not receive OK");
	}
	//disable register network unsolicited result code
	os_uart_mqtt_log("cmd:%s", AT_GPRS_DISABLE_REG_NET_FMT);
	if(lite_send_at_cmd(AT_GPRS_DISABLE_REG_NET_FMT, 11, "OK", 1000))	//return +CREG:1,1
	{
		os_uart_mqtt_log("please register mobile network");
		return -1;
	}

	os_uart_mqtt_log("cmd:%s", AT_GPRS_CHECK_REG_NET_FMT);
	if(lite_send_at_cmd(AT_GPRS_CHECK_REG_NET_FMT, 10, "+CREG: 0,1", 1000))	//return +CREG:1,1
	{
		os_uart_mqtt_log("please register mobile network");
		return -1;
	}

	//define PDP conext and set apn
	os_uart_mqtt_log("cmd:%s", AT_GPRS_DEFINE_PDP_CONTEXT_FMT);
	if(lite_send_at_cmd(AT_GPRS_DEFINE_PDP_CONTEXT_FMT, 27, "OK", 1000))	//return +CGREG:1 \r\n OK \r\n +CGRE:1
	{
		os_uart_mqtt_log("set apn failed");
		return -1;
	}

	//check whether gprs network is ok
	os_uart_mqtt_log("cmd:%s", AT_GPRS_SET_GPRS_NET_NORMAL_FMT);

	//activate PDP context.
	if(lite_send_at_cmd(AT_GPRS_SET_GPRS_NET_NORMAL_FMT, 14, "OK", 10000))
	{
		os_uart_mqtt_log("activate PDP context failed");
		return -1;
	}
	if(lite_send_at_cmd(AT_GPRS_CHECK_GPRS_NET_NORMAL_FMT, 11, "+CGACT: 1, 1", 1000))
	{
		os_uart_mqtt_log("activate PDP context failed");
		return -1;
	}

	os_uart_mqtt_log("Init gprs module successfully...........................");

	return 0;
exit:
    return -1;
}

int32_t lite_mqtt_net_connect(const char *pHost, uint16_t pPort)
{
	char cmd[64] = {0};
//  send establish tcp connection at cmd
//	snprintf(cmd, sizeof(cmd), AT_TCP_SOCKET_CONNECT_FMT, "139.196.135.135", pPort);
//	os_uart_mqtt_log("cmd:%s", cmd);
//	lite_mqtt_nettype_disconnect(NULL);
	if(lite_send_at_cmd("AT+CIPSTART=\"TCP\",\"139.196.135.135\",1883\r\n", 41, "CONNECT OK", 5000))
	{
		os_uart_mqtt_log("connect iot cloud failed");
		return -1;
	}

	mico_rtos_resume_thread( &recv_thread );

	return 0;
}

// transplant mode
int32_t lite_mqtt_nettype_read(Network *pNet, char *pRecvBuffer, uint32_t recvBufferlen, uint32_t timeOutMs)
{
 	if(!pRecvBuffer)
 	{
 		os_uart_mqtt_log("param error, recvbuffer is null");
 		return -1;
 	}
 	int32_t readLen = -1;
 	int8_t  uartMsg[MQTT_RING_BUFFER_LEN] = {0};
 	int32_t uartMsgLen = 0;
	int32_t dataLen = 0;
	char * recvBuffer = NULL;
	iotx_time_t timer;
	iotx_time_init(&timer);
	utils_time_countdown_ms(&timer, timeOutMs);
	recvBuffer = pRecvBuffer;

	do{
		dataLen = lite_ring_buffer_used_space(&mqtt_ring_buffer);
		if(dataLen)
		{
			// read mqtt data from ringbuffer
			readLen = ( (dataLen > recvBufferlen) || (dataLen == recvBufferlen) ) ? recvBufferlen : dataLen;
			lite_ring_buffer_read(&mqtt_ring_buffer, recvBuffer, readLen);
			if(readLen == recvBufferlen)
				return readLen;
			else{
				recvBufferlen -= readLen;
				recvBuffer += readLen;
			}
		}
		else // read mqtt data from recv_ring_buffer. then put uart data into mqtt_ring_buffer
		{
			/* get data from recv thread */
		 	mico_rtos_get_semaphore( &uart_sem, timeOutMs );
			uartMsgLen = lite_ring_buffer_used_space(&recv_ring_buffer);
			if(uartMsgLen)
			{
				lite_ring_buffer_read( &recv_ring_buffer, uartMsg, uartMsgLen );
	//os_uart_mqtt_log("recv data:%x %x %x %x", *uartMsg, *(uartMsg+1), *(uartMsg+2),*(uartMsg+3));
				lite_ring_buffer_write(&mqtt_ring_buffer, (const unsigned char*)uartMsg, uartMsgLen);
				readLen = ( (uartMsgLen > recvBufferlen) || (uartMsgLen == recvBufferlen) ) ? recvBufferlen : uartMsgLen;
				lite_ring_buffer_read(&mqtt_ring_buffer, recvBuffer, readLen);
				if(readLen == recvBufferlen)
					return readLen;
				else{
					recvBufferlen -= readLen;
					recvBuffer += readLen;
				}
			}
		}
	}while(!utils_time_is_expired(&timer));

	return -1;
}

int32_t lite_mqtt_nettype_write(Network *pNet, const char *pSendBuffer, uint32_t sendBufferlen, uint32_t timeOutMs)
{
	//os_uart_mqtt_log("mqtt_write: dataLen=%ld timeout=%ld", sendBufferlen, timeOutMs);
	char cmd[64] = {0};
	uint8_t dest[1024];
	memset(dest, 0, sizeof(dest));
	if(!pSendBuffer)
	{
		os_uart_mqtt_log("param error, sendbuffer is null");
		return -1;
	}
//os_uart_mqtt_log("############ write data ###############");
//	for( int i=0; i<sendBufferlen; i++){
//		printf("%x ", *(pSendBuffer + i));
//	}
//	printf("\r\n");

	//send tcp data at cmd
	lite_uart_send(AT_TCP_SOCKET_SEND_DATA_FMT, 12);
	HAL_SleepMs(500);
	memcpy(dest, pSendBuffer, sendBufferlen);
	*(dest+sendBufferlen) = 0x1A;
	if(lite_uart_send((char*)dest, sendBufferlen+1)){
	    return -1;
	}
	//send data and recv OK imediately.
	HAL_SleepMs(1000);
	os_uart_mqtt_log("send data successfully");
	return sendBufferlen;
}

void lite_mqtt_nettype_disconnect(Network *pNet)
{
	int8_t cmd[32] = {0};
	//snprintf(cmd, sizeof(cmd), AT_TCP_SOCKET_CLOSE_FMT);
	lite_uart_send("AT+CIPCLOSE\r\n", 13);
	HAL_SleepMs(3000);
}


