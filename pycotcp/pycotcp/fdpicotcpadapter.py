#!/usr/bin/env python

#from ctypes import * #TODO only import stuff used
from _fdpicotcp import ffi
from _fdpicotcp import lib as pycoclib
from pycoexceptions import LibraryException
from pycoexceptions import ConfigurationException
from pycoexceptions import ParseException
from adapter import Adapter

class FDPicoTCPAdapter(Adapter):

    initialized = False

    LIBRARY_NAME = "fdpicotcp"

    AF_PICO_INET = 1018
    AF_PICO_INET6 = 1019

    IS_OK = 0
    NOT_OK = -1
    NOT_SUPPORTED = -4
    ERROR = -3
    PARSE_FAIL = -2

    open_sockets = {}

    def __init__(self):
        print "initing fdpicotcpadapter"
        if not FDPicoTCPAdapter.initialized:
            #pycoclib.pico_stack_init()
            FDPicoTCPAdapter.initialized = True

    def fileno(self, sock_name):
        return self.open_sockets[sock_name]['fd']

    def stackTick(self):
        return pycoclib.pico_stack_tick()

    def idle(self):
        return pycoclib.idle()

    def getPicoError(self):
        return 0

    def check_error(self, result):
        if result == FDPicoTCPAdapter.PARSE_FAIL:
            raise ConfigurationException("PARSE_FAIL (%s)" % self.getPicoError(), FDPicoTCPAdapter.PARSE_FAIL, FDPicoTCPAdapter.LIBRARY_NAME)
        elif result == FDPicoTCPAdapter.NOT_OK:
            raise ConfigurationException("NOT_OK (%s)" % self.getPicoError(), FDPicoTCPAdapter.NOT_OK, FDPicoTCPAdapter.LIBRARY_NAME)
        elif result == FDPicoTCPAdapter.ERROR:
            raise LibraryException("ERROR (%s)" % self.getPicoError(), FDPicoTCPAdapter.ERROR, FDPicoTCPAdapter.LIBRARY_NAME)
        else:
            return result

    def deleteLink4(self, link_name):
        result = pycoclib.deletelink4(link_name)

        if result == FDPicoTCPAdapter.PARSE_FAIL:
            raise ParseException("PARSE_FAIL", self.PARSE_FAIL, library_name)
        else:
            return result

    def deleteLink6(self, link_name):
        result = pycoclib.deletelink6(link_name)

        if result == FDPicoTCPAdapter.PARSE_FAIL:
            raise ParseException("PARSE_FAIL", self.PARSE_FAIL, library_name)
        else:
            return result

    def deleteSocketBox(self, box_name):
        result = pycoclib.deleteSocketBox(box_name)

        if result == parse_fail:
            raise ParseException("PARSE_FAIL", self.PARSE_FAIL, library_name)
        else:
            return result

    def createDevice(self, device_type, device_name = None, device_path = None):
        result = None
        return result

    def createMreq(self, mreq_name, dest_addr, link_addr, stype, source_addr = None):
        result = None

        if source_addr is None:
            result = pycoclib.createMreq(mreq_name, dest_addr, link_addr, stype)
        else:
            result = pycoclib.createMreq(mreq_name, dest_addr, source_addr, link_addr, stype)

        return self.check_error(result)

    def createMreqSource(self, mreq_name, dest_addr, source_addr, link_addr, stype):
        return self.createMreq(mreq_name, dest_addr,link_addr, stype, source_addr)

    def deleteMreq(self, mreq_name):
        result = pycoclib.deleteMreq(mreq_name)

        if result == FDPicoTCPAdapter.PARSE_FAIL:
            raise ConfigurationException("PARSE_FAIL", FDPicoTCPAdapter.PARSE_FAIL, FDPicoTCPAdapter.LIBRARY_NAME)
        else:
            return result

    def deleteMreqSource(self, mreq_name):
        result = pycoclib.deleteMreqSource(mreq_name)

        if result == FDPicoTCPAdapter.PARSE_FAIL:
            raise ConfigurationException("PARSE_FAIL", FDPicoTCPAdapter.PARSE_FAIL, FDPicoTCPAdapter.LIBRARY_NAME)
        else:
            return result

    def linkAddIp4(self, link_name, address, netmask):
        return 0

    def linkAddIp6(self, link_name, address, netmask):
        return 0

    def linkDelIp4(self, link_name, address):
        return 0

    def linkDelIp6(self, link_name, address):
        return 0

    def pingStartIp4(self, address, packet_count, time_interval, timeout, packet_size, callback):
        result = pycoclib.pingStartIp4(address, packet_count, time_interval, timeout, packet_size, callback)

        if result == 0: # perhaps repair pycoclib.c: ping returns NULL in C
            raise ConfigurationException("PARSE_FAIL", FDPicoTCPAdapter.PARSE_FAIL, FDPicoTCPAdapter.LIBRARY_NAME)

        return self.check_error(result)

    def pingStartIp6(self, address, packet_count, time_interval, timeout, packet_size, callback):
        result = pycoclib.pingStartIp6(address, packet_count, time_interval, timeout, packet_size, callback)

        if result == 0: # perhaps repair pycoclib.c: ping returns NULL in C
            raise ConfigurationException("PARSE_FAIL", FDPicoTCPAdapter.PARSE_FAIL, FDPicoTCPAdapter.LIBRARY_NAME)

        return self.check_error(result)

    def pingAbortIp4(self, ping_id):
        result = pycoclib.pingAbortIp4(ping_id)

        return self.check_error(result)

    def socketOpen(self, sock_name, version, protocol, version_family):
        """ Opens a socket
        sock_name is the name of the socket
        version is "ipv4" or "ipv6"
        protocol is "tcp" or "udp"
        returns file descriptor of this socket
        """
        family = self.AF_PICO_INET
        if version == "ipv6":
            family = self.AF_PICO_INET6

        proto = 1 #SOCK_STREAM
        if protocol == "udp":
            proto = 2 #SOCK_DGRAM

        if sock_name in self.open_sockets:
            #This means the socket was already initialized eg. from accept, so I already have an fd to read/write on. Don't touch it.
            return

        print "Calling socket with %s %s %s" % (str(family), str(proto), "0")
        result = pycoclib.pico_socket(family, proto, 0)

        if result >= 0:
            print "Adding open socket %s" % sock_name
            self.open_sockets[sock_name] = {}
            self.open_sockets[sock_name]['fd'] = result
            self.open_sockets[sock_name]['family'] = family
            self.open_sockets[sock_name]['proto'] = proto

        return self.check_error(result)

    def socketBind(self, sock_name, sock_addr, sock_port):
        """ Binds a socket
        sock_name is the name of the socket to bind
        sock_addr is the local_address of the socket
        sock_port is the port of the socket
        """
        #TODO find out what to pass as the first argument
        current_socket = self.open_sockets[sock_name]
        #TODO AF_PICO_INET6 if ipv6
        #TODO ANY alias for 0.0.0.0 (and ::)
        #TODO address family
        #sockaddr = pycoclib.create_server_sockaddr(2, int(sock_port))
        #sockaddr = pycoclib.create_server_sockaddr(current_socket['family'], int(sock_port))
        sockaddr = pycoclib.create_sockaddr(current_socket['family'], int(sock_port), sock_addr)

        result = pycoclib.pico_bind(current_socket['fd'], sockaddr, pycoclib.sockaddr_size())

        return self.check_error(result)

    def socketConnect(self, sock_name, sock_addr, sock_port):
        """ Connects to a socket
        sock_name is the name of the socket to connect to
        sock_addr is the local_address of the socket
        sock_port is the port of the socket
        """
        current_socket = self.open_sockets[sock_name]
        print "Running connect with %s %s %s" % (str(current_socket['fd']), sock_addr, sock_port)
        result = pycoclib.pico_connectw(current_socket['fd'], sock_addr, int(sock_port))

        return self.check_error(result)

    def socketSend(self, sock_name, data, data_length):
        """ Sends data to a socket
        sock_name is the name of the socket to send data to
        data is the data buffer
        data_length is the length of the data buffer
        """
        return self.socketWrite(sock_name, data, data_length)

    def socketRecv(self, sock_name, data_length):
        """ Receives data from a socket
        sock_name is the name of the socket to receive from
        data_length is the length of the data buffer
        returns data received as string (char array)
        """
        current_socket = self.open_sockets[sock_name]
        result = ffi.string(pycoclib.pico_readw(current_socket['fd'], data_length))
        return self.check_error(result)

    def socketRead(self, sock_name, data_length):
        """ Reads data from a socket
        sock_name is the name of the socket to read from
        data_length is the length of the data buffer
        returns data received as string (char array)
        """
        return self.socketRecv(sock_name, data_length)

    def socketWrite(self, sock_name, data, data_length):
        """ Writes data on a socket
        sock_name is the name of the socket to write on
        data is the data to write
        data_length is the length of the data buffer
        """
        current_socket = self.open_sockets[sock_name]
        result = pycoclib.pico_write(current_socket['fd'], data, data_length)

        return self.check_error(result)

    def socketSendTo(self, sock_name, data, data_length, sock_addr, sock_port):
        """ Sends data to a socket
        sock_name is the name of the socket to send from
        data is the data to send
        data_length is the length of the data buffer
        sock_addr is the address of the socket to which to send data
        sock_port is the port of the socket to which to send data
        """
        #TODO fdpicotcp doesn't have "send to"
        result = pycoclib.socketSendTo(sock_name, data, data_length, sock_addr, sock_port)

        return self.check_error(result)

    def socketRecvFrom(self, sock_name, data_length, sock_type):
        """ Receives data from a socket
        sock_name is the name of the socket to receive from
        data_length is the length of the data buffer
        sock_type is the type of the socket
        """
        return self.socketRecv(sock_name, data_length)

    def socketSendToExt(self, sock_name, data, data_length, sock_addr, sock_port, ttl, qos):
        """ Sends data to a socket
        sock_name is the name of the socket to send to
        data is the data to send
        data_length is the length of the data buffer
        sock_addr is the address of the socket
        sock_port is the port of the socket
        ttl is time to live
        qos is quality(?) of service
        """
        result = pycoclib.socketSendToExt(sock_name, data, data_length, sock_addr, sock_port, ttl, qos)

        return self.check_error(result)

    def socketRecvFromExt(self, sock_name, data_length, sock_type):
        """ Receives data from a socket
        sock_name is the name of the socket to receive from
        data_length is the length of the data buffer
        sock_type is the type of the socket
        """
        return self.socketRecv(sock_name, data_length)

    def socketClose(self, sock_name):
        """ Closes a socket
        sock_name is the name of the socket to close
        """
        current_socket = self.open_sockets[sock_name]
        result = pycoclib.pico_close(current_socket['fd'])
        del self.open_sockets[sock_name]

        return self.check_error(result)

    def socketShutdown(self, sock_name, sock_mode):
        """ Shutdown a socket
        sock_name is the name of the socket to shutdown
        sock_mode is one of "rw", "r", "w"
        """
        #TODO fdpicotcp doesn't have shutdown
        #result = pycoclib.socketShutdown(sock_name, sock_mode)
        result = 0

        return self.check_error(result)

    def socketListen(self, sock_name, backlog):
        """ Listens to a socket
        sock_name is the name of the socket to listen to
        backlog is maximum number of connections
        """
        current_socket = self.open_sockets[sock_name]
        result = pycoclib.pico_listen(current_socket['fd'], backlog)

        return self.check_error(result)

    def socketAccept(self, sock_name):
        """ Accepts a socket
        sock_name is the name of the socket to accept
        returns None if no one is trying to connect
        """
        current_socket = self.open_sockets[sock_name]

        size = ffi.new("unsigned int*")
        size[0] = pycoclib.sockaddr_size()

        result_sockaddr = pycoclib.create_empty_sockaddr();
        result = pycoclib.pico_accept(current_socket['fd'], result_sockaddr, pycoclib.sockaddr_size())

        self.check_error(result)

        new_sock_name = str(result) #Generate a more unique name
        self.open_sockets[new_sock_name] = {}
        self.open_sockets[new_sock_name]['fd'] = result
        self.open_sockets[new_sock_name]['sockaddr'] = result_sockaddr

        return (result, result)

