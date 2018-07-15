#!/usr/bin/env python

from pycotcp.pycotcp import Pyco
from pycotcp.pycotcp import DeviceInfo
import pycotcp.socket as socket
import time

ioth = Pyco()
device = DeviceInfo().with_type("vde") \
        .with_name("stotest") \
        .with_path("/tmp/vde.ctl") \
        .with_address("fd01:8ec4:cef9:87c1::0002") \
        .with_netmask("ffff:ffff:ffff:ffff::") \
        .create()

ioth.start_handler()

clientsocket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
clientsocket.connect(('fd01:8ec4:cef9:87c1::0003', 8092))

time.sleep(1)
count_sent = clientsocket.write('hello world!')
#count_sent = clientsocket.send('hello world!')
#count_sent = clientsocket.sendall(memoryview("hello world"))
#count_sent = clientsocket.sendto('hello world!', ('10.40.0.153', 8092))
print "I sent %d bytes!" % count_sent

time.sleep(1)

clientsocket.close()
