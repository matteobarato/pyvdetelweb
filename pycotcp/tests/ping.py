#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import unittest

class BasicTestSuite(unittest.TestCase):
    """Basic test cases.
    Only start after running
    sudo vde_switch -s /tmp/pic0.ctl -m 777 -t vde0
    sudo ip addr add 10.40.0.1 dev vde0
    """

    #@unittest.skip("Things, Cose")
    def test_ping(self):
        """ Tests ping to 10.40.0.1, thus requires a VDE setup.
        a vde_switch with ip address set to 10.40.0.1 is enough
        """
        global error
        global done
        global number_of_pings
        error = False
        done = False
        number_of_pings = 5

        def callback_ping(dest, size, seq, time, ttl, err):
            global error
            global done
            global number_of_pings

            number_of_pings = number_of_pings - 1

            if err != 0:
                error = True

            if number_of_pings == 0:
                done = True

        def ping_on_create(pyco):
            global ping_id

            ping_id = pyco.start_ping("10.40.0.1", callback = callback_ping)

        def ping_on_tick(pyco):
            if done:
                pyco.stop_handler()

        def ping_on_destroy(pyco):
            global ping_id

            pyco.stop_ping(ping_id)

        ioth = Pyco(ping_on_create, ping_on_tick, ping_on_destroy)

        device = DeviceInfo().with_type("vde") \
                        .with_name("pic0") \
                        .with_path("/tmp/pic0.ctl") \
                        .with_address("10.40.0.10") \
                        .with_netmask("255.255.255.0") \
                        .create()

        device.destroy()

        ioth.start_handler(async = False)

        print "Ping done all ticks"
        assert done and not error

    @unittest.skip("DHCP not implemented yet")
    def test_dhcp(self):
        """ Tests DHCP Server and Client """

        def dhcp_callback(code, address, netmask, nameserver, xid):
            pass

        def dhcp_on_create(pyco):
            pass

        def dhcp_on_tick(pyco):
            pass

        def dhcp_on_destroy(pyco):
            pass

        ioth = Pyco(dhcp_on_create, dhcp_on_tick, dhcp_on_destroy)

        device = DeviceInfo().with_type("vde") \
                        .with_name("pic0") \
                        .with_path("/tmp/pic0.ctl") \
                        .with_address("10.40.0.10") \
                        .with_netmask("255.255.255.0") \
                        .create()

        #TODO implement dhcp methods in Pyco and a way to store it's info

        device.destroy()

        assert True

    def test_sockets(self):
        """ Tests UDP and TCP sockets """

        global error
        global done
        error = True
        done = False

        ioth = Pyco()

        device = DeviceInfo().with_type("vde") \
                        .with_name("pic0") \
                        .with_path("/tmp/pic0.ctl") \
                        .with_address("10.40.0.55") \
                        .with_netmask("255.255.255.0") \
                        .create()

        print "Creating sockets"
        sock_udp = "udpsocket"
        receiver_udp = "udpreceiver"
        sock_tcp = "tcpsocket"

        udpsocket = Socket(proto = "udp")
        udpreceiver = Socket(proto = "udp")

        print "Binding"
        udpsocket.bind(("10.40.0.50", "6668"))
        udpreceiver.bind(("10.40.0.55", "6667"))

        print "Connecting"
        udpsocket.connect(("10.40.0.55", "6667"))

        print "Sending data"
        udpsocket.send("Hello there")

        ioth.start_handler(async = True)

        udpreceiver.listen(5)

        while not done:
            result = udpreceiver.recv() #TODO trovare il modo di farlo bloccante
            if result[0] and result[0] == "Hello there":
                print "Result: %s" % str(result[0])
                error = False
                done = True

        ioth.stop_handler()

        udpsocket.close()
        udpreceiver.close()

        device.destroy()

        assert done and not error

if __name__ == '__main__':
    if __package__ is None:
        import sys
        from os import path
        sys.path.append(path.dirname(path.dirname(path.abspath(__file__))))
        from pycotcp.pycotcp import Pyco
        from pycotcp.pycotcp import DeviceInfo
        from pycotcp.pycotcp import Socket
        from pycotcp.picotcpadapter import PicoTCPAdapter
    else:
        from ..pycotcp.pycotcp import Pyco
        from ..pycotcp.pycotcp import DeviceInfo
        from pycotcp.pycotcp import Socket
        from ..pycorcp.picotcpadapter import PicoTCPAdapter

    unittest.main()

