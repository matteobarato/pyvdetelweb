#!/usr/bin/env python

import os
import sys
import argparse
import runpy
import time

#from pycotcp.pycotcp import Pyco
#from pycotcp.pycotcp import DeviceInfo
#from pycotcp.socket import Socket

try:
    from pycotcp.picotcpadapter import PicoTCPAdapter
except ImportError:
    print >> sys.stderr, "PicoTCP Not installed, it will NOT be available"

try:
    from pycotcp.fdpicotcpadapter import FDPicoTCPAdapter
except ImportError:
    print >> sys.stderr, "FDPicoTCP Not installed, it will NOT be available"

try:
    from pycotcp.lwipv6adapter import Lwipv6Adapter
except ImportError:
    print >> sys.stderr, "LWIPv6 Not installed, it will NOT be available"

parser = argparse.ArgumentParser(description = "Starts PycoTCP helper thread before the specified script is run, stops it afterwards. Also creates a device on which this process will run.")
parser.add_argument("-a", "--address", metavar = "addr", type = str, required = False, help = "Address of the device that will be created on the VDE")
parser.add_argument("-n", "--netmask", metavar = "netmask", type = str, required = False, help = "Netmask of the device that will be created on the VDE")
parser.add_argument("-f", "--filename", metavar = "file", type = str, required = False, help = "path of the vde_switch, will be created if it doesn't exist")
parser.add_argument("-d", "--device", metavar = "dev", type = str, required = False, help = "Device to be created on the VDE")
parser.add_argument("-t", "--devtype", metavar = "devtype", type = str, required = False, help = "Type of the device to be created on the VDE")
parser.add_argument("-l", "--library", metavar = "library", type = str, required = False, default = "picotcp", choices = ["picotcp", "lwipv6", "fdpicotcp"], help = "Library to be used for IoT&T")
parser.add_argument("-p", "--parameters", metavar = "param", type = str, required = False, nargs = "+", help = "Parameters for the script to run")
parser.add_argument("script", metavar = "script", type = str, help = "Script to run inside VDE")

args = parser.parse_args()

address = args.address
netmask = args.netmask
filename = args.filename
device = args.device
devtype = args.devtype
parameters = args.parameters

if not address:
    address = "10.40.0.55"

if not netmask:
    netmask = "255.255.255.0"

if not filename:
    filename = "/tmp/pic0.ctl"

if not device:
    device = "vde0"

if not devtype:
    devtype = "vde"

if not parameters:
    parameters = []

def sleep_a_bit(pyco):
    time.sleep(0.001)

current_milli_time = lambda: int(round(time.time() * 1000))
last_tick = current_milli_time()

def _debug_print_tick_time(pyco):
    global last_tick
    delta_time = current_milli_time() - last_tick
    sys.stdout.write("%sms | " % delta_time)
    sys.stdout.flush()
    last_tick = current_milli_time()

#context = None
#
#if args.library == "picotcp":
#    context = PicoTCPAdapter()
#elif args.library == "lwipv6":
#    context = Lwipv6Adapter()
#elif args.library == "fdpicotcp":
#    context = FDPicoTCPAdapter()

os.environ["PYCOTCP_CONTEXT"] = args.library
os.environ["PYCOTCP_DEVTYPE"] = devtype
os.environ["PYCOTCP_DEVICE"] = device
os.environ["PYCOTCP_FILENAME"] = filename
os.environ["PYCOTCP_NETMASK"] = netmask
os.environ["PYCOTCP_ADDRESS"] = address
#Socket.context = context

#ioth = Pyco(on_tick = _debug_print_tick_time)
#ioth = Pyco(context=context)
#device = DeviceInfo(context=context).with_type(devtype) \
#        .with_name(device) \
#        .with_path(filename) \
#        .with_address(address) \
#        .with_netmask(netmask) \
#        .create()
#
#if args.library == "picotcp":
#    ioth.start_handler()

print "running %s" % args.script

sys.argv = sys.argv[:1] #TODO leave 'pyco-runner' as program name, or change it to the called script?
sys.argv = sys.argv + parameters

runpy.run_path(args.script, run_name = "__main__")

print "Stopping script, stopping handler"
#TODO stop_handler da spostare in socket IN QUALCHE MANIERA....?
#ioth.stop_handler()
