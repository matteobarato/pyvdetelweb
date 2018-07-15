#!/usr/bin/env python

from pycotcp.pycotcp import Pyco
from pycotcp.pycotcp import DeviceInfo
from pycotcp.lwipv6adapter import Lwipv6Adapter
import pycotcp.socket as socket
import time

context = Lwipv6Adapter()

ioth = Pyco(context=context)
device = DeviceInfo(context=context).with_type("vde") \
        .with_name("vde") \
        .with_path("/tmp/vde.ctl") \
        .with_address("10.40.0.55") \
        .with_netmask("255.255.255.0") \
        .create()

serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM, context=context)
serversocket.bind(('10.40.0.55', 8092))
serversocket.listen(5) # become a server socket, maximum 5 connections

while True:
    print "Accepting..."
    connection, address = serversocket.accept()
    print "Accepted connection from %s" % connection.name
    buf = connection.recv(64)
    if len(buf) > 0:
        print buf
        break
    print "Read nothing..."

serversocket.shutdown(socket.SHUT_RDWR)
serversocket.close()
