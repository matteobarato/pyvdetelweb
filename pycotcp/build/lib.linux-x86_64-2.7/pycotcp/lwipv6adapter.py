#!/usr/bin/env python

#import lwipv6 #TODO doesn't even exist just yet
#from ctypes import * #TODO only import stuff used

from _pycolwipv6 import ffi
from _pycolwipv6 import lib as lwipv6
from adapter import Adapter
import os

#class IP_ADDR(Structure):
#    _fields = [("addr", c_uint32 * 4)]

#class lwip_stack(Structure):
#    pass

open_sockets = {}

class Lwipv6Adapter(Adapter):

    @ffi.def_extern()
    def event_callback(event):
        #This callback will receive the name of the socket that made this event happen, and whether it is a POLLIN or POLLOUT event
        eventstring = str(ffi.string(ffi.cast('char*', event)))
        print "Event callback %s" % eventstring
        split = eventstring.split(':')
        # split is [sock_name, event] 'read' or 'write'
        sock_name = split[0]
        ev = split[1]
        if ev == 'read':
            os.write(open_sockets[sock_name]['read_write_fakefd'], 'a')
            open_sockets[sock_name]['read_written_characters'] += 1
        elif ev == 'write':
            os.write(open_sockets[sock_name]['write_write_fakefd'], 'a')
            open_sockets[sock_name]['write_written_characters'] += 1

    def wait_for_read(self, sock_name):
        print "Wait for read on %s" % sock_name
        result = lwipv6.lwip_event_subscribe_read(lwipv6.event_callback, open_sockets[sock_name]['event_read'], open_sockets[sock_name]['fd'])
        if result != 0:
            #TODO no need to wait
            print "No need to wait on %s" % sock_name
            return False
        #TODO wait on the pipe for the read events
        print "Removing from %s's pipe" % sock_name
        os.read(open_sockets[sock_name]['read_read_fakefd'], open_sockets[sock_name]['read_written_characters'])
        open_sockets[sock_name]['read_written_characters'] = 0
        return True

    def wait_for_write(self, sock_name):
        print "Wait for write on %s" % sock_name
        result = lwipv6.lwip_event_subscribe_write(lwipv6.event_callback, open_sockets[sock_name]['event_write'], open_sockets[sock_name]['fd'])
        if result != 0:
            #TODO no need to wait
            print "No need to wait on %s" % sock_name
            return False
        #TODO wait on the pipe for the write events
        print "Removing from %s's pipe" % sock_name
        os.read(open_sockets[sock_name]['write_read_fakefd'], open_sockets[sock_name]['write_written_characters'])
        open_sockets[sock_name]['write_written_characters'] = 0
        return True

    LIBRARY_NAME = "lwipv6"

    current_stack = 0
    stack_pointer_list = []
    interfaces = {}
    links = {}

    ############################
    # LWIPV6 MACRO REDEFINITIONS
    ############################

    #def IP4_ADDRX(self, a, b, c, d):
    #    return lwipv6.htonl(((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) << 8) | (d & 0xff))

    #def IP64_PREFIX(self):
    #    return lwipv6.htonl(0xffff)

    #def IP6_ADDR(self, ipaddr, a, b, c, d, e, f, g, h):
    #    ipaddr.addr[0] = htonl(((a & 0xffff) << 16) | (b & 0xffff))
    #    ipaddr.addr[1] = htonl(((c & 0xffff) << 16) | (d & 0xffff))
    #    ipaddr.addr[2] = htonl(((e & 0xffff) << 16) | (f & 0xffff))
    #    ipaddr.addr[3] = htonl(((g & 0xffff) << 16) | (h & 0xffff))
    #    return ipaddr

    #def IP64_ADDR(self, ipaddr, a, b, c, d):
    #    ipaddr.addr[0] = 0
    #    ipaddr.addr[1] = 0
    #    ipaddr.addr[2] = self.IP64_PREFIX()
    #    ipaddr.addr[3] = self.IP4_ADDRX(a, b, c, d)
    #    return ipaddr

    #def IP64_MASKADDR(self, ipaddr, a, b, c, d):
    #    ipaddr.addr[0] = 0xffffffffff
    #    ipaddr.addr[1] = 0xffffffffff
    #    ipaddr.addr[2] = 0xffffffffff
    #    ipaddr.addr[3] = self.IP4_ADDRX(a, b, c, d)

    #def IP_ADDR_IS_V4(self, ipaddr):
    #    return ipaddr.addr[0] == 0 and ipaddr.addr[1] == 0 and ipaddr.addr[2] == self.IP64_PREFIX()

    ############################

    ############################
    # Parameter adaptations
    ############################

    def ipv6_address_str_to_array(self, original_address):
        split = original_address.split(':')
        address = []
        index = 0
        for integer in split:
            index = index + 1 #Doing so, index points to the current address element we stopped on when breaking.
            if integer == '':
                break
            address.append(int(integer, 16))

        while len(address) < 8 - len(split[index:]):
            address.append(0)

        for integer in split[index:]:
            if integer == '':
                address.append(0)
                continue
            address.append(int(integer, 16))

        print "Converted %s to %s" % (str(original_address), str(address))
        return address

    def ipv4_address_str_to_array(self, address):
        split = address.split('.')
        address = []
        for integer in split:
            address.append(int(integer))
        return address

    ############################

    def __init__(self):
        print "initing lwipv6adapter"
        #lwipv6 = CDLL("liblwipv6.so")

        #lwipv6.lwip_stack_new.restype = pointer(lwip_stack())
        #lwipv6.lwip_stack_new.restype = c_void_p
        #lwipv6.lwip_vdeif_add.argtypes = [pointer(lwip_stack()), c_void_p]

        stack_pointer = lwipv6.lwip_stack_new()
        self.stack_pointer_list.append(stack_pointer)
        print "pointer is %s" % stack_pointer
        self.current_stack = len(self.stack_pointer_list) - 1

    def stackTick(self):
        pass

    def idle(self):
        pass

    def get_current_stack_pointer(self):
        value = self.stack_pointer_list[self.current_stack]
        return value

    def check_error(self, result):
        #TODO error checking... And exception throwing...
        if result < 0:
            print "Error checking %s..." % str(result)

        return result

    def fileno(self, sock_name):
        return open_sockets[sock_name]['read_read_fakefd']

    def _deleteLink(self, netif_pointer, ip_addr_pointer, netmask_pointer):
        lwipv6.lwip_del_addr(netif_pointer, ip_addr_pointer, netmask_pointer)

    def deleteLink4(self, link_name):
        self._deleteLink(0, 0, 0) #TODO grab from dict

    def deleteLink6(self, link_name):
        self._deleteLink(0, 0, 0) #TODO grab from dict

    def deleteSocketBox(self, box_name):
        pass #NOOP only for picoTCP

    def createDevice(self, device_type, device_name = None, device_path = None):
        print "Creating LWIPv6 device %s %s %s" % (device_type, device_name, device_path)

        if device_name is None:
            #TODO generate a new name ?
            device_name = "vde"

        if device_path is None:
            device_path = "/tmp/" + device_name + ".ctl"

        interface = None
        if device_type == "vde":
            print "building vde interface"
            interface = lwipv6.lwip_vdeif_add(self.get_current_stack_pointer(), device_path)
        elif device_type == "tun":
            interface = lwipv6.lwip_tunif_add(self.get_current_stack_pointer(), device_path)
        elif device_type == "tap":
            interface = lwipv6.lwip_tapif_add(self.get_current_stack_pointer(), device_path)
        print "interface done building %s" % str(interface)

        lwipv6.lwip_ifup(interface)
        self.interfaces[device_name] = {
                    'pointer': interface
                }
        return interface

    def linkAddIp6(self, link_name, address, netmask):
        self.linkAddIp4(link_name, address, netmask)

    def linkAddIp4(self, link_name, address, netmask):
        print "Adding LWIPv6 address %s %s %s" % (link_name, address, netmask)

        if ":" in address or ":" in netmask:
            lwipv6.lwip_add_addr6(self.interfaces[link_name]['pointer'], self.ipv6_address_str_to_array(address), self.ipv6_address_str_to_array(netmask))
        else:
            lwipv6.lwip_add_addr4(self.interfaces[link_name]['pointer'], self.ipv4_address_str_to_array(address), self.ipv4_address_str_to_array(netmask))

        lwipv6.lwip_ifup(self.interfaces[link_name]['pointer']);

        #TODO maybe create helper C methods to get the struct ip_addr pointers?
        self.interfaces[link_name]['address'] = address
        self.interfaces[link_name]['netmask'] = netmask

        return self.check_error(0)

    def _close_pipes(self, sock_obj):
        os.close(sock_obj['write_read_fakefd'])
        os.close(sock_obj['write_write_fakefd'])
        os.close(sock_obj['read_read_fakefd'])
        os.close(sock_obj['read_write_fakefd'])

    def _add_open_socket(self, sock_name):
        open_sockets[sock_name] = {}
        open_sockets[sock_name]['event_read'] = "%s:%s" % (sock_name, "read")
        open_sockets[sock_name]['event_write'] = "%s:%s" % (sock_name, "write")
        open_sockets[sock_name]['read_read_fakefd'], open_sockets[sock_name]['read_write_fakefd'] = os.pipe()

        open_sockets[sock_name]['write_read_fakefd'], open_sockets[sock_name]['write_write_fakefd'] = os.pipe()
        open_sockets[sock_name]['read_written_characters'] = 0
        open_sockets[sock_name]['write_written_characters'] = 0

        os.write(open_sockets[sock_name]['read_write_fakefd'], 'a')
        open_sockets[sock_name]['read_written_characters'] += 1


    def socketOpen(self, sock_name, version, protocol, version_family):
        """ Opens a socket
        sock_name is the name of the socket
        version is "ipv4" or "ipv6"
        protocol is "tcp" or "udp"
        """

        family = version_family

        udptcp = 1 #SOCK_STREAM is 1 (tcp)
        if protocol == "udp":
            udptcp = 2 #SOCK_DGRAM is 2 (udp)

        #NOTE as name I could use the socket's file descriptor. would solve a lot of problems after 'accept' works...
        if sock_name in open_sockets:
            #This means the socket was already initialized eg. from accept, so I already have an fd to read/write on. Don't touch it.
            #TODO maybe fill up missing stuff?
            return 0

        result = lwipv6.lwip_msocket(self.get_current_stack_pointer(), family, udptcp, 0)

        self._add_open_socket(sock_name);
        open_sockets[sock_name]['fd'] = result
        open_sockets[sock_name]['version'] = version
        open_sockets[sock_name]['protocol'] = protocol
        open_sockets[sock_name]['family'] = family
        #TODO pipe per risolvere i problemi della select del python per fileno
        return self.check_error(result)

    def socketBind(self, sock_name, sock_addr, sock_port):
        """ Binds a socket
        sock_name is the name of the socket to bind
        sock_addr is the local_address of the socket
        sock_port is the port of the socket
        """

        family = open_sockets[sock_name]['family']

        #TODO alias for ANY
        sockaddr = lwipv6.create_server_sockaddr(family, int(sock_port))

        result = lwipv6.lwip_bind(open_sockets[sock_name]['fd'], ffi.cast("struct sockaddr*", sockaddr), lwipv6.sockaddr_size())
        open_sockets[sock_name]['sockaddr'] = sockaddr
        open_sockets[sock_name]['port'] = sock_port
        open_sockets[sock_name]['addr'] = sock_addr

        return self.check_error(result)

    def socketConnect(self, sock_name, sock_addr, sock_port):
        """ Connects to a socket
        sock_name is the name of the socket to connect to
        sock_addr is the local_address of the socket
        sock_port is the port of the socket
        """
        print "LWIPv6 doing socketConnect..."
        result = lwipv6.lwip_connectw(open_sockets[sock_name]['fd'], sock_addr, int(sock_port), open_sockets[sock_name]['family'])
        return self.check_error(result)

    def socketSend(self, sock_name, data, data_length):
        """ Sends data to a socket
        sock_name is the name of the socket to send data to
        data is the data buffer
        data_length is the length of the data buffer
        """
        self.wait_for_write(sock_name)
        result = lwipv6.lwip_send(open_sockets[sock_name]['fd'], data, data_length, 0)

        return self.check_error(result)

    def socketRecv(self, sock_name, data_length):
        """ Receives data from a socket
        sock_name is the name of the socket to receive from
        data_length is the length of the data buffer
        returns data received as string (char array)
        """
        print "LWIPv6 Reading on %s [%d]..." % (sock_name, open_sockets[sock_name]['fd'])
        #TODO how do I set flags?
        self.wait_for_read(sock_name)
        result = ffi.string(lwipv6.lwip_recvw(open_sockets[sock_name]['fd'], data_length, 0))

        return self.check_error(result)

    def socketRead(self, sock_name, data_length):
        """ Reads data from a socket
        sock_name is the name of the socket to read from
        data_length is the length of the data buffer
        returns data received as string (char array)
        """
        self.wait_for_read(sock_name)
        result = lwipv6.lwip_readw(open_sockets[sock_name]['fd'], data_length)

        return self.check_error(result)

    def socketWrite(self, sock_name, data, data_length):
        """ Writes data on a socket
        sock_name is the name of the socket to write on
        data is the data to write
        data_length is the length of the data buffer
        """
        self.wait_for_write(sock_name)
        result = lwipv6.lwip_write(open_sockets[sock_name]['fd'], data, data_length)

        return self.check_error(result)

    def socketSendTo(self, sock_name, data, data_length, sock_addr, sock_port, flags = 0):
        """ Sends data to a socket
        sock_name is the name of the socket to send from
        data is the data to send
        data_length is the length of the data buffer
        sock_addr is the address of the socket to which to send data
        sock_port is the port of the socket to which to send data
        """

        self.wait_for_write(sock_name)
        sockaddr = lwipv6.create_sockaddr(open_sockets[sock_name]['family'], int(sock_port), sock_addr)

        result = lwipv6.lwip_sendto(open_sockets[sock_name]['fd'], data, data_length, flags, sockaddr, lwipv6.sockaddr_size())

        return self.check_error(result)

    def socketRecvFrom(self, sock_name, data_length, sock_type, sock_addr, sock_port, flags = 0):
        """ Receives data from a socket
        sock_name is the name of the socket to receive from
        data_length is the length of the data buffer
        sock_type is the type of the socket ipv4 or ipv6 'ipv4' 'ipv6'
        """
        #NOTE family based on sock_type...? sock_type is ipv4 or ipv6... don't think I need it in lwipv6
        self.wait_for_read(sock_name)

        sockaddr = lwipv6.create_sockaddr(open_sockets[sock_name]['family'], int(sock_port), sock_addr)

        result = lwipv6.lwip_recvfromw(open_sockets[sock_name]['fd'], data_length, sockaddr)

        return self.check_error(result)

    def socketSendToExt(self, sock_name, data, data_length, sock_addr, sock_port, ttl, qos, flags = 0):
        """ Sends data to a socket
        sock_name is the name of the socket to send to
        data is the data to send
        data_length is the length of the data buffer
        sock_addr is the address of the socket
        sock_port is the port of the socket
        ttl is time to live
        qos is quality(?) of service
        """

        self.wait_for_write(sock_name)
        return open_socketsendTo(sock_name, data, data_length, sock_addr, sock_port, flags)

    def socketRecvFromExt(self, sock_name, data_length, sock_type, flags = 0):
        """ Receives data from a socket
        sock_name is the name of the socket to receive from
        data_length is the length of the data buffer
        sock_type is the type of the socket
        """
        return self.socketRecvFrom(sock_name, data, data_length, sock_addr, sock_port, flags)

    def socketClose(self, sock_name):
        """ Closes a socket
        sock_name is the name of the socket to close
        """
        result = lwipv6.lwip_close(open_sockets[sock_name]['fd'])
        self._close_pipes(open_sockets[sock_name])
        del open_sockets[sock_name]

        return result

    def socketShutdown(self, sock_name, sock_mode):
        """ Shutdown a socket
        sock_name is the name of the socket to shutdown
        sock_mode is one of "rw", "r", "w"
        """

        if sock_mode == "r":
            mode = 0
        elif sock_mode == "w":
            mode = 1
        elif sock_mode == "rw":
            mode = 2

        result = lwipv6.lwip_shutdownw(open_sockets[sock_name]['fd'], mode)

        return result

    def socketListen(self, sock_name, backlog):
        """ Listens to a socket
        sock_name is the name of the socket to listen to
        backlog is maximum number of connections
        """
        result = lwipv6.lwip_listen(open_sockets[sock_name]['fd'], backlog)

        return self.check_error(result)

    #TODO is accept supposed to be blocking?
    def socketAccept(self, sock_name):
        """ Accepts a socket
        sock_name is the name of the socket to accept
        returns the file descriptor of the incoming connection
                if negative an error occurred
        """
        print "LWIPv6 now accepting connections on %s..." % sock_name
        print "LWIPv6 accepting with parameters %s NULL NULL" % open_sockets[sock_name]['fd']
        #lwipv6.check_sockaddr(ffi.cast("struct sockaddr*", open_sockets[sock_name]['sockaddr']))
        size = ffi.new("unsigned int*")
        size[0] = lwipv6.sockaddr_size()

        result_sockaddr = lwipv6.create_empty_sockaddr();

        result = lwipv6.lwip_accept(open_sockets[sock_name]['fd'], ffi.cast("struct sockaddr*", result_sockaddr), size)
        print "LWIPv6 done accepting %s" % str(result)
        #lwipv6.check_sockaddr(ffi.cast("struct sockaddr*", result_sockaddr))
        self.check_error(result)

        my_sock = open_sockets[sock_name]

        new_sock_name = str(result)
        self._add_open_socket(new_sock_name);
        open_sockets[new_sock_name]['fd'] = result
        open_sockets[new_sock_name]['sockaddr'] = result_sockaddr
        open_sockets[new_sock_name]['family'] = my_sock['family']

        port = lwipv6.fetch_port(ffi.cast("struct sockaddr*", result_sockaddr), open_sockets[new_sock_name]['family'])

        #TODO second return value should be the address
        #TODO this should be a tuple
        return (result, (result, port)) #TODO this is a socket fd... need to create a name for it and return that? this will probably blow up the default skeleton as it expects a string that is the socket's name (NOTE Actually, this should work. In the caller's code in socket.c it creates a new socket with name equal to the fd. just create the same thing inside open_sockets and problem solved! (probably need to build an appropriate 'struct sockaddr')


