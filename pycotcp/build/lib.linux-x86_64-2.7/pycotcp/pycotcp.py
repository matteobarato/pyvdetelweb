#!/usr/bin/env python

""" pycotcp main module
    Contains DeviceInfo and Pyco

    :platform: Unix, Windows
    :synopsis: Main module

.. moduleauthor:: Federico Giuggioloni <federico.giuggioloni@gmail.com>

"""

from picotcpadapter import PicoTCPAdapter
from threading import Thread
from time import sleep

class DeviceInfo:
    """ Class DeviceInfo
        Contains all information related to a single Device
        device_type (str): Type of the device (vde, tun, tap)
        device_name (str): Name of the device
        device_path (str): Path of the device
        address (str): Address of the device (not a tuple)
        netmask (str): Netmask of the device
        is_ipv6 (bool): Whether or not IPv6 is enabled on this Device
    """

    #Possible init with default values
    #def __init__(self, device_type = "vde", device_name = "pic0", device_path = "/tmp/pic0.ctl", address = "10.40.0.10", netmask = "255.255.255.0"):
    def __init__(self, context = None, device_type = None, device_name = None, device_path = None, address = None,  netmask = None, is_ipv6 = False):
        """ Initializes a device

        Args:

        Kwargs:
            context (Adapter): Adapter to use, if None it defaults to PicoTCPAdapter
            device_type (str): Type of device to create
            device_name (str): Name of device to create
            device_path (str): Path of device to create
            address (str): Address of device to create
            netmask (str): Netmask of device to create
            is_ipv6 (bool): True for IPv6, False otherwise. Defaults to False.

        """
        if context is None:
            context = PicoTCPAdapter()

        self.context = context
        self.device_type = device_type
        self.device_name = device_name
        self.device_path = device_path
        self.address = address
        self.netmask = netmask
        self.is_ipv6 = is_ipv6

        self.created = False

    def with_type(self, device_type):
        self.device_type = device_type
        return self

    def with_name(self, device_name):
        self.device_name = device_name
        return self

    def with_path(self, device_path):
        self.device_path = device_path
        return self

    def with_address(self, address):
        if ":" in address:
            self.is_ipv6 = True
        else:
            self.is_ipv6 = False

        self.address = address
        return self

    def with_netmask(self, netmask):
        if ":" in netmask:
            self.is_ipv6 = True
        else:
            self.is_ipv6 = False

        self.netmask = netmask
        return self

    def set_ipv6(self, is_ipv6):
        self.is_ipv6 = is_ipv6
        return self

    def create(self):
        """ Creates the device """
        print "Creating device %s, which is %s and %s" % (self.device_name, self.is_ipv6, self.device_path)
        if not self.created:
            self.context.createDevice(self.device_type, self.device_name, self.device_path)

            if self.address:
                if self.is_ipv6:
                    self.context.linkAddIp6(self.device_name, self.address, self.netmask)
                else:
                    self.context.linkAddIp4(self.device_name, self.address, self.netmask)
            self.created = True

        return self

    def destroy(self):
        """ Deletes the device """
        print "Destroying device %s" % self.device_name
        if self.created:
            if self.address:
                if self.is_ipv6:
                    self.context.linkDelIp6(self.device_name, self.address)
                else:
                    self.context.linkDelIp4(self.device_name, self.address)

            self.context.deleteDevice(self.device_name)
            self.created = False

        return self

    def update(self):
        """ Deletes and recreates the device """
        self.delete()
        self.create()

class Pyco:

    #_pycotcp_initialized = False

    def _empty_func(name = None):
        """ Empty function to set as default for on_create, on_tick and on_detroy """
        pass

    def __init__(self, on_create = _empty_func, on_tick = _empty_func, on_destroy = _empty_func, context = None, auto_start = False):
        """ Initialize PycoTCP instance
        Pass 3 functions: on_create, on_tick, on_destroy
        on_create(pycotcp)
        on_tick(pycotcp)
        on_destroy(pycotcp)
        if context is omitted, it defaults to PicoTCPAdapter
        """

        if context is None:
            context = PicoTCPAdapter()

        self.context = context
        self.on_create = on_create
        self.on_tick = on_tick
        self.on_destroy = on_destroy
        self.handler_thread = None

        # Should not be necessary: init is handled in adapter's __init__()
        #if not Pyco._pycotcp_initialized:
        #    self.context.init()
        #    Pyco._pycotcp_initialized = True

        if auto_start:
            self.start_handler()

    def wait_completion(self):
        self.handler_thread.join()

    def start_handler(self, async = True):
        """ Starts pycotcp handler thread """

        # Stops the handler_thread if it still exists
        if self.handler_thread is not None:
            self.stop_handler() #TODO ricontrollare il join...

        self.running = True
        if async:
            self.handler_thread = Thread(target = self.handler)
            self.handler_thread.setDaemon(True)
            self.handler_thread.start()
        else:
            self.handler()

        return self.handler_thread

    def stop_handler(self, wait_end = False):
        """ Stops pycotcp handler thread """
        self.running = False
        if self.handler_thread:
            if wait_end:
                self.handler_thread.join()
            self.handler_thread = None

    def handler(self):
        """ Handler function used by the pycotcp handler thread
        Starts by calling the on_create function defined in __init__
        on_tick is called on each tick
        on_destroy is called when thread is no longer running
        """
        self.on_create(self)
        while self.running:
            self.context.stackTick()
            sleep(0.002)
            #self.context.idle()
            self.on_tick(self)
        self.on_destroy(self)

    def start_ping(self, address, packet_count = 10, time_interval = 1000, timeout = 3000, packet_size = 64, callback = _empty_func, is_ipv6 = False):
        """ Starts pinging target address with specified parameters """
        ping_id = None

        print "Starting ping %s %d %d %d %d" % (address, packet_count, time_interval, timeout, packet_size)

        if is_ipv6:
            ping_id = self.context.pingStartIp6(address, packet_count, time_interval, timeout, packet_size, callback)
        else:
            ping_id = self.context.pingStartIp4(address, packet_count, time_interval, timeout, packet_size, callback)

        return ping_id

    def stop_ping(self, ping_id, is_ipv6 = False):
        """ Stops ping with id ping_id """

        print "Aborting ping %s" % (str(ping_id))

        if is_ipv6:
            self.context.pingAbortIp6(ping_id)
        else:
            self.context.pingAbortIp4(ping_id)

