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


#include "hardware/lite_gprs_network.h"
#include "lite-log.h"
#include "iot_import.h"
#include "mico.h"


uintptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    log_info("host:%s\r\n\t\t\t\t\tpost:%d....................................................", host, port);
   if(lite_mqtt_net_connect(host, port)){
       log_info("Establish tcp network error!");
       return -1;
   }
   log_info("Connect success");
   return 1;
}


int32_t HAL_TCP_Destroy(uintptr_t fd)
{
    lite_mqtt_nettype_disconnect(NULL);
    return 0;
}


int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    return lite_mqtt_nettype_write(NULL, buf, len, timeout_ms);
}


int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    return lite_mqtt_nettype_read(NULL, buf, len, timeout_ms);
}
