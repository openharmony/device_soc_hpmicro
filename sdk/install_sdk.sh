#!/bin/sh

# Copyright (c) 2023 HPMicro.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

rm -rf hpm_sdk
mkdir hpm_sdk
unzip hpm_sdk_*.zip -d hpm_sdk

pushd hpm_sdk

rm -vrf .git docs samples cmake scripts env.cmd  env.sh hpm_sdk_version.h.in Jenkinsfile

pushd middleware
rm -vrf azure_rtos CMSIS coremark  erpc fatfs  FreeRTOS  hpm_math libjpeg-turbo  lvgl  lwip  mbedtls \
    microros ptpd rtthread-nano segger_rtt tflm  tinycrypt  tinyusb ucos_iii vglite 
popd

pushd soc
shopt -s extglob
rm -rvf !(ip|CMakeLists.txt|HPM5301|HPM5361|HPM6280|HPM6360|HPM6750|HPM6880) 
popd

rm -vrf soc/*/toolchains/segger
rm -vrf boards/hpm*validation hpm6754evk2
rm -vrf boards/openocd/boards/hpm*validation*.cfg
rm -vrf boards/openocd/boards/hpm6750rd.cfg


popd






