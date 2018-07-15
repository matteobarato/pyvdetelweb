#!/usr/bin/env python

import socket
import time

time.sleep(2)

clientsocket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
clientsocket.connect(('fd01:8ec4:cef9:87c1::0004', 8092))
clientsocket.send('hello')

time.sleep(1)

clientsocket.close()
