#!usr/bin/env python
import os
import select
import threading
import traceback
from StringIO import StringIO

try:
    from pycotcp.pycotcp import Pyco
    from pycotcp.pycotcp import DeviceInfo
except ImportError:
    pass
    #TODO these imports should not be needed
    #try:
    #    from .pycotcp import Pyco
    #    from .pycotcp import DeviceInfo
    #except ImportError:
    #    pass

#TODO
import imp
oldsocket = imp.load_source('socket', os.path.dirname(os.__file__) + "/socket.py")
#inet_pton = oldsocket.inet_pton
#print "inet_pton: %s" % str(inet_pton)

error = oldsocket.error
gaierror = oldsocket.gaierror
herror = oldsocket.herror
timeout = oldsocket.timeout

_GLOBAL_DEFAULT_TIMEOUT = object()

AF_INET = 2
AF_INET6 = 10

SOCK_STREAM = 1
SOCK_DGRAM = 2
SOCK_RAW = 3

SHUT_RDWR = "rw"
SHUT_RD = "r"
SHUT_WR = "w"

SOL_SOCKET = 1
SO_REUSEADDR = 2
SO_ERROR = 4

IPPROTO_IP = 0
IPPROTO_UDP = 17
IPPROTO_TCP = 6

TCP_NODELAY = 1

#def fromfd(fd, family, sock_type):
#    """ Gets a socket from file descriptor
#    where are picotcp's sockets?
#    """
#    #TODO find sockets...
#    return None

#socket() create a new socket object
#socketpair() create a pair of new socket objects [*]
#fromfd() create a socket object from an open file descriptor [*]
#gethostname() return the current hostname
#gethostbyname() map a hostname to its IP number
#gethostbyaddr() map an IP number or hostname to DNS info
#getservbyname() map a service name and a protocol name to a port number
#getprotobyname() map a protocol name (e.g. 'tcp') to a number
#ntohs(), ntohl() convert 16, 32 bit int from network to host byte order
#htons(), htonl() convert 16, 32 bit int from host to network byte order
#inet_aton() convert IP addr string (123.45.67.89) to 32-bit packed format
#inet_ntoa() convert 32-bit packed format IP to string (123.45.67.89)
#ssl() secure socket layer support (only available if configured)
#socket.getdefaulttimeout() get the default timeout value
#socket.setdefaulttimeout() set the default timeout value
#create_connection() connects to an address, with an optional timeout and optional source address.

def getfqdn(name=''):
    """Get fully qualified domain name from name.

    An empty argument is interpreted as meaning the local host.

    First the hostname returned by gethostbyaddr() is checked, then
    possibly existing aliases. In case no FQDN is available, hostname
    from gethostname() is returned.
    """
    name = name.strip()
    if not name or name == '0.0.0.0':
        name = gethostname()
    try:
        hostname, aliases, ipaddrs = gethostbyaddr(name)
    except: #error :
        pass
    else:
        aliases.insert(0, hostname)
        for name in aliases:
            if '.' in name:
                break
        else:
            name = hostname
    return name

def socketpair(): #create a pair of new socket objects [*]
    pass

def fromfd(fd, family, type, proto = 0): #create a socket object from an open file descriptor [*]
    print "Looking for socket with data (%s, %s, %s, %s)" % (str(fd), str(family), str(type), str(proto))
    toreturn = Socket(addr_family = family, sock_type = type, from_fd = fd)   #TODO check what proto does, only documented as "0 for default protocol" (maybe ipv4/6?)
    return toreturn

def gethostname(): #return the current hostname
    return "hostname" #TODO get hostname...

def gethostbyname(): #map a hostname to its IP number
    pass

#def gethostbyaddr(): #map an IP number or hostname to DNS info
#    pass

def getservbyname(): #map a service name and a protocol name to a port number
    pass

def getprotobyname(): #map a protocol name (e.g. 'tcp') to a number
    pass

#convert 16, 32 bit int from network to host byte order
def ntohs():
    pass

def ntohl():
    pass

#convert 16, 32 bit int from host to network byte order
def htons():
    pass

def htonl():
    pass

def inet_aton(): #convert IP addr string (123.45.67.89) to 32-bit packed format
    pass

def inet_ntoa(): #convert 32-bit packed format IP to string (123.45.67.89)
    pass

def ssl(): #secure socket layer support (only available if configured)
    pass

def getaddrinfo(host, port, family = AF_INET, socktype = SOCK_STREAM, proto = IPPROTO_TCP, flags = 0):
    #TODO this tuple will not work in most cases (for ex. if host is a domain name)
    return [(family, socktype, proto, None, (host, port))]

