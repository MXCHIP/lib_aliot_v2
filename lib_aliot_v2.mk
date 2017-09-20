############################################################################### 
#
#  The MIT License
#  Copyright (c) 2016 MXCHIP Inc.
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy 
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights 
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is furnished
#  to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
#  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
############################################################################### 

NAME := Lib_Aliot_V2.0

GLOBAL_INCLUDES += .\
					guider\
					mqtt\
					mqtt/MQTTPacket\
					platform/os/mico/hardware\
					sdk-impl\
					sdk-impl/exports\
					sdk-impl/imports\
					system\
					utils/digest\
					utils/misc\


$(NAME)_SOURCES :=  guider/guider.c\
					mqtt/mqtt_client.c\
					mqtt/MQTTPacket/MQTTConnectClient.c\
					mqtt/MQTTPacket/MQTTDeserializePublish.c\
					mqtt/MQTTPacket/MQTTPacket.c\
					mqtt/MQTTPacket/MQTTSerializePublish.c\
					mqtt/MQTTPacket/MQTTSubscribeClient.c\
					mqtt/MQTTPacket/MQTTUnsubscribeClient.c\
					platform/os/mico/HAL_OS_mico.c\
					platform/os/mico/HAL_TCP_mico.c\
					platform/os/mico/hardware/lite_mqtt_nettype.c\
					platform/os/mico/hardware/lite_platform_uart.c\
					sdk-impl/sdk-impl.c\
					system/ca.c\
					system/device.c\
					utils/digest/utils_base64.c\
					utils/digest/utils_hmac.c\
					utils/digest/utils_md5.c\
					utils/digest/utils_sha1.c\
					utils/misc/utils_epoch_time.c\
					utils/misc/utils_httpc.c\
					utils/misc/utils_list.c\
					utils/misc/utils_net.c\
					utils/misc/utils_timer.c\


$(NAME)_COMPONENTS :=  lib_aliot_v2/packages\
					   #libraries/utilities
					


