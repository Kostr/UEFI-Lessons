#!/bin/bash

##
#  This is a simple script that generates a random 4-byte hex value for a PCD Token
##

hexdump -vn4 -e'"0x%08X\n"' /dev/urandom