def create_connection(address, timeout=_GLOBAL_DEFAULT_TIMEOUT,
                      source_address=None):
    """Connect to *address* and return the socket object.

    Convenience function.  Connect to *address* (a 2-tuple ``(host,
    port)``) and return the socket object.  Passing the optional
    *timeout* parameter will set the timeout on the socket instance
    before attempting to connect.  If no *timeout* is supplied, the
    global default timeout setting returned by :func:`getdefaulttimeout`
    is used.  If *source_address* is set it must be a tuple of (host, port)
    for the socket to bind as a source address before making the connection.
    An host of '' or port 0 tells the OS to use the default.
    """

    host, port = address
    err = None
    for res in getaddrinfo(host, port, 0, SOCK_STREAM):
        af, socktype, proto, canonname, sa = res
        sock = None
        try:
            sock = socket(af, socktype, proto)
            if timeout is not _GLOBAL_DEFAULT_TIMEOUT:
                sock.settimeout(timeout)
            if source_address:
                sock.bind(source_address)
            sock.connect(sa)
            return sock

        except error as _:
            err = _
            if sock is not None:
                sock.close()

    if err is not None:
        raise err
    else:
        raise error("getaddrinfo returns an empty list")

#def socket(addr_family = None, sock_type = None, proto = IPPROTO_TCP, context = None, name = "__", is_ipv6 = False, auto_open = True):
#    return Socket(addr_family, sock_type, context, name, proto, is_ipv6, auto_open)

