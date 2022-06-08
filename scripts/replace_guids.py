##
# Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
#
# SPDX-License-Identifier: MIT
##

from argparse import ArgumentParser
from shutil import copyfile

parser = ArgumentParser(description="Convert GUIDs to text identifiers in UEFI firmware boot log")
parser.add_argument('-g', '--guids', help="Guid.xref file location", required=True)
parser.add_argument('-e', '--guids_extra', help="additional Guid.xref file location")
parser.add_argument('-i', '--log_input', help="input log file location", required=True)
parser.add_argument('-o', '--log_output', help="output log file location (by default input file is changed in place)")
args = parser.parse_args()

GUIDS_FILE_PATH = args.guids
EXTRA_GUIDS_FILE_PATH = args.guids_extra

LOG_IN_FILE_PATH = args.log_input
if args.log_output:
    LOG_OUT_FILE_PATH = args.log_output
else:
    LOG_OUT_FILE_PATH = args.log_input

guids = {}

with open(GUIDS_FILE_PATH) as p:
    for line in p:
        l = line.split(" ")
        if len(l)==2:
            guids[l[0].upper()] = l[1][:-1]

if EXTRA_GUIDS_FILE_PATH:
    with open(EXTRA_GUIDS_FILE_PATH) as p:
        for line in p:
            l = line.split(" ")
            if len(l)==2:
                guids[l[0].upper()] = l[1][:-1]

if LOG_IN_FILE_PATH != LOG_OUT_FILE_PATH:
    copyfile(LOG_IN_FILE_PATH, LOG_OUT_FILE_PATH)

f = open(LOG_OUT_FILE_PATH, 'r')
filedata = f.read()
f.close()

for key,val in guids.items():
    filedata = filedata.replace(key, val)

f = open(LOG_OUT_FILE_PATH, 'w')
f.write(filedata)
f.close()
