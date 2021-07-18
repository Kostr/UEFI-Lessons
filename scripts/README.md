# Usefull custom scripts to ease working with edk2

- Create UEFI modules with templates:

[createNewApp.sh](createNewApp.sh) - create new UEFI_APPLICATION by name

[createNewDriver.sh](createNewDriver.sh) - create new UEFI_DRIVER by name

- Replace GUIDs in log file:

[replace_guids.py](replace_guids.py) - script that translates GUIDs in 'debug.log' file to readable strings

[Guid_extra.xref](Guid_extra.xref) - extra GUIDs for 'replace_guids.py' script

- GDB:

[efi.py](efi.py) - script by Artem Nefedov that helps to load debug symbols in GDB (https://github.com/artem-nefedov/uefi-gdb/blob/master/efi.py)

[run_gdb.sh](run_gdb.sh) - script that launches tmux session ready for UEFI shell apllication/driver debug

[run_gdb_ovmf.sh](run_gdb_ovmf.sh) - script that launches tmux session ready for OVMF debug

