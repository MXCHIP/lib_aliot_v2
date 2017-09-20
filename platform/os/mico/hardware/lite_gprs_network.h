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


#ifndef __LITE_GPRS_NETWORK_H_
#define __LITE_GPRS_NETWORK_H_

#include "mico.h"


/*******************************************lite_platform_uart.c*********************************************************/

/*
    send uart data
*/
int lite_uart_send(char *pSendBuffer, uint32_t bufferLen);

/*
    receive uart data
*/
int lite_uart_recv(char *pRecvBuffer, uint32_t bufferLen,  uint32_t timeOutMs);


/*
    initialize the baud rate of the uart
*/
int32_t lite_uart_init(void);

/*
     the length of the data readable inside uart buffer
*/
int32_t lite_uart_buffer_len(void);


/*************************************************lite_mqtt_nettype.c******************************************************/
typedef struct Network Network;
struct Network
{
    char *pHostAddress;                    ///< Pointer to a string defining the endpoint for the MQTT service
    uint16_t pHostPort;                    ///< MQTT service listening port
    uint16_t ppubkey_len;                  ///<CA file length
    char *pPubKey;                         ///< Pointer to a string defining the Root CA file (full file, not path)

    int my_socket;                                                  /**< Connect the socket handle. */
    int (*mqttread)(Network *,  char *, uint32_t, uint32_t);        /**< Read data from server function pointer. */
    int (*mqttwrite)(Network *, const char *, uint32_t, uint32_t);  /**< Send data to server function pointer. */
    int (*disconnect)(Network *);                                  /**< Disconnect the network function pointer.婵縿鍊曢崵閬嶅极閻х湋ose socket闁告艾閰ｅ〒鍓佹啺娴ｇ鐏ュ┑顔碱儏鐎靛弶绋夐敓锟�-1闁挎稑鑻々褔寮稿锟界拹锟�-1闁告帗鐟ょ粭澶愬礃瀹ュ棗鈷旈悶娑樼暠lose闁瑰灝绉崇紞锟�*/
    int (*mqttConnect)(Network *);
};

int32_t lite_phy_net_connect();

int32_t lite_mqtt_net_connect(const char *pHost, uint16_t pPort);

int32_t lite_mqtt_nettype_read(Network *pNet, char *pRecvBuffer, uint32_t recvBufferlen, uint32_t timeOutMs);

int32_t lite_mqtt_nettype_write(Network *pNet, const char *pSendBuffer, uint32_t sendBufferlen, uint32_t timeOutMs);

int32_t lite_mqtt_nettype_connect(Network *pNet);

void lite_mqtt_nettype_disconnect(Network *pNet);

#endif

