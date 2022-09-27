#!/bin/sh

# Copyright (c) 2022 HPMicro.
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

cd hpm_sdk

rm -rf .git doc samples cmake scripts env.cmd  env.sh  index.md  index_zh.md README.md  README_zh.md
cd middleware
rm -rf coremark  fatfs  FreeRTOS   libjpeg-turbo  littlevgl  lwip  tflm  tinycrypt  tinyusb erpc segger_rtt
cd ..

cd ..