class Socket:

    default_context = None

    existing_sockets = {}

    TCP = "tcp"
    UDP = "udp"

    VERSION_IPV4 = "ipv4"
    VERSION_IPV6 = "ipv6"

    DEFAULT_NAME_PREFIX = "__"
    DEFAULT_NAME_COUNTER = 0

    SOCK_STREAM = 1
    SOCK_DGRAM = 2

    running = True

    @classmethod
    def from_name(cls, name):
        return Socket.existing_sockets[name]

    @classmethod
    def from_fd(cls, fd):
        #TODO change this
        for key, socket in Socket.existing_sockets:
            if socket.accept_read_fakefd == fd:
                return socket
        return None

    def settimeout(number):
        """ It is inside python's sockets """
        #TODO implement?
        pass

    def __init__(self, addr_family = None, sock_type = None, proto = "tcp", context = None, name = DEFAULT_NAME_PREFIX, is_ipv6 = False, auto_open = True, from_fd = None):
        """ Initializes a socket
        addr_family is currently ignored, as picotcp doesn't expose those parameters... (sock_type SOCK_STREAM SOCK_DGRAM to decide TCP or UDP)
        context is an instance of an adapter and, if missing, defaults to PicoTCPAdapter
        if name is not specified, a name is generated automatically
        """
        ######################################
        #NOTE Context and environment handling
        ######################################
        environ_context = os.environ.get("PYCOTCP_CONTEXT")
        new_context = None

        if Socket.default_context is None and environ_context is not None:
            if environ_context  == "picotcp":
                from picotcpadapter import PicoTCPAdapter
                new_context = PicoTCPAdapter()

            elif environ_context == "lwipv6":
                from lwipv6adapter import Lwipv6Adapter
                new_context = Lwipv6Adapter()

            elif environ_context == "fdpicotcp":
                from fdpicotcpadapter import FDPicoTCPAdapter
                new_context = FDPicoTCPAdapter()

        elif Socket.default_context is not None:
            print "Taking default context %s..." % Socket.default_context
            context = Socket.default_context

        if context is None and new_context is None:
            from picotcpadapter import PicoTCPAdapter

            context = PicoTCPAdapter()
            Socket.default_context = new_context
            self.context = context
        elif context is None and new_context is not None:
            print "Taking context %s from environment..." % environ_context
            self.context = new_context

            devtype = os.environ.get("PYCOTCP_DEVTYPE")
            device = os.environ.get("PYCOTCP_DEVICE")
            filename = os.environ.get("PYCOTCP_FILENAME")
            address = os.environ.get("PYCOTCP_ADDRESS")
            netmask = os.environ.get("PYCOTCP_NETMASK")

            ioth = Pyco(context=new_context)
            device = DeviceInfo(context=new_context).with_type(devtype) \
                    .with_name(device) \
                    .with_path(filename) \
                    .with_address(address) \
                    .with_netmask(netmask) \
                    .create()

            if environ_context == "picotcp":
                ioth.start_handler()

            Socket.default_context = new_context
        else:
            self.context = context
        ######################################

        self.name = name
        self.proto = proto
        self.is_ipv6 = is_ipv6
        self.bound_address = ("0.0.0.0", 0)

        if sock_type == Socket.SOCK_STREAM:
            self.proto = 'tcp'
        elif sock_type == Socket.SOCK_DGRAM:
            self.proto = 'udp'

        if proto == IPPROTO_TCP:
            self.proto = 'tcp'
        elif proto == IPPROTO_UDP:
            self.proto = 'udp'

        #Check if doing "if self.name in Socket.existing_sockets" is better
        if self.name is None or self.name == Socket.DEFAULT_NAME_PREFIX:
            self.name = self.name + str(Socket.DEFAULT_NAME_COUNTER)
            Socket.DEFAULT_NAME_COUNTER = Socket.DEFAULT_NAME_COUNTER + 1

        self.proto_version = "ipv4"
        self.proto_family = AF_INET
        if is_ipv6 or addr_family == AF_INET6:
            self.is_ipv6 = True
            self.proto_version = "ipv6"
            self.proto_family = AF_INET6

        if auto_open:
            self.open(self.proto)

        Socket.existing_sockets[self.name] = self


    def open(self, proto):
        """ Opens the specified socket
        proto is 'udp' or 'tcp'
        self.proto_version is 'ipv4' or 'ipv6'
        self.proto_family is AF_INET or AF_INET6
        """
        print "Opening socket %s %s %s %s." % (self.name, self.proto_version, proto, self.proto_family)
        self.context.socketOpen(self.name, self.proto_version, proto, self.proto_family)

    def close(self):
        print "Closing socket %s" % self.name
        del Socket.existing_sockets[self.name]
        self.context.socketClose(self.name)

    def bind(self, address):
        """ Binds this socket to target address and port """
        print "Binding %s to %s, %s." % (self.name, address[0], address[1])
        self.context.socketBind(self.name, address[0], str(address[1]))
        self.bound_address = address

    def connect(self, address):
        print "Connecting to %s" % str(address)
        self.context.socketConnect(self.name, address[0], str(address[1]))

    def send(self, data):
        tosend = data
        if isinstance(tosend, memoryview):
            print "Send tosend type is a memoryview so doing tobytes"
            tosend = data.tobytes()

        return self.context.socketSend(self.name, tosend, len(data))

    def sendall(self, data):
        tosend = data
        print "Sendall tosend type is %s" % type(tosend).__name__
        if isinstance(tosend, memoryview):
            print "Sendall tosend type is a memoryview so doing tobytes"
            tosend = data.tobytes()

        return self.context.socketSend(self.name, tosend, len(data))

    def sendto(self, data, address):
        tosend = data
        if isinstance(tosend, memoryview):
            tosend = data.tobytes()

        return self.context.socketSendTo(self.name, tosend, len(data), address[0], str(address[1]))

    def write(self, data):
        tosend = data
        if isinstance(tosend, memoryview):
            tosend = data.tobytes()

        return self.context.socketWrite(self.name, tosend, len(data))

    def recvfrom(self, bufsize = 512, moreinfo = False):
        result = None
        if moreinfo:
            result = self.context.socketRecvFromExt(self.name, bufsize, self.proto_version)
        else:
            result = self.context.socketRecvFrom(self.name, bufsize, self.proto_version)
        return (result[0], result[1]) #0: data 1: IP 2: port

    def recv(self, bufsize = 512, flags = None):
        return self.context.socketRecv(self.name, bufsize)

    def listen(self, maxconn):
        print "Listening on %s %d" % (self.name, maxconn)
        return self.context.socketListen(self.name, maxconn)

    def shutdown(self, how):
        return self.context.socketShutdown(self.name, how)

    def accept(self):
        """ Does an accept on the socket
        Blocking operation
        Blocks until a socket is connected to this one
        raises an error() exception in case of error
        """
        result = None

        print "Accepting %s..." % self.name
        result = self.context.socketAccept(self.name)
        if result is None:
            raise error()
            return (result, result)
        print "Socket accepting %s" % str(result)
        return (Socket(name = str(result[0]), auto_open = False, context = self.context), result[1])  #TODO controllare, serve coppia (connection, address)

    def setsockopt(self, arg1, arg2, arg3):
        #TODO find out what this does, if it has a function inside picoTCP and if it does, call that
        pass

    def getsockname(self):
        #TODO is this the expected behaviour?
        return self.bound_address

    def fileno(self):
        #TODO this should be moved to the adapter, as it is library-specific
        #print "Returning fileno %d" % self.accept_read_fakefd
        #os.write(self.accept_write_fakefd, 'a')
        #fd = self.accept_read_fakefd
        fd = self.context.fileno(self.name)
        return fd

    def makefile(self, mode='r', bufsize=-1):
        """makefile([mode[, bufsize]]) -> file object

        Return a regular file object corresponding to the socket.  The mode
        and bufsize arguments are as for the built-in open() function."""
        return _fileobject(self, mode, bufsize)


