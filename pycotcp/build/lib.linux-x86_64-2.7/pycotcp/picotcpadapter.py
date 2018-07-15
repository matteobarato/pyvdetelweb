#!/usr/bin/env python

import pycoclib
import os
import time
from threading import Semaphore

from pycoexceptions import *
from adapter import Adapter

open_sockets = {}

PICO_SOCK_EV_RD = 1
PICO_SOCK_EV_WR = 2
PICO_SOCK_EV_CONN = 4
PICO_SOCK_EV_CLOSE = 8
PICO_SOCK_EV_FIN = 16
PICO_SOCK_EV_ERR = 128

def unlock_sem(semaphore, count):
    if count > 0:
        return False
    else:
        semaphore.release()
        return True

def waiton_sem(semaphore):
    semaphore.acquire()

def V_accept(sock_obj):
    #if unlock_sem(sock_obj['accept_semaphore'], sock_obj['accept_count']):
    #    sock_obj['accept_count'] += 1
    sock_obj['accept_semaphore'].release()
    sock_obj['accept_count'] += 1
    os.write(sock_obj['accept_write_fakefd'], 'a')
    sock_obj['accept_written_characters'] = sock_obj['accept_written_characters'] + 1

def P_accept(sock_obj):
    waiton_sem(sock_obj['accept_semaphore'])
    sock_obj['accept_count'] -= 1

def V_read(sock_obj):
    if unlock_sem(sock_obj['read_semaphore'], sock_obj['read_count']):
        sock_obj['read_count'] += 1
    os.write(sock_obj['read_write_fakefd'], 'a')
    sock_obj['read_written_characters'] = sock_obj['read_written_characters'] + 1

def P_read(sock_obj):
    waiton_sem(sock_obj['read_semaphore'])
    sock_obj['read_count'] -= 1

def V_write(sock_obj):
    if unlock_sem(sock_obj['write_semaphore'], sock_obj['write_count']):
        sock_obj['write_count'] += 1

def P_write(sock_obj):
    waiton_sem(sock_obj['write_semaphore'])
    sock_obj['write_count'] -= 1


def _socket_callback(param, socket):
    """ Handles pico_socket callbacks
    CONN --> Socket connected
    RD --> Ready to read from socket
    WR --> Ready to write to socket
    CLOSE --> Socket closed, no further communications are possible
    FIN --> FIN segment received (TCP Only)
    ERR --> Error on socket
    """

    #print "socket callback %s: %d" % (socket, param)

    if param & PICO_SOCK_EV_CONN:
        V_accept(open_sockets[socket])

    if param & PICO_SOCK_EV_RD:
        try:
            print "V read on %s" % socket
            V_read(open_sockets[socket])
        except:
            pass

    if param & PICO_SOCK_EV_WR:
        try:
            V_write(open_sockets[socket])
        except:
            pass

    if param & PICO_SOCK_EV_CLOSE:
        pass

    if param & PICO_SOCK_EV_FIN:
        pass

    if param & PICO_SOCK_EV_ERR:
        pass

