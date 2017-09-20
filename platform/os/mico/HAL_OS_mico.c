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



#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lite-log.h"
#include "iot_import.h"
#include "mico.h"
#include "iot_import.h"

void *HAL_MutexCreate(void)
{
    OSStatus err = kNoErr;
    mico_mutex_t mutex;

    err = mico_rtos_init_mutex( &mutex );
    if(err != kNoErr){
    	log_err("create mutex error");
        return NULL;
    }

    return mutex;
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
    if (0 != mico_rtos_deinit_mutex( &mutex )) {
    	log_err("destroy mutex error");
    }
}

void HAL_MutexLock(_IN_ void *mutex)
{
    if (0 != mico_rtos_lock_mutex( &mutex )) {
    	log_err("lock mutex error");
    }
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
    if (0 != mico_rtos_unlock_mutex( &mutex )) {
    	log_err("unlock mutex error");
    }
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return malloc(size);
}

void HAL_Free(_IN_ void *ptr)
{
    return free(ptr);
}

uint32_t HAL_UptimeMs(void)
{
    return mico_rtos_get_time( );
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
    mico_thread_msleep(ms);
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
    extern mico_mutex_t stdio_tx_mutex;
    static char zc_log_buf[512];
    va_list args;
    int len;

    mico_rtos_lock_mutex( &stdio_tx_mutex );
    va_start( args, fmt );
    len = vsnprintf( zc_log_buf, sizeof(zc_log_buf), fmt, args );
    va_end( args );

    if ( len > 511 )
    {
        len = 511;
    }
    printf("%s\r\n", zc_log_buf);
    mico_rtos_unlock_mutex( &stdio_tx_mutex );
}

char *HAL_GetPartnerID(char pid_str[])
{
    return NULL;
}
