#!/bin/sh

# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# start jb-dev
# 334698 = JRN19
# 342231 = JRN26D
# end jb-dev

source ../../../common/clear-factory-images-variables.sh
BUILD=342231
DEVICE=grouper
PRODUCT=nakasi
VERSION=jrn26d
BOOTLOADERFILE=bootloader.bin
BOOTLOADER=3.23
SLEEPDURATION=10
source ../../../common/generate-factory-images-common.sh