class PicoTCPAdapter(Adapter):

    initialized = False

    LIBRARY_NAME = "picotcp"
    IS_OK = 0
    NOT_OK = -1
    NOT_SUPPORTED = -4
    ERROR = -3
    PARSE_FAIL = -2

    def __init__(self):
        if not PicoTCPAdapter.initialized:
            pycoclib.init()
            PicoTCPAdapter.initialized = True

    def fileno(self, sock_name):
        return open_sockets[sock_name]['accept_read_fakefd']

    def stackTick(self):
        return pycoclib.stackTick()

    def idle(self):
        return pycoclib.idle()

    def check_error(self, result):
        import inspect
        caller = inspect.currentframe().f_back.f_code.co_name

        if result == PicoTCPAdapter.PARSE_FAIL:
            raise ConfigurationException("PARSE_FAIL (%s)" % pycoclib.getPicoError(), PicoTCPAdapter.PARSE_FAIL, PicoTCPAdapter.LIBRARY_NAME)
        elif result == PicoTCPAdapter.NOT_OK:
            raise ConfigurationException("NOT_OK (%s)" % pycoclib.getPicoError(), PicoTCPAdapter.NOT_OK, PicoTCPAdapter.LIBRARY_NAME)
        elif result == PicoTCPAdapter.ERROR:
            raise LibraryException("ERROR (%s)" % pycoclib.getPicoError(), PicoTCPAdapter.ERROR, PicoTCPAdapter.LIBRARY_NAME)
        else:
            return result

    def deleteLink4(self, link_name):
        result = pycoclib.deletelink4(link_name)

        if result == PicoTCPAdapter.PARSE_FAIL:
            raise ParseException("PARSE_FAIL", self.PARSE_FAIL, library_name)
        else:
            return result

    def deleteLink6(self, link_name):
        result = pycoclib.deletelink6(link_name)

        if result == PicoTCPAdapter.PARSE_FAIL:
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
        print "Creating device with type %s, name %s, path %s" % (device_type, device_name, device_path)
        result = None
        if device_name is not None:
            if device_path is not None:
                result = pycoclib.createDevice(device_type, device_name, device_path)
            else:
                result = pycoclib.createDevice(device_type, device_name)
        else:
            result = pycoclib.createDevice(device_type)

        return self.check_error(result)

    def deleteDevice(self, device_name):
        result = pycoclib.deleteDevice(device_name)

        if result == PicoTCPAdapter.PARSE_FAIL:
            raise ParseException("PARSE_FAIL", PicoTCPAdapter.PARSE_FAIL, PicoTCPAdapter.LIBRARY_NAME)
        else:
            return result   #TODO result is from picotcp's C code, which means it is an integer and nothing else. maybe check it from python itself? (to raise exceptions such as DeviceNotFoundException

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

        if result == PicoTCPAdapter.PARSE_FAIL:
            raise ConfigurationException("PARSE_FAIL", PicoTCPAdapter.PARSE_FAIL, PicoTCPAdapter.LIBRARY_NAME)
        else:
            return result

    def deleteMreqSource(self, mreq_name):
        result = pycoclib.deleteMreqSource(mreq_name)

        if result == PicoTCPAdapter.PARSE_FAIL:
            raise ConfigurationException("PARSE_FAIL", PicoTCPAdapter.PARSE_FAIL, PicoTCPAdapter.LIBRARY_NAME)
        else:
            return result

    def linkAddIp4(self, link_name, address, netmask):
        result = pycoclib.linkAddIp4(link_name, address, netmask)

        return self.check_error(result)

    def linkAddIp6(self, link_name, address, netmask):
        result = pycoclib.linkAddIp6(link_name, address, netmask)

        return self.check_error(result)

    def linkDelIp4(self, link_name, address):
        result = pycoclib.linkDelIp4(link_name, address)

        return self.check_error(result)

    def linkDelIp6(self, link_name, address):
        result = pycoclib.linkDelIp6(link_name, address)

        return self.check_error(result)

    def pingStartIp4(self, address, packet_count, time_interval, timeout, packet_size):
        result = pycoclib.pingStartIp4(address, packet_count, time_interval, timeout, packet_size, _socket_callback)

        if result == 0: # perhaps repair pycoclib.c: ping returns NULL in C
            raise ConfigurationException("PARSE_FAIL", PicoTCPAdapter.PARSE_FAIL, PicoTCPAdapter.LIBRARY_NAME)

        return self.check_error(result)

    def pingStartIp6(self, address, packet_count, time_interval, timeout, packet_size):
        result = pycoclib.pingStartIp6(address, packet_count, time_interval, timeout, packet_size, _socket_callback)

        if result == 0: # perhaps repair pycoclib.c: ping returns NULL in C
            raise ConfigurationException("PARSE_FAIL", PicoTCPAdapter.PARSE_FAIL, PicoTCPAdapter.LIBRARY_NAME)

        return self.check_error(result)

    def pingAbortIp4(self, ping_id):
        result = pycoclib.pingAbortIp4(ping_id)

        return self.check_error(result)

    def _close_pipes(self, sock_obj):
        os.close(sock_obj['accept_read_fakefd'])
        os.close(sock_obj['accept_write_fakefd'])
        os.close(sock_obj['read_read_fakefd'])
        os.close(sock_obj['read_write_fakefd'])

    def _add_open_socket(self, sock_name):
        open_sockets[sock_name] = {}
        open_sockets[sock_name]['context'] = self
        open_sockets[sock_name]['accept_semaphore'] = Semaphore(0)
        open_sockets[sock_name]['accept_count'] = 0
        open_sockets[sock_name]['accept_read_fakefd'], open_sockets[sock_name]['accept_write_fakefd'] = os.pipe()
        open_sockets[sock_name]['accept_written_characters'] = 0
        open_sockets[sock_name]['read_semaphore'] = Semaphore(0)
        open_sockets[sock_name]['read_count'] = 0
        open_sockets[sock_name]['read_read_fakefd'], open_sockets[sock_name]['read_write_fakefd'] = os.pipe()
        open_sockets[sock_name]['read_written_characters'] = 0
        open_sockets[sock_name]['write_semaphore'] = Semaphore(0)
        open_sockets[sock_name]['write_count'] = 0
        return open_sockets[sock_name]

    def socketOpen(self, sock_name, version, protocol, version_family):
        """ Opens a socket
        sock_name is the name of the socket
        version is "ipv4" or "ipv6"
        protocol is "tcp" or "udp"
        """
        if sock_name in open_sockets:
            return

        result = pycoclib.socketOpen(sock_name, version, protocol, _socket_callback)

        self._add_open_socket(sock_name)
        open_sockets[sock_name]['protocol'] = protocol

        return self.check_error(result)

    def socketBind(self, sock_name, sock_addr, sock_port):
        """ Binds a socket
        sock_name is the name of the socket to bind
        sock_addr is the local_address of the socket
        sock_port is the port of the socket
        """
        result = pycoclib.socketBind(sock_name, sock_addr, sock_port)

        return self.check_error(result)

    def socketConnect(self, sock_name, sock_addr, sock_port):
        """ Connects to a socket
        sock_name is the name of the socket to connect to
        sock_addr is the local_address of the socket
        sock_port is the port of the socket
        """
        result = pycoclib.socketConnect(sock_name, sock_addr, sock_port)
        try:
            self.check_error(result)
        except:
            #self.socketClose(sock_name)
            #TODO find a way to raise the error or import oldsocket
            #raise oldsocket.error()
            return None

        if open_sockets[sock_name]['protocol'] == 'tcp':
            P_accept(open_sockets[sock_name])

        return result

    def socketSend(self, sock_name, data, data_length):
        """ Sends data through a socket
        sock_name is the name of the socket to send data to
        data is the data buffer
        data_length is the length of the data buffer
        """
        self.write_check(sock_name);
        result = pycoclib.socketSend(sock_name, data, data_length)

        return self.check_error(result)

    def write_check(self, sock_name):
        print "P_write %s" % sock_name
        if sock_name not in open_sockets:
            return

        P_write(open_sockets[sock_name])

    def read_check(self, sock_name):
        print "P_read %s" % sock_name
        P_read(open_sockets[sock_name])
        os.read(open_sockets[sock_name]['read_read_fakefd'], open_sockets[sock_name]['read_written_characters'])
        open_sockets[sock_name]['read_written_characters'] = 0

    def socketRecv(self, sock_name, data_length):
        """ Receives data from a socket
        sock_name is the name of the socket to receive from
        data_length is the length of the data buffer
        returns data received as string (char array)
        """
        print "Reading from %s" % sock_name
        self.read_check(sock_name)
        result = pycoclib.socketRecv(sock_name, data_length)
        return self.check_error(result)

    def socketRead(self, sock_name, data_length):
        """ Reads data from a socket
        sock_name is the name of the socket to read from
        data_length is the length of the data buffer
        returns data received as string (char array)
        """
        self.read_check(sock_name)
        result = pycoclib.socketRead(sock_name, data_length)
        return self.check_error(result)

    def socketWrite(self, sock_name, data, data_length):
        """ Writes data on a socket
        sock_name is the name of the socket to write on
        data is the data to write
        data_length is the length of the data buffer
        """
        self.write_check(sock_name);
        result = pycoclib.socketWrite(sock_name, data, data_length)

        return self.check_error(result)

    def socketSendTo(self, sock_name, data, data_length, sock_addr, sock_port):
        """ Sends data to a socket
        sock_name is the name of the socket to send from
        data is the data to send
        data_length is the length of the data buffer
        sock_addr is the address of the socket to which to send data
        sock_port is the port of the socket to which to send data
        """
        self.write_check(sock_name);
        result = pycoclib.socketSendTo(sock_name, data, data_length, sock_addr, sock_port)

        return self.check_error(result)

    def socketRecvFrom(self, sock_name, data_length, sock_type):
        """ Receives data from a socket
        sock_name is the name of the socket to receive from
        data_length is the length of the data buffer
        sock_type is the type of the socket
        """
        print "Reading from %s" % sock_name
        self.read_check(sock_name)
        result = pycoclib.socketRecvFrom(sock_name, data_length, sock_type)
        return self.check_error(result)

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
        self.write_check(sock_name);
        result = pycoclib.socketSendToExt(sock_name, data, data_length, sock_addr, sock_port, ttl, qos)

        return self.check_error(result)

    def socketRecvFromExt(self, sock_name, data_length, sock_type):
        """ Receives data from a socket
        sock_name is the name of the socket to receive from
        data_length is the length of the data buffer
        sock_type is the type of the socket
        """
        print "Reading from %s" % sock_name
        self.read_check(sock_name)
        result = pycoclib.socketRecvFromExt(sock_name, data_length, sock_type)
        return self.check_error(result)

    def socketClose(self, sock_name):
        """ Closes a socket
        sock_name is the name of the socket to close
        """
        result = pycoclib.socketClose(sock_name)
        open_sockets[sock_name]
        self._close_pipes(open_sockets[sock_name])
        del open_sockets[sock_name]

        if result == PicoTCPAdapter.PARSE_FAIL:
            raise ParseException("PARSE_FAIL", PicoTCPAdapter.PARSE_FAIL, PicoTCPAdapter.LIBRARY_NAME)
        elif result == PicoTCPAdapter.NOT_OK:
            raise ConfigurationException("NOT_OK", PicoTCPAdapter.NOT_OK, PicoTCPAdapter.LIBRARY_NAME)
        elif result == PicoTCPAdapter.ERROR:
            #raise LibraryException("ERROR", PicoTCPAdapter.ERROR, PicoTCPAdapter.LIBRARY_NAME)
            pass #Same as shutdown
        else:
            return result

    def socketShutdown(self, sock_name, sock_mode):
        """ Shutdown a socket
        sock_name is the name of the socket to shutdown
        sock_mode is one of "rw", "r", "w"
        """
        result = pycoclib.socketShutdown(sock_name, sock_mode)

        if result == PicoTCPAdapter.PARSE_FAIL:
            raise ParseException("PARSE_FAIL", PicoTCPAdapter.PARSE_FAIL, PicoTCPAdapter.LIBRARY_NAME)
        elif result == PicoTCPAdapter.NOT_OK:
            raise ConfigurationException("NOT_OK", PicoTCPAdapter.NOT_OK, PicoTCPAdapter.LIBRARY_NAME)
        elif result == PicoTCPAdapter.ERROR:
            #raise LibraryException("ERROR", PicoTCPAdapter.ERROR, PicoTCPAdapter.LIBRARY_NAME)
            pass    #Should not need an exception, socket is already closed
        else:
            return result

    def socketListen(self, sock_name, backlog):
        """ Listens to a socket
        sock_name is the name of the socket to listen to
        backlog is maximum number of connections
        """
        result = pycoclib.socketListen(sock_name, backlog)

        return self.check_error(result)

    #TODO is accept supposed to be blocking?
    def socketAccept(self, sock_name):
        """ Accepts a socket
        sock_name is the name of the socket to accept
        returns None if no one is trying to connect
            else returns a tuple (ssi) (name, address, port)
        """
        print "P_accept %s" % sock_name
        P_accept(open_sockets[sock_name])
        result = pycoclib.socketAccept(sock_name, _socket_callback)
        print "Result is %s" % str(result)

        new_sock_name = result[0]
        self._add_open_socket(new_sock_name)

        if result == PicoTCPAdapter.PARSE_FAIL:
            raise ParseException("PARSE_FAIL", PicoTCPAdapter.PARSE_FAIL, PicoTCPAdapter.LIBRARY_NAME)
        elif result == PicoTCPAdapter.NOT_OK:
        #    raise ConfigurationException("NOT_OK", PicoTCPAdapter.NOT_OK, PicoTCPAdapter.LIBRARY_NAME)
            return None
        elif result == PicoTCPAdapter.ERROR:
            raise LibraryException("ERROR", PicoTCPAdapter.ERROR, PicoTCPAdapter.LIBRARY_NAME)
        else:
            return result

    def socketGetName(self, sock_name, version):
        """ Gets the specified socket's name
        sock_name is the name of the socket to accept
        version is "ipv4" or "ipv6"
        """
        result = pycoclib.socketGetName(sock_name, version)

        return self.check_error(result)

    def socketGetPeerName(self, sock_name, version):
        """ Gets the specified socket's peer name
        sock_name is the name of the socket to accept
        version is "ipv4" or "ipv6"
        """
        result = pycoclib.socketGetPeerName(sock_name, version)

        return self.check_error(result)

    def socketSetOption(self, sock_name, option, value):
        """ Sets an option on the specified socket """
        result = pycoclib.socketSetOption(sock_name, option, value)

        return self.check_error(result)

    def socketSetOptionMreq(self, sock_name, option, value):
        """ Sets an option on the specified socket """
        result = pycoclib.socketSetOptionMreq(sock_name, option, value)

        return self.check_error(result)

    def socketSetOptionMreqSource(self, sock_name, option, value):
        """ Sets an option on the specified socket """
        result = pycoclib.socketSetOptionMreqSource(sock_name, option, value)

        return self.check_error(result)

    def socketGetOption(self, sock_name, option):
        """ Gets an option on the specified socket """
        result = pycoclib.socketGetOption(sock_name, option)

        return self.check_error(result)

    def dhcpClientInitiate(self, device_name, callback):
        """ Starts DHCP client on given device
        returns DHCP negotiation ID
        """
        #TODO FIXME FIX BUG IN pycoclib.C: ID IS NOT RETURNED!
        result = pycoclib.dhcpClientInitiate(device_name, callback)

        return self.check_error(result)

    def dhcpClientAbort(self, dhcp_neg_id):
        """ Aborts DHCP client """
        result = pycoclib.dhcpClientAbort(dhcp_neg_id)

        return self.check_error(result)

    def dhcpServerInitiate(self, device_name, address, netmask, startpool, endpool, leasetime):
        """ Initiates a DHCP server on the given local device and address.
        startpool, endpool and leasetime
        """
        result = pycoclib.dhcpServerInitiate(device_name, device_name, address, netmask, startpool, endpool, leasetime)

        return self.check_error(result)

    def dhcpServerDestroy(self, device_name):
        """ Stops DHCP server """
        result = pycoclib.dhcpServerDestroy(device_name)

        return self.check_error(result)

    def sntpSync(self, sntp_server, callback):
        """ Syncs with given SNTP server """
        result = pycoclib.sntpSync(sntp_server, callback)

        return self.check_error(result)

    def sntpGetTimeOfTheDay(self, sntp_server, callback):
        """ Get time of day from SNTP server """
        result = pycoclib.sntpGetTimeOfTheDay(sntp_server, callback)

        if result == PicoTCPAdapter.ERROR:
            raise LibraryException("ERROR", PicoTCPAdapter.ERROR, PicoTCPAdapter.LIBRARY_NAME)
        else:
            return result

    def slaacv4ClaimIP(self, device_name, callback):
        """ Claims slaacv4 IP """
        result = pycoclib.slaacv4ClaimIP(device_name, callback)

        return self.check_error(result)

    def slaacv4UnregisterIP(self):
        """ Unregisters slaacv4 IP """
        result = pycoclib.slaacv4UnregisterIP()

        return self.check_error(result)

    def dnsNameServer(self, dns_name, flags):
        """ Starts a DNS """
        result = pycoclib.dnsNameServer(dns_name, flags)

        return self.check_error(result)

    def dnsGetAddr(self, url, callback):
        """ Gets address from given url """
        result = pycoclib.dnsGetAddr(url, callback)

        return self.check_error(result)

    def dnsGetName(self, address, callback):
        """ Gets name from given address """
        result = pycoclib.dnsGetAddr(address, callback)

        return self.check_error(result)

    def filterIpv4Add(self, device_name, protocol, out_address, out_mask, in_address, in_mask, out_port, in_port, priority, tos, action):
        """ filterIpv4Add, returns filter_id """
        result = pycoclib.filterIpv4Add(device_name, protocol, out_address, out_mask, in_address, in_mask, out_port, in_port, priority, tos, action)

        return self.check_error(result)

    def filterIpv4Del(self, filter_id):
        """ filterIpv4Del """
        result = pycoclib.filterIpv4Del(filter_id)

        return self.check_error(result)

    def olsrAdd(self, device_name):
        """ olsrAdd specified device ONLY IF PICOTCP WAS COMPILED WITH THIS """
        result = pycoclib.olsrAdd(device_name)

        return self.check_error(result)

    def aodvAdd(self, device_name):
        """ aodvAdd specified device """
        result = pycoclib.aodvAdd(device_name)

        return self.check_error(result)

    def pppSetSerialRead(self, device_name, callback):
        """ pppSetSerialRead """
        result = pycoclib.pppSetSerialRead(device_name, callback)

        return self.check_error(result)

    def pppSetSerialWrite(self, device_name, callback):
        """ pppSetSerialWrite """
        result = pycoclib.pppSetSerialWrite(device_name, callback)

        return self.check_error(result)

    def pppSetSerialSpeed(self, device_name, callback):
        """ pppSetSerialSpeed """
        result = pycoclib.pppSetSerialSpeed(device_name, callback)

        return self.check_error(result)

    def pppSetAPN(self, device_name, apn):
        """ pppSetAPN """
        result = pycoclib.pppSetAPN(device_name, apn)

        return self.check_error(result)

    def pppSetUsername(self, device_name, username):
        """ pppSetUsername
        username is an object
        """ #TODO find out what username looks like
        result = pycoclib.pppSetAPN(device_name, apn)

        return self.check_error(result)

    def pppSetPassword(self, device_name, password):
        """ pppSetPassword
        password is an object
        """ #TODO find out what password looks like
        result = pycoclib.pppSetPassword(device_name, password)

        return self.check_error(result)

    def pppConnect(self, device_name):
        """ pppConnect """
        result = pycoclib.pppSetPassword(device_name)

        return self.check_error(result)

    def pppDisconnect(self, device_name):
        """ pppDisconnect """
        result = pycoclib.pppDisconnect(device_name)

        return self.check_error(result)

    def mdnsInit(self, hostname, address, callback):
        """ mdnsInit """
        result = pycoclib.mdnsInit(hostname, address, callback)

        return self.check_error(result)

    def mdnsGetHostname(self):
        """ mdnsGetHostname """
        result = pycoclib.mdnsGetHostname()

        return self.check_error(result)

    def mdnsSetHostname(self, url):
        """ mdnsSetHostname """
        result = pycoclib.mdnsSetHostname(url)

        return self.check_error(result)

    def mdnsClaim(self, rtree, callback):
        """ mdnsClaim """
        result = pycoclib.mdnsClaim(rtree, callback)

        return self.check_error(result)

    def mdnsGetRecord(self, url, record_type, callback):
        """ mdnsGetRecord """
        result = pycoclib.mdnsGetRecord(url, record_type, callback)

        return self.check_error(result)

    def mdnsRecordCreate(self, name, url, data, data_length, record_type, ttl, flags):
        """ mdnsRecordCreate """
        result = pycoclib.mdnsRecordCreate(url, name, url, data, data_length, record_type, ttl, flags)

        return self.check_error(result)

    def mdnsIsHostnameRecord(self, name):
        """ mdnsIsHostnameRecord """
        result = pycoclib.mdnsIsHostnameRecord(url, name)

        return self.check_error(result)

    def dnssdInit(self, hostname, address, callback):
        """ dnssdInit """
        result = pycoclib.dnssdInit(url, hostname, address, callback)

        return self.check_error(result)

    def dnssdRegisterService(self, service_name, service_type, port, name, ttl, callback):
        """ dnssdRegisterService """
        result = pycoclib.dnssdRegisterService(service_name, service_type, port, name, ttl, callback)

        return self.check_error(result)

    def dnssdKVVectorAdd(self, name, key, value):
        """ dnssdKVVectorAdd """
        result = pycoclib.dnssdKVVectorAdd(name, key, value)

        return self.check_error(result)

    def tftpListen(self, family, callback):
        """ tftpListen """
        result = pycoclib.tftpListen(family, callback)

        return self.check_error(result)

    def tftpRejectRequest(self, address, port, error, message):
        """ tftpRejectRequest """
        result = pycoclib.tftpRejectRequest(address, port, error, message)

        return self.check_error(result)

    def tftpSessionSetup(self, family, filename, address):
        """ tftpSessionSetup """
        result = pycoclib.tftpSessionSetup(family, filename, address)

        return self.check_error(result)

    def tftpParseRequestArgs(self, filename, data_len, note):
        """ tftpParseRequestArgs """
        result = pycoclib.tftpParseRequestArgs(filename, data_len, note)

        return self.check_error(result)

    def tftpSetOption(self, session, option_type, note):
        """ tftpSetOption """
        result = pycoclib.tftpSetOption(session, option_type, note)

        return self.check_error(result)

    def tftpGetOption(self, session, option_type):
        """ tftpGetOption """
        result = pycoclib.tftpGetOption(session, option_type)

        return self.check_error(result)

    def tftpSend(self, session, note):
        """ tftpSend """
        result = pycoclib.tftpSend(session, note)

        return self.check_error(result)

    def tftpGetFileSize(self, session):
        """ tftpGetFileSize """
        result = pycoclib.tftpGetFileSize(session)

        return self.check_error(result)

    def tftpAbort(self, session, error, message):
        """ tftpAbort """
        result = pycoclib.tftpAbort(session, error, message)

        return self.check_error(result)

    def tftpCloseServer(self, session, error, message):
        """ tftpCloseServer """
        result = pycoclib.tftpCloseServer(session, error, message)

        return self.check_error(result)

    def tftpAppSetup(self, address, port, family):
        """ tftpAppSetup """
        result = pycoclib.tftpAppSetup(address, port, family)

        return self.check_error(result)

    def tftpAppStartRx(self, session, name):
        """ tftpAppStartRx """
        result = pycoclib.tftpAppStartRx(session, name)

        return self.check_error(result)

    def tftpAppStartTx(self, session, name):
        """ tftpAppStartRx """
        result = pycoclib.tftpAppStartTx(session, name)

        return self.check_error(result)

    def tftpGet(self, session):
        """ tftpGet """
        result = pycoclib.tftpGet(session)

        return self.check_error(result)

    def tftpPut(self, session, data):
        """ tftpPut """
        result = pycoclib.tftpPut(session, data)

        return self.check_error(result)

    def tftpStartTx(self, port, filename, callback, session, note):
        """ tftpStartTx """
        result = pycoclib.tftpStartTx(port, filename, callback, session, note)

        return self.check_error(result)

    def tftpStartRx(self, port, filename, callback, session, note):
        """ tftpStartRx """
        result = pycoclib.tftpStartRx(port, filename, callback, session, note)

        return self.check_error(result)

    def createKvVector(self, name):
        """ createKvVector """
        result = pycoclib.createKvVector(name)

        return self.check_error(result)

    def deleteKvVector(self, name):
        """ deleteKvVector """
        result = pycoclib.deleteKvVector(name)

        return self.check_error(result)

    def createRTree(self, name):
        """ createRTree """
        result = pycoclib.createRTree(name)

        return self.check_error(result)

    def deleteRTree(self, name):
        """ delteRTree """
        result = pycoclib.deleteRTree(name)

        return self.check_error(result)

    def getPicoError(self):
        """ getPicoError """
        result = pycoclib.getPicoError()

        return self.check_error(result)

    def isNetmaskIp4(self, address):
        """ isNetmaskIp4 """
        result = pycoclib.isNetmaskIp4(address)

        return self.check_error(result)

    def isUnicastIp4(self, address):
        """ isUnicastIp4 """
        result = pycoclib.isNetmaskIp4(address)

        return self.check_error(result)

    def findSourceIp4(self, address):
        """ findSourceIp4 """
        result = pycoclib.isNetmaskIp4(address)

        return self.check_error(result)

    def linkFindIp4(self, address):
        """ linkFindIp4 """
        result = pycoclib.linkFindIp4(address)

        return self.check_error(result)

    def natEnableIp4(self, name):
        """ natEnableIp4 """
        result = pycoclib.natEnableIp4(name)

        return self.check_error(result)

    def natDisableIp4(self):
        """ natDisableIp4 """
        result = pycoclib.natDisableIp4()

        return self.check_error(result)

    def portForwardIp4(self, public_address, public_port, private_address, private_port, protocol, persistent):
        """ portForwardIp4 """
        result = pycoclib.portForwardIp4(public_address, public_port, private_address, private_port, protocol, persistent)

        return self.check_error(result)

    def isMulticastIp6(self, address):
        """ isMulticastIp6 """
        result = pycoclib.isMulticastIp6(address)

        return self.check_error(result)

    def isUnicastIp6(self, address):
        """ isUnicastIp6 """
        result = pycoclib.isUnicastIp6(address)

        return self.check_error(result)

    def isGlobalIp6(self, address):
        """ isGlobalIp6 """
        result = pycoclib.isGlobalIp6(address)

        return self.check_error(result)

    def isUniqueLocalIp6(self, address):
        """ isUniqueLocalIp6 """
        result = pycoclib.isUniqueLocalIp6(address)

        return self.check_error(result)

    def isSiteLocalIp6(self, address):
        """ isSiteLocalIp6 """
        result = pycoclib.isSiteLocalIp6(address)

        return self.check_error(result)

    def isLinkLocalIp6(self, address):
        """ isLinkLocalIp6 """
        result = pycoclib.isLinkLocalIp6(address)

        return self.check_error(result)

    def isLocalHostIp6(self, address):
        """ isLocalHostIp6 """
        result = pycoclib.isLoaclHostIp6(address)

        return self.check_error(result)

    def isUnspecifiedIp6(self, address):
        """ isUnspecifiedIp6 """
        result = pycoclib.isUnspecifiedIp6(address)

        return self.check_error(result)

    def findSourceIp6(self, address):
        """ findSourceIp6 """
        result = pycoclib.findSourceIp6(address)

        return self.check_error(result)

