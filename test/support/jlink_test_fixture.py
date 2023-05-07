#!/usr/bin/env python3

'''
File: jlink_test_fixture.py

Copyright (c) 2019 Diego Asanza <f.asanza@gmail.com>

SPDX-License-Identifier: Apache-2.0
'''

import subprocess
import argparse
import os
import re
import sys
import threading
from shutil import which
import time

jlink_gdbserver = 'JLinkGDBServer'
arm_none_eabi_gdb = 'arm-none-eabi-gdb'
arm_none_eabi_addr2line = "arm-none-eabi-addr2line"

def addr2line(file, addr):
    client = subprocess.Popen([arm_none_eabi_addr2line, '-e',  file, addr], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    buf = client.stdout.read()
    return buf.decode('ascii', 'ignore').replace('\n', '')


def startGdbClient(name, gdbinit, executable, port):

    gdbargs = [arm_none_eabi_gdb,
        '-q', '-batch',
        '-ex', 'target remote localhost:' + port,
        '-ex', 'mon reset 0',
        '-ex', 'monitor semihosting enable',
        '-ex', 'monitor semihosting thumbswi 0xab',
        '-ex', 'monitor semihosting IOClient 2',
        '-ex', 'load',
        '-ex', 'continue',
        '-ex', 'quit',
        executable
        ]

    if gdbinit != None:
        gdbargs.append('-x')
        gdbargs.append(gdbinit)

    try:
        gdbClient = subprocess.Popen(gdbargs, stdout=fnull, stderr=subprocess.PIPE)
    except FileNotFoundError:
        print('Error: ' + arm_none_eabi_gdb + ' not found')
        print('Check that arm gnu tools are installed and available in your path\n')
        print_failure()
        exit(1)

    buf = gdbClient.stderr.read()
    buf = buf.decode('ascii', 'ignore')

    if re.search(r'Connection timed out', buf):
        print("Connection Timed Out")
        return -1

    lines = buf.split('\n')

    for line in lines:
        line = line.replace('\x00', '')
        line = line.replace('\r', '')
        line = line.replace('`', '')
        if re.search(r'^.*(Semihosting|Semi-hosting|SYSRESETREQ).*$', line):
            continue
        print(line, end='')

        if re.search(r'LR   :', line):
            print(" ({})".format(addr2line(executable, line.split(':')[1])), end='')

        if re.search(r'PC   :', line):
            print(" ({})".format(addr2line(executable, line.split(':')[1])), end='')

        print('')
        if re.search(r'^OK', line):
            return 0
    return 0


def print_failure():
    print('-----------------------')
    print('0 Tests 0 Failures 0 Ignored')
    print('FAIL')


def startJlinkGdbServer(name, iface, device, debug_iface, speed, port):
    try:
        gdbServer = subprocess.Popen([name,
        '-select', iface,
        '-device', device,
        '-if',     debug_iface,
        '-speed',  speed,
        '-noir',
        '-port',   port,
        '-silent',
        '-singlerun',
        '-nogui',
        '-strict'
        ], stdout = subprocess.PIPE, stderr = subprocess.PIPE)

    except FileNotFoundError:
        print('Error: ' + name + ' not found')
        print('Check that JLink Utilities are installed and available in your path\n')
        print_failure()
        return -1

    buf = gdbServer.stderr.read().decode('ascii', 'ignore')
    if re.search(r'Could not connect', buf):
        print('Error starting JLink:')
        print(buf)
        print_failure()
        return -1

    return 0


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='JLink Debugger Test Fixture')

    parser.add_argument('-select', default='USB')
    parser.add_argument('-iface', default='SWD', help='SWD or JTAG interface')
    parser.add_argument('-speed', default='auto', help='Interface speed or auto')
    parser.add_argument('-port', default='3333', help='GDBServer port')
    parser.add_argument('-jlinkpath', default=None, help='Path to jlink tools')
    parser.add_argument('-gdbpath', default=None, help='Path to gdb')
    parser.add_argument('executable', help='executable file path')
    parser.add_argument('-gdbinit', help='gdb init script file path')

    required = parser.add_argument_group('required named arguments')
    required.add_argument('-device', help='Microcontroller device. See JLINK Doc', required=True)

    args = parser.parse_args()

    fnull = open(os.devnull, 'w')

    if args.jlinkpath == None:
        if os.name == 'nt':
            jlink_gdbserver = jlink_gdbserver + 'CL.exe'
        else:
            jlink_gdbserver = jlink_gdbserver

    if args.gdbpath == None:
        if os.name == 'nt':
            arm_none_eabi_gdb = arm_none_eabi_gdb + '.exe'
    else:
        arm_none_eabi_gdb = args.gdbpath

    t = threading.Thread(target=startJlinkGdbServer,
        args = (jlink_gdbserver, args.select, args.device, args.iface,
            args.speed, args.port))

    t.start()

    # Give some time for the gdbserver to fully startup.
    time.sleep(.001)

    if not t.is_alive():
        # gdb server died really fast... exit with error
        print("Error: Could not start gdbserver")
        exit( -1 )

    # start the client
    rval = startGdbClient(arm_none_eabi_gdb, args.gdbinit, args.executable, args.port)

    exit( rval )
