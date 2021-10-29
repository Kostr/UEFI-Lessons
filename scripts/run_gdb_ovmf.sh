#!/bin/bash
##
# Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
#
# SPDX-License-Identifier: MIT
##

##### Controllable parameters #####
QEMU_SHARED_FOLDER=~/UEFI_disk
###################################

function show_help {
  echo "Description:"
  echo "  run_gdb_ovmf.sh is a script that helps to debug OVMF"
  echo ""
  echo "Usage: run_gdb_ovmf.sh [-1] [-q <dir>]"
  echo "  -1            This is a first run of this configuration"
  echo "                (in this case before main gdb launch there would be another QEMU start that will create 'debug.log' file)"
  echo "  -q <dir>      QEMU shared directory"
  echo "                (by default it is equal to QEMU_SHARED_FOLDER variable in the head of the script)"
  echo ""
  echo "Examples:"
  echo " run_gdb_ovmf.sh -1      - create 'debug.log' file with the necessary address information"
  echo "                           and debug OVMF it with gdb"
  echo " run_gdb_ovmf.sh         - debug OVMF with gdb ('debug.log' was created in the last run, no need to remake it again)"
}


# A POSIX variable
OPTIND=1         # Reset in case getopts has been used previously in the shell.

while getopts "h?1q:" opt; do
  case "$opt" in
    h|\?)
      show_help
      exit 0
      ;;
    1)  FIRST_RUN=1
      ;;
    q)  QEMU_SHARED_FOLDER=$OPTARG
      ;;
  esac
done

shift $((OPTIND-1))

[ "${1:-}" = "--" ] && shift



function test_file {
  FILE_NAME=$1
  if [[ ! -f ${FILE_NAME} ]]; then
    echo "Error! There is no file ${FILE_NAME}"
    exit 1;
  fi
}

OVMF="Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd"

test_file "${OVMF}"


if [[ ! -z "${FIRST_RUN}" || ! -f debug.log ]]; then
  touch debug.log
  # If it is a first run, we need to create 'debug.log' file for addresses
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
             -s" C-m Enter \;
fi

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
            -s -S" C-m Enter \; \
 select-pane -t 0 \; \
 send-keys "efi -64" Enter \; \
 send-keys "target remote :1234" Enter \;

 
