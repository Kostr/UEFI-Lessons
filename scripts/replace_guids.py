from shutil import copyfile

GUIDS_FILE_PATH = "Build/OvmfX64/DEBUG_GCC5/FV/Guid.xref"
EXTRA_GUIDS_FILE_PATH = ""
LOG_IN_FILE_PATH = "debug.log"
LOG_OUT_FILE_PATH = "debug_parsed.log"

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

copyfile(LOG_IN_FILE_PATH, LOG_OUT_FILE_PATH)

f = open(LOG_OUT_FILE_PATH, 'r')
filedata = f.read()
f.close()

for key,val in guids.items():
    filedata = filedata.replace(key, val)

f = open(LOG_OUT_FILE_PATH, 'w')
f.write(filedata)
f.close()
