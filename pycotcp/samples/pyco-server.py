#!/usr/bin/env python

from pycotcp.pycotcp import Pyco
from pycotcp.pycotcp import DeviceInfo
import pycotcp.socket as socket
import time

ioth = Pyco()
device = DeviceInfo().with_type("vde") \
        .with_name("vde0") \
        .with_path("/tmp/vde.ctl") \
        .with_address("10.40.0.153") \
        .with_netmask("255.255.255.0") \
        .create()

ioth.start_handler()

serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.bind(('0.0.0.0', 8092))
serversocket.listen(16) # become a server socket, maximum 5 connections

while True:
    print "Accepting..."
    connection, address = serversocket.accept()
    print "Accept DONE, connection: %s, address: %s" % (connection.name, address)
    print "Accepted connection from %s" % connection.name
    buf = connection.recv(64)
    if len(buf) > 0:
        print buf
        break
    print "Read nothing..."

serversocket.shutdown(socket.SHUT_RDWR)
serversocket.close()
ioth.stop_handler()
