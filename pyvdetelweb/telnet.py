import pycotcp.socket as socket
TBUFFER = 1024
PORT = 23

def telnet (mgmt):
    global TBUFFER

    #use PycoTCP or LWIPv6 socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, context=mgmt.SYS_INFO['stack_context'])

    try:
        s.bind((mgmt.SYS_INFO['config']['ip'], PORT))
    except socket.error as msg:
        print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()

    print 'Socket bind complete'

    #Start listening on socket
    s.listen(10)
    print 'Socket now listening'

    #now keep talking with the client
    try:
        while True:
            #wait to accept a connection - blocking call
            conn, addr = s.accept()
            print "Accept DONE, connection: %s, address: %s" % (conn.name, addr)
            print "Accepted connection from %s" % conn.name

            #REQUEST AUTHENTICATION
            #---------------------
            user = ''
            psw = ''

            conn.sendall('\nUsername:') #send showinfo (welcome message)
            while not user:
                user = conn.recv(TBUFFER).strip()
            conn.sendall('Password:') #send showinfo (welcome message)
            while not psw:
                psw = conn.recv(TBUFFER).strip()


            if not mgmt.authenticate(user, psw): # if not authenticate
                conn.sendall('\nCould not verify your account.\r\r\nInccorrect username or password')
            else:
                conn.sendall('\n'+mgmt.showinfo()['text']+'\n') #send showinfo (welcome message)

                #infinite loop so that function do not terminate and thread do not end.
                while True :
                    res = conn.sendall(mgmt.SYS_INFO['terminal_prefix'])
                    reply = ''
                    #Receiving from client
                    data = conn.recv(TBUFFER)
                    print 'recived: '+ data
                    if data.strip() == '__history':
                        reply += mgmt.get_history()
                        conn.sendall(reply+'\n')
                    elif not (data == '\n' or data == '\r\n') :
                        reply += mgmt.send(data)['raw']
                        conn.sendall(reply+'\n')
                    time.sleep(.5)### TODO: altrimenti non funziona
            print "connection CLOSED, connection: %s, address: %s" % (conn.name, addr)
            conn.close()
    finally:
        s.shutdown(socket.SHUT_RDWR)
        s.close()
def pyvdetel2 (mgmt):
    import time
    global TBUFFER

    #use PycoTCP or LWIPv6 socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, context=mgmt.SYS_INFO['stack_context'])

    try:
        s.bind((mgmt.SYS_INFO['config']['ip'], PORT))
    except socket.error as msg:
        print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()

    print 'Socket bind complete'

    #Start listening on socket
    s.listen(10)
    print 'Socket now listening'

    #now keep talking with the client
    try:
        while True:
            #wait to accept a connection - blocking call
            conn, addr = s.accept()
            print "Accept DONE, connection: %s, address: %s" % (conn.name, addr)
            print "Accepted connection from %s" % conn.name

            #infinite loop so that function do not terminate and thread do not end.
            while True :
                res = conn.sendall(mgmt.SYS_INFO['terminal_prefix'])
                data = conn.recv(TBUFFER)
                print data
                res = conn.sendall(data)
            print "connection CLOSED, connection: %s, address: %s" % (conn.name, addr)
            conn.close()
    finally:
        s.shutdown(socket.SHUT_RDWR)
        s.close()