########################################################################################################
# ORIGINAL SOCKET UTILITIES
########################################################################################################

class _fileobject(object):
    """Faux file object attached to a socket object."""

    default_bufsize = 8192
    name = "<socket>"

    __slots__ = ["mode", "bufsize", "softspace",
                 # "closed" is a property, see below
                 "_sock", "_rbufsize", "_wbufsize", "_rbuf", "_wbuf", "_wbuf_len",
                 "_close"]

    def __init__(self, sock, mode='rb', bufsize=-1, close=False):
        self._sock = sock
        self.mode = mode # Not actually used in this version
        if bufsize < 0:
            bufsize = self.default_bufsize
        self.bufsize = bufsize
        self.softspace = False
        # _rbufsize is the suggested recv buffer size.  It is *strictly*
        # obeyed within readline() for recv calls.  If it is larger than
        # default_bufsize it will be used for recv calls within read().
        if bufsize == 0:
            self._rbufsize = 1
        elif bufsize == 1:
            self._rbufsize = self.default_bufsize
        else:
            self._rbufsize = bufsize
        self._wbufsize = bufsize
        # We use StringIO for the read buffer to avoid holding a list
        # of variously sized string objects which have been known to
        # fragment the heap due to how they are malloc()ed and often
        # realloc()ed down much smaller than their original allocation.
        self._rbuf = StringIO()
        self._wbuf = [] # A list of strings
        self._wbuf_len = 0
        self._close = close

    def _getclosed(self):
        return self._sock is None
    closed = property(_getclosed, doc="True if the file is closed")

    def close(self):
        try:
            if self._sock:
                self.flush()
        finally:
            if self._close:
                self._sock.close()
            self._sock = None

    def __del__(self):
        try:
            self.close()
        except:
            # close() may fail if __init__ didn't complete
            pass

    def flush(self):
        if self._wbuf:
            data = "".join(self._wbuf)
            self._wbuf = []
            self._wbuf_len = 0
            buffer_size = max(self._rbufsize, self.default_bufsize)
            data_size = len(data)
            write_offset = 0
            view = memoryview(data)
            try:
                while write_offset < data_size:
                    self._sock.sendall(view[write_offset:write_offset+buffer_size])
                    write_offset += buffer_size
            finally:
                if write_offset < data_size:
                    remainder = data[write_offset:]
                    del view, data  # explicit free
                    self._wbuf.append(remainder)
                    self._wbuf_len = len(remainder)

    def fileno(self):
        return self._sock.fileno()

    def write(self, data):
        data = str(data) # XXX Should really reject non-string non-buffers
        if not data:
            return
        self._wbuf.append(data)
        self._wbuf_len += len(data)
        if (self._wbufsize == 0 or
            self._wbufsize == 1 and '\n' in data or
            self._wbuf_len >= self._wbufsize):
            self.flush()

    def writelines(self, list):
        # XXX We could do better here for very long lists
        # XXX Should really reject non-string non-buffers
        lines = filter(None, map(str, list))
        self._wbuf_len += sum(map(len, lines))
        self._wbuf.extend(lines)
        if (self._wbufsize <= 1 or
            self._wbuf_len >= self._wbufsize):
            self.flush()

    def read(self, size=-1):
        # Use max, disallow tiny reads in a loop as they are very inefficient.
        # We never leave read() with any leftover data from a new recv() call
        # in our internal buffer.
        rbufsize = max(self._rbufsize, self.default_bufsize)
        # Our use of StringIO rather than lists of string objects returned by
        # recv() minimizes memory usage and fragmentation that occurs when
        # rbufsize is large compared to the typical return value of recv().
        buf = self._rbuf
        buf.seek(0, 2)  # seek end
        if size < 0:
            # Read until EOF
            self._rbuf = StringIO()  # reset _rbuf.  we consume it via buf.
            while True:
                try:
                    data = self._sock.recv(rbufsize)
                except error, e:
                    if e.args[0] == EINTR:
                        continue
                    raise
                if not data:
                    break
                buf.write(data)
            return buf.getvalue()
        else:
            # Read until size bytes or EOF seen, whichever comes first
            buf_len = buf.tell()
            if buf_len >= size:
                # Already have size bytes in our buffer?  Extract and return.
                buf.seek(0)
                rv = buf.read(size)
                self._rbuf = StringIO()
                self._rbuf.write(buf.read())
                return rv

            self._rbuf = StringIO()  # reset _rbuf.  we consume it via buf.
            while True:
                left = size - buf_len
                # recv() will malloc the amount of memory given as its
                # parameter even though it often returns much less data
                # than that.  The returned data string is short lived
                # as we copy it into a StringIO and free it.  This avoids
                # fragmentation issues on many platforms.
                try:
                    data = self._sock.recv(left)
                except error, e:
                    if e.args[0] == EINTR:
                        continue
                    raise
                if not data:
                    break
                n = len(data)
                if n == size and not buf_len:
                    # Shortcut.  Avoid buffer data copies when:
                    # - We have no data in our buffer.
                    # AND
                    # - Our call to recv returned exactly the
                    #   number of bytes we were asked to read.
                    return data
                if n == left:
                    buf.write(data)
                    del data  # explicit free
                    break
                assert n <= left, "recv(%d) returned %d bytes" % (left, n)
                buf.write(data)
                buf_len += n
                del data  # explicit free
                #assert buf_len == buf.tell()
            return buf.getvalue()

    def readline(self, size=-1):
        buf = self._rbuf
        buf.seek(0, 2)  # seek end
        if buf.tell() > 0:
            # check if we already have it in our buffer
            buf.seek(0)
            bline = buf.readline(size)
            if bline.endswith('\n') or len(bline) == size:
                self._rbuf = StringIO()
                self._rbuf.write(buf.read())
                return bline
            del bline
        if size < 0:
            # Read until \n or EOF, whichever comes first
            if self._rbufsize <= 1:
                # Speed up unbuffered case
                buf.seek(0)
                buffers = [buf.read()]
                self._rbuf = StringIO()  # reset _rbuf.  we consume it via buf.
                data = None
                recv = self._sock.recv
                while True:
                    try:
                        while data != "\n":
                            data = recv(1)
                            if not data:
                                break
                            buffers.append(data)
                    except error, e:
                        # The try..except to catch EINTR was moved outside the
                        # recv loop to avoid the per byte overhead.
                        if e.args[0] == EINTR:
                            continue
                        raise
                    break
                return "".join(buffers)

            buf.seek(0, 2)  # seek end
            self._rbuf = StringIO()  # reset _rbuf.  we consume it via buf.
            while True:
                try:
                    data = self._sock.recv(self._rbufsize)
                except error, e:
                    if e.args[0] == EINTR:
                        continue
                    raise
                if not data:
                    break
                nl = data.find('\n')
                if nl >= 0:
                    nl += 1
                    buf.write(data[:nl])
                    self._rbuf.write(data[nl:])
                    del data
                    break
                buf.write(data)
            return buf.getvalue()
        else:
            # Read until size bytes or \n or EOF seen, whichever comes first
            buf.seek(0, 2)  # seek end
            buf_len = buf.tell()
            if buf_len >= size:
                buf.seek(0)
                rv = buf.read(size)
                self._rbuf = StringIO()
                self._rbuf.write(buf.read())
                return rv
            self._rbuf = StringIO()  # reset _rbuf.  we consume it via buf.
            while True:
                try:
                    data = self._sock.recv(self._rbufsize)
                except error, e:
                    if e.args[0] == EINTR:
                        continue
                    raise
                if not data:
                    break
                left = size - buf_len
                # did we just receive a newline?
                nl = data.find('\n', 0, left)
                if nl >= 0:
                    nl += 1
                    # save the excess data to _rbuf
                    self._rbuf.write(data[nl:])
                    if buf_len:
                        buf.write(data[:nl])
                        break
                    else:
                        # Shortcut.  Avoid data copy through buf when returning
                        # a substring of our first recv().
                        return data[:nl]
                n = len(data)
                if n == size and not buf_len:
                    # Shortcut.  Avoid data copy through buf when
                    # returning exactly all of our first recv().
                    return data
                if n >= left:
                    buf.write(data[:left])
                    self._rbuf.write(data[left:])
                    break
                buf.write(data)
                buf_len += n
                #assert buf_len == buf.tell()
            return buf.getvalue()

    def readlines(self, sizehint=0):
        total = 0
        list = []
        while True:
            line = self.readline()
            if not line:
                break
            list.append(line)
            total += len(line)
            if sizehint and total >= sizehint:
                break
        return list

    # Iterator protocols

    def __iter__(self):
        return self

    def next(self):
        line = self.readline()
        if not line:
            raise StopIteration
        return line


socket = Socket
print "Loaded PycoSocket"

