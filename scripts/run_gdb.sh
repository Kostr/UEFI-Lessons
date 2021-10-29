#!/bin/bash
##
# Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
#
# SPDX-License-Identifier: MIT
##

##### Controllable parameters #####
QEMU_SHARED_FOLDER=~/UEFI_disk
PACKAGE=UefiLessonsPkg
###################################

function show_help {
  echo "Description:"
  echo "  run_gdb.sh is a script that helps to debug UEFI shell applications and drivers"
  echo ""
  echo "Usage: run_gdb.sh -m <module> [-1|-f|-p <package>|-q <dir>]"
  echo "  -1            This is a first run of this configuration"
  echo "                (in this case before main gdb launch there would be another QEMU start that will create 'debug.log' file)"
  echo "  -f            Load all debug symbols"
  echo "                (this will load all OVMF debug symbols - with this you could step inside OVMF functions)"
  echo "  -m <module>   UEFI module to debug"
  echo "  -p <package>  UEFI package to debug"
  echo "                (by default it is equal to PACKAGE variable in the head of the script)"
  echo "  -q <dir>      QEMU shared directory"
  echo "                (by default it is equal to QEMU_SHARED_FOLDER variable in the head of the script)"
  echo ""
  echo "Examples:"
  echo " run_gdb.sh -1 -m MyApp      - create 'debug.log' file with the necessary address information for the 'MyApp'"
  echo "                               and debug it with gdb"
  echo " run_gdb.sh -1 -m MyApp -f   - create 'debug.log' file with the necessary address information for the 'MyApp'"
  echo "                               and debug it with gdb (all debug symbols are included, i.e. you can step into OVMF functions)"
  echo " run_gdb.sh -m MyApp         - debug 'MyApp' with gdb ('debug.log' was created in the last run, no need to remake it again)"
}


# A POSIX variable
OPTIND=1         # Reset in case getopts has been used previously in the shell.

while getopts "h?1fm:q:p:" opt; do
  case "$opt" in
    h|\?)
      show_help
      exit 0
      ;;
    1)  FIRST_RUN=1
      ;;
    f)  FULL=1
      ;;
    m)  TARGET=$OPTARG
      ;;
    q)  QEMU_SHARED_FOLDER=$OPTARG
      ;;
    p)  PACKAGE=$OPTARG
      ;;
  esac
done

shift $((OPTIND-1))

[ "${1:-}" = "--" ] && shift



if [[ ! -z "${FULL}" ]]; then
  DRIVERS=''
else
  DRIVERS=${TARGET}
fi

if [[ -z $TARGET ]]; then
  echo "Error! Module is not provided."
  echo ""
  show_help
  exit 1
fi

function test_file {
  FILE_NAME=$1
  if [[ ! -f ${FILE_NAME} ]]; then
    echo "Error! There is no file ${FILE_NAME}"
    exit 1;
  fi
}

TARGET_INF="${PACKAGE}/${TARGET}/${TARGET}.inf"
TARGET_C="${PACKAGE}/${TARGET}/${TARGET}.c"

OVMF="Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd"

test_file "${TARGET_INF}"
test_file "${TARGET_C}"
test_file "${OVMF}"

ENTRY_POINT_NAME=$(grep ENTRY_POINT ${TARGET_INF} | cut -f 2 -d "=")
if [ ${ENTRY_POINT_NAME} == "ShellCEntryLib" ]; then
  ENTRY_POINT_NAME="ShellAppMain"
fi
ENTRY_POINT_LINE=$(grep -n ${ENTRY_POINT_NAME} ${TARGET_C} | cut -f 1 -d ":")

MODULE_TYPE=$(grep MODULE_TYPE ${TARGET_INF} | cut -f 2 -d "=")
if [ ${MODULE_TYPE} == "UEFI_DRIVER" ]; then
  LAUNCH_COMMAND="load fs0:${TARGET}.efi"
else
  LAUNCH_COMMAND="fs0:${TARGET}.efi"
fi

if [[ ! -z "${FIRST_RUN}" || ! -f debug.log ]]; then
  touch debug.log
  # If it is a first run, we need to create 'debug.log' file for addresses
  TARGET_EFI="Build/${PACKAGE}/RELEASE_GCC5/X64/${TARGET}.efi"
  test_file "${TARGET_EFI}"
  cp ${TARGET_EFI} ${QEMU_SHARED_FOLDER}
  tmux new-session \; \
   send-keys "tail -f debug.log" Enter \; \
   split-window -v \; \
   send-keys "qemu-system-x86_64 \
             -drive if=pflash,format=raw,readonly,file=${OVMF} \
             -drive format=raw,file=fat:rw:${QEMU_SHARED_FOLDER} \
             -net none \
             -nographic \
             -global isa-debugcon.iobase=0x402 \
             -debugcon file:debug.log \
             -s" C-m Enter \; \
   send-keys C-m Enter \; \
   send-keys "${LAUNCH_COMMAND}" Enter \;
fi


test_file "${QEMU_SHARED_FOLDER}/${TARGET}.efi"
touch debug_temp.log
tmux new-session \; \
 send-keys "gdb -ex 'source efi.py' -tui" Enter \; \
 split-window -h \; \
 send-keys "tail -f debug_temp.log" Enter \; \
 split-window -v \; \
 send-keys "qemu-system-x86_64 \
            -drive if=pflash,format=raw,readonly,file=${OVMF} \
            -drive format=raw,file=fat:rw:${QEMU_SHARED_FOLDER} \
            -net none \
            -nographic \
            -global isa-debugcon.iobase=0x402 \
            -debugcon file:debug_temp.log \
            -s" C-m Enter \; \
 select-pane -t 0 \; \
 send-keys "efi -64 ${DRIVERS}" Enter \; \
 send-keys "b ${TARGET_C}:${ENTRY_POINT_LINE}" Enter \; \
 send-keys Enter \; \
 send-keys "target remote :1234" Enter \; \
 send-keys "c" Enter \; \
 select-pane -t 2 \; \
 send-keys C-m Enter \; \
 send-keys "${LAUNCH_COMMAND}" Enter \; 

 
