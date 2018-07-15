#!/usr/bin/env python

from pycotcp.pycotcp import Pyco
from pycotcp.pycotcp import DeviceInfo
from pycotcp.lwipv6adapter import Lwipv6Adapter
import pycotcp.socket as socket
import time

context = Lwipv6Adapter()

ioth = Pyco(context=context)
device = DeviceInfo(context=context).with_type("vde") \
        .with_name("stotest") \
        .with_path("/tmp/vde.ctl") \
        .with_address("fd01:8ec4:cef9:87c1::0002") \
        .with_netmask("ffff:ffff:ffff:ffff::") \
        .create()

time.sleep(2)

clientsocket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM, context=context)
clientsocket.connect(('fd01:8ec4:cef9:87c1::0003', 8092))
print "Sending data..."
clientsocket.send('hello')

time.sleep(2)

clientsocket.close()

time.sleep(2)
