#!/usr/bin/env python

import os
import sys
import socket
import time

print "sys path IS %s" % sys.path
print "PYTHONPATH IS %s" % os.environ["PYTHONPATH"]
print "Imported socket %s" % str(socket)

serversocket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
serversocket.bind(('::', 8092))
serversocket.listen(5) # become a server socket, maximum 5 connections

while True:
    connection, address = serversocket.accept()
    buf = connection.recv(64)
    if len(buf) > 0:
        print buf
        break
    print "Read nothing..."

serversocket.shutdown(socket.SHUT_RDWR)
serversocket.close()
