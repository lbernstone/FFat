#!/usr/bin/python3

import binascii
import struct

buf = open("/tmp/12Mstatehead",'rb').read()
buf = (binascii.crc32(buf, 0xFFFFFFFF) & 0xFFFFFFFF)
print("%08X" % buf)
print(buf.to_bytes(4, byteorder='little', signed=False))
