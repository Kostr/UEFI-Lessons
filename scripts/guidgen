#!/bin/bash
##
# Copyright (c) 2022, Konstantin Aladyshev <aladyshev22@gmail.com>
#
# SPDX-License-Identifier: MIT
##

# Simple script that prints random GUID in string and C-style formats
#
# Example:
# $ ./guidgen
# 3894b412-d616-4760-a428-fd60b4cca24a
# {0x3894b412, 0xd616, 0x4760, {0xa4, 0x28, 0xfd, 0x60, 0xb4, 0xcc, 0xa2, 0x4a}}

which uuidgen > /dev/null
if [ $? -ne 0 ]
then
  echo "Please install 'uuidgen' utility"
  exit 1
fi

UUID=$(uuidgen)
echo ${UUID}
echo "{0x${UUID:0:8}, 0x${UUID:9:4}, 0x${UUID:14:4}, {0x${UUID:19:2}, 0x${UUID:21:2}, 0x${UUID:24:2}, 0x${UUID:26:2}, 0x${UUID:28:2}, 0x${UUID:30:2}, 0x${UUID:32:2}, 0x${UUID:34:2}}}"
