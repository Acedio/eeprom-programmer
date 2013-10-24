#!/bin/python

import serial
import argparse

parser = argparse.ArgumentParser(description='Interact with the eeprom.')
reqd = parser.add_mutually_exclusive_group(required=True)
reqd.add_argument('--clear', action='store_true')
reqd.add_argument('-f')
args = parser.parse_args()

ser = serial.Serial("/dev/ttyUSB0", 115200)

waiting = True

while waiting:
    line = ser.readline()
    print("waiting ... ")
    print(line)
    if line[0:9] == b'eepeepack':
        waiting = False

ser.write(b'eepeepack')

if args.clear:
    ser.write(b'c')
elif args.f != None:
    pass

while True:
    line = ser.readline()
    print(line.decode("utf-8"),end='')
