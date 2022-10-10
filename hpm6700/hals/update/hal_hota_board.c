/*
 * Copyright (c) 2022 HPMicro.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hal_hota_board.h"
#include "stdio.h"
static BOOL HOTA_STATUS = 0;
int HotaHalInit(void)
{
    if (HOTA_STATUS == 1) {
        return OHOS_FAILURE;
    } else {
        HOTA_STATUS = 1;
        return OHOS_SUCCESS;
    }
}

int HotaHalGetUpdateIndex(unsigned int *index)
{
    return OHOS_SUCCESS;
}

int HotaHalDeInit(void)
{
    if (HOTA_STATUS == 0) {
        return OHOS_FAILURE;
    } else {
        HOTA_STATUS = 0;
        return OHOS_SUCCESS;
    }
}

int HotaHalRead(int partition, unsigned int offset, unsigned int bufLen,
                unsigned char *buffer)
{
    return -1;
}

int HotaHalWrite(int partition, unsigned char *buffer, unsigned int offset,
                 unsigned int bufLen)
{
    return OHOS_SUCCESS;
}

int HotaHalRestart(void)
{
    return OHOS_SUCCESS;
}

int HotaHalSetBootSettings(void)
{
    return OHOS_SUCCESS;
}

int HotaHalRollback(void)
{
    return OHOS_SUCCESS;
}

const ComponentTableInfo *HotaHalGetPartitionInfo()
{
    return 0;
}

unsigned char *HotaHalGetPubKey(unsigned int *length)
{
    return 0;
}


int HotaHalGetUpdateAbility(void)
{
    return ABILITY_PKG_SEARCH | ABILITY_PKG_DLOAD;
}

int HotaHalGetOtaPkgPath(char *path, int len)
{
    return OHOS_SUCCESS;
}

int HotaHalIsDeviceCanReboot(void)
{
    return 1;
}

int HotaHalGetMetaData(UpdateMetaData *metaData)
{
    return OHOS_SUCCESS;
}

int HotaHalSetMetaData(UpdateMetaData *metaData)
{
    return OHOS_SUCCESS;
}

int HotaHalRebootAndCleanUserData(void)
{
    return OHOS_SUCCESS;
}

int HotaHalRebootAndCleanCache(void)
{
    return OHOS_SUCCESS;
}

int HotaHalIsDevelopMode(void)
{
    return OHOS_SUCCESS;
}

int HotaHalCheckVersionValid(const char *currentVersion, const char *pkgVersion, unsigned int pkgVersionLength)
{
    return (strncmp(currentVersion, pkgVersion, pkgVersionLength) == 0) ? 1 : 0;
}
