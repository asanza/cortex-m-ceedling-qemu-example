#!/usr/bin/env python3

'''
SPDX-License-Identifier: Apache-2.0 OR MIT

Copyright(c) 2023 Diego Asanza <f.asanza@gmail.com>
'''

import subprocess
import argparse
import sys
import re
import os

qemu = 'qemu-system-arm'
arm_none_eabi_addr2line = 'arm-none-eabi-addr2line'

def addr2line(file, addr):
    client = subprocess.Popen([arm_none_eabi_addr2line, '-e',  file, addr], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    buf = client.stdout.read()
    return buf.decode('ascii', 'ignore').replace('\n', '')

def runQemu(cpu, machine, executable):
    qemu_args = [
        qemu,
        '-cpu', cpu,
        '-M', machine,
        '-nographic',
        '-monitor', 'null',
        '-serial', 'null',
        '-semihosting',
        '-kernel', executable
    ]

    client = subprocess.Popen(qemu_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    buf = client.stdout.read().decode('ascii', 'ignore')

    lines = buf.split('\n')
    fail_pc = '??:0'
    fail_lr = '??:0'
    for line in lines:
        if re.search(r'^.*(Semihosting|Semi-hosting|SYSRESETREQ).*$', line):
            continue

        if re.search(r'LR   :', line):
            fail_lr = addr2line(executable, line.split(':')[1])
            line = line + " ({})".format(fail_lr)

        if re.search(r'PC   :', line):
            fail_pc = addr2line(executable, line.split(':')[1])
            line = line + " ({})".format(fail_pc)

        if(re.search(r'Hard Fault Handler Called', line)):
            if fail_pc != '??:0':
                line = line.replace(':0:', ':{}:'.format(fail_pc.split(':')[1]))
            if fail_lr != '??:0':
                line = line.replace(':0:', ':{}:'.format(fail_lr.split(':')[1]))

        print(line)

        if re.search(r'^OK', line):
            return (0)
    return -1


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='QEMU Test Fixture')

    parser.add_argument('-qemu', default='qemu-system-arm', help='path to the qemu executable')
    parser.add_argument('-cpu', default='cortex-m4', help='QEMU CPU architecture')
    parser.add_argument('-M', default='netduinoplus2', help='QEMU Machine')
    parser.add_argument('-addr2line', default='arm-none-eabi-addr2line', help='Path to addr2line util')
    parser.add_argument('executable', help='executable file path')

    args = parser.parse_args()

    arm_none_eabi_addr2line = args.addr2line
    qemu = args.qemu

    exit(runQemu(args.cpu, args.M, args.executable))