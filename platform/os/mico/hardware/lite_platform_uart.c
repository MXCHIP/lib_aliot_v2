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


#include "lite_gprs_network.h"
#include "iot_import.h"
#include "mico.h"

#define UART_RX_BUFFER_LEN 2048
static ring_buffer_t  uart_ring_buffer;
static uint8_t  uart_ring_data[UART_RX_BUFFER_LEN] = {0};

int lite_uart_send( char *pSendBuffer, uint32_t bufferLen)
{
	return MicoUartSend(UART_FOR_NBIOT, pSendBuffer, bufferLen);
}

int lite_uart_recv(char *pRecvBuffer, uint32_t bufferLen,  uint32_t timeOutMs)
{
	return MicoUartRecv(UART_FOR_NBIOT, pRecvBuffer, bufferLen, timeOutMs);
}

int32_t lite_uart_init(void)
{
	mico_uart_config_t uart_config = {0};

	uart_config.baud_rate	 = 115200;
	uart_config.data_width	 = DATA_WIDTH_8BIT;
	uart_config.parity		 = NO_PARITY;
	uart_config.stop_bits	 = STOP_BITS_1;
	uart_config.flow_control = FLOW_CONTROL_DISABLED;

	/* enable power supply */
	MicoGpioInitialize( MICO_GPIO_POWER, OUTPUT_PUSH_PULL );
	MicoGpioOutputHigh(MICO_GPIO_POWER);

    MicoGpioInitialize( MICO_GPIO_GPRS_START, OUTPUT_PUSH_PULL );
    MicoGpioOutputHigh(MICO_GPIO_GPRS_START);
    mico_rtos_thread_msleep(100);
    MicoGpioOutputLow(MICO_GPIO_GPRS_START);

    MicoGpioInitialize( MICO_GPIO_GPRS_RST, OUTPUT_PUSH_PULL );
    MicoGpioOutputHigh(MICO_GPIO_GPRS_RST);
    mico_rtos_thread_msleep(4000);
    MicoGpioOutputLow(MICO_GPIO_GPRS_RST);

	//while(1);
	ring_buffer_init(&uart_ring_buffer, (uint8_t *)uart_ring_data, UART_RX_BUFFER_LEN);
	return MicoUartInitialize( UART_FOR_NBIOT, &uart_config, &uart_ring_buffer );
}

/*
	Read the length of the data that is already recived by uart driver and stored in buffer
*/
int32_t lite_uart_buffer_len(void)
{
	return MicoUartGetLengthInBuffer(UART_FOR_NBIOT);
}

