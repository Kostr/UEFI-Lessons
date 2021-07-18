#!/usr/bin/env python3
# Load OVMF debug symbols into gdb
# Author: Artem Nefedov

import gdb
import re
import os
from collections import OrderedDict


def clear_symbols():
    gdb.execute('file')
    gdb.execute('symbol-file')


def remote_connect():
    gdb.execute('target remote :1234')


def update_addresses(base_addr, text_addr, data_addr):
    try:
        print(' Base address ' + base_addr)
        i_base_addr = int(base_addr, 16)
    except:
        print('Failed to locate base address')
        return None

    try:
        print('.text address ' + text_addr)
        print('.data address ' + data_addr)
        i_text_addr = int(text_addr, 16)
        i_data_addr = int(data_addr, 16)
    except:
        print("Failed to locate sections' addresses")
        return None

    return ((i_base_addr + i_text_addr), (i_base_addr + i_data_addr))


def add_symbol_file(file_name, i_text_addr, i_data_addr):
    gdb.execute('add-symbol-file ' + file_name +
                ' ' + hex(i_text_addr) +
                ' -s .data ' + hex(i_data_addr))


def get_pagination():
    out = gdb.execute('show pagination', to_string=True)
    return out.split()[-1].rstrip('.')


class Command_efi(gdb.Command):
    """
    Load all debug symbols for UEFI OVMF image. Image must be build with EDK2.
    Requires debug.log to be located in working directory.
    By default, it will try to load all drivers listed in debug.log.
    Alternatively, you can explicitly specify a list of drives to load.

    Usage: efi [options] [driver_1 driver_2 ...]

    Options:
    -r  - connect to remote target after execution
    -64 - use X64 architecture (default is IA32)
    """

    LOG_FILE = 'debug.log'
    A_PATTERN = r'Loading [^ ]+ at (0x[0-9A-F]{8,}) [^ ]+ ([^ ]+)\.efi'

    def __init__(self):
        super(Command_efi, self).__init__("efi", gdb.COMMAND_OBSCURE)
        self.arch = 'IA32'

    def find_file(self, name):
        # look in working directory first (without subdirectories)
        for f in os.listdir('.'):
            if f == name and os.path.isfile(f):
                return f
        # if nothing is found, look in "Build" directory and subdirectories
        if not os.path.isdir('Build'):
            return None
        for root, dirs, files in os.walk('Build'):
            if name in files and self.arch in root.split(os.sep):
                return os.path.join(root, name)

    def get_addresses(self, file_name):
        gdb.execute('file ' + file_name)
        ok_arch = False
        file_arch = None

        for line in gdb.execute('info files', to_string=True).split('\n'):
            if ' is .text' in line:
                text_addr = line.split()[0]
            elif ' is .data' in line:
                data_addr = line.split()[0]
            elif ' file type ' in line:
                file_arch = line.split()[-1].rstrip('.')
                if file_arch == 'pei-i386' and self.arch == 'IA32':
                    ok_arch = True
                elif file_arch == 'pei-x86-64' and self.arch == 'X64':
                    ok_arch = True

        gdb.execute('file')

        if ok_arch:
            return (text_addr, data_addr)
        else:
            print('Bad file architecture ' + str(file_arch))
            return (None, None)

    def get_drivers(self, drivers):
        print('Looking for addresses in ' + self.LOG_FILE)
        with open(self.LOG_FILE, 'r', errors='ignore') as f:
            for match in re.finditer(self.A_PATTERN, f.read()):
                name = match.group(2)
                if not drivers or name in drivers or name + '.efi' in drivers:
                    yield (match.group(1), name + '.efi')

    def invoke(self, arg, from_tty):
        self.dont_repeat()
        clear_symbols()

        pagination = get_pagination()

        if pagination == 'on':
            print('Turning pagination off')
            gdb.execute('set pagination off')

        if arg:
            drivers = [d for d in arg.split() if not d.startswith('-')]
            if drivers:
                print('Using pre-defined driver list: ' + str(drivers))
            if '-64' in arg.split():
                self.arch = 'X64'
                gdb.execute('set architecture i386:x86-64:intel')
        else:
            drivers = None

        if not os.path.isdir('Build'):
            print('Directory "Build" is missing')

        print('With architecture ' + self.arch)

        files_in_log = OrderedDict()
        load_addresses = {}
        used_addresses = {}

        # if same file is loaded multiple times in log, use last occurence
        for base_addr, file_name in self.get_drivers(drivers):
            files_in_log[file_name] = base_addr

        for file_name in files_in_log:
            efi_file = self.find_file(file_name)

            if not efi_file:
                print('File ' + file_name + ' not found')
                continue

            debug_file = efi_file[:-3] + 'debug'

            if not os.path.isfile(debug_file):
                print('No debug file for ' + efi_file)
                continue

            print('EFI file ' + efi_file)

            if efi_file and debug_file:
                text_addr, data_addr = self.get_addresses(efi_file)

                if not text_addr or not data_addr:
                    continue

                base_addr = files_in_log[file_name]

                prev_used = used_addresses.get(base_addr)

                if prev_used:
                    print('WARNING: duplicate base address ' + base_addr)
                    print('(was previously provided for ' + prev_used + ')')
                    print('Only new file will be loaded')
                    del load_addresses[prev_used]
                else:
                    used_addresses[base_addr] = debug_file

                load_addresses[debug_file] = (
                    update_addresses(base_addr,
                                     text_addr,
                                     data_addr))

        if load_addresses:
            for debug_file in load_addresses:
                add_symbol_file(debug_file, *load_addresses[debug_file])
        else:
            print('No symbols loaded')

        if pagination == 'on':
            print('Restoring pagination')
            gdb.execute('set pagination on')

        if arg and '-r' in arg.split():
            remote_connect()


Command_efi()
