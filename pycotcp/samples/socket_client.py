#!/usr/bin/env python

import socket
import time

time.sleep(2)

clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientsocket.connect(('10.40.0.54', 8092))
clientsocket.send('hello')

time.sleep(1)

clientsocket.close()
