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



#ifndef __SDK_IMPL_INTERNAL__
#define __SDK_IMPL_INTERNAL__

#if defined(__cplusplus)
extern "C" {
#endif

#include "../../lib_aliot_v2/sdk-impl/iot_import.h"
#include "../../lib_aliot_v2/sdk-impl/iot_export.h"

#include "../../lib_aliot_v2/packages/LITE-log/lite-log.h"
#include "../../lib_aliot_v2/packages/LITE-utils/lite-utils.h"
#include "../../lib_aliot_v2/guider/guider.h"

#define POINTER_SANITY_CHECK(ptr, err) \
    do { \
        if (NULL == (ptr)) { \
            log_err("Invalid argument, %s = %p", #ptr, ptr); \
            return (err); \
        } \
    } while(0)

#define STRING_PTR_SANITY_CHECK(ptr, err) \
    do { \
        if (NULL == (ptr)) { \
            log_err("Invalid argument, %s = %p", #ptr, (ptr)); \
            return (err); \
        } \
        if (0 == strlen((ptr))) { \
            log_err("Invalid argument, %s = '%s'", #ptr, (ptr)); \
            return (err); \
        } \
    } while(0)

#if defined(__cplusplus)
}
#endif
#endif  /* __SDK_IMPL_INTERNAL__ */
