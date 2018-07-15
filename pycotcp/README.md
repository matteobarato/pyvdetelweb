PycoTCP Module Repository
========================

About the Internet of Threads
------------
PycoTCP is based on two main libraries that implement the "Internet of Threads" concept, or IoT&T (Internet of Threads and Things).

It's goal is to simplify IoT&T experimentations so that anyone can start prototyping with this new approach to networking. To do so, it provides a tool that allows execution of any existing Python scripts that use the 'socket' python module as an IoT&T process, with it's own address and connected to an existing VDE (Virtual Distributed Ethernet)

It has many possible applications, such as:
   * Run multiple Servers on the same machine on the same port, using different IP Addresses.
   * Create a network of processes (for example, simulate a IoT network with multiple processes acting as the real IoT nodes)
   * Run multiple Web Servers on a single host that interact with each other as if they were on a real network

Specifically, using PycoTCP:
   * Run existing Python scripts that use the socket module with their own IP Address
   * Use existing Python WSGI Libraries to create simple Web Servers on the Internet of Threads

Installation
------------

### Install all required Dependencies ###

#### Tested on Debian 8.5.0: ####
   * git
   * make
   * gcc
   * vde2
   * libvdeplug-dev
   * python-pip
   * python-dev
   * libffi-dev

Then use pip to get install these modules:
   * setuptools

Ex.:

```bash
sudo apt-get install git make gcc vde2 libvdeplug-dev python-pip python-dev libffi-dev

sudo pip install setuptools
```

### Install optional dependencies ###

#### To use FDPicoTCP ####

First install FDPicoTCP, then install or reinstall PycoTCP

To install it, visit [FDPicoTCP's git repository](https://github.com/exmorse/fd_picotcp)

#### To use LWIPv6 ####

Use apt-get to install (on Debian):
   * liblwipv6-dev
Then install or reinstall PycoTCP

For more information on LWIPv6, visit [VDE's wiki, Virtual Square](http://wiki.v2.cs.unibo.it/)

### From the command line ###

First clone the repository:

```bash
git clone --recursive https://github.com/LuigiPower/pycotcp.git
cd pycotcp
```

#### Install for the current user: ####

```bash
make install
```

##### NOTE #####
If you install as an user, the executables are installed in ~/.local/bin/ which, by default, is not in your PATH environment variable.

#### Install for all users: ####

```bash
make globalinstall #root permissions required
```

If all required libraries are installed correctly, the setup should build the latest version of PicoTCP, then install PycoTCP itself using PicoTCP.
(NOTE: This will be changed in a future patch, when PicoTCP will be moved from the required dependencies to the optional ones using CFFI instead of it's source code)

Installation - Troubleshooting
------------------------------

For errors during the setup, first check if the error that pops up is due to PicoTCP, VDE or PycoTCP itself.

For PicoTCP errors refer to [PicoTCP's git repository](https://github.com/tass-belgium/picotcp)

For FDPicoTCP errors refer to [FDPicoTCP's git repository](https://github.com/exmorse/fd_picotcp)

For VDE errors refer to [VDE's wiki, Virtual Square](http://wiki.v2.cs.unibo.it/)

If it's a problem relative to PycoTCP, ensure you installed all the required dependencies on the target system. If nothing works out, [Contact the author](mailto:federico.giuggioloni@gmail.com) and he will try to fix your problem, maybe in a new update to the library.


Uninstall
---------
If pycoTCP was installed as user

```bash
pip uninstall pycotcp
```

else, run the same command with root permissions

```bash
sudo pip uninstall pycotcp
```

NOTE: This is also required to remove old versions

Usage
-----

### Explicitly ###

In any Python Script:

```python
from pycotcp.picotcpadapter import PicoTCPAdapter
from pycotcp.fdpicotcpadapter import FDPicoTCPAdapter
from pycotcp.lwipv6adapter import Lwipv6Adapter

from pycotcp.pycotcp import Pyco
from pycotcp.pycotcp import DeviceInfo
import pycotcp.socket as socket

#One context type
context = PicoTCPAdapter()
#context = FDPicoTCPAdapter()
#context = Lwipv6Adapter()

ioth = Pyco(context=context)
device = DeviceInfo(context=context).with_type(devtype) \
        .with_name(device) \
        .with_path(filename) \
        .with_address(address) \
        .with_netmask(netmask) \
        .create()

ioth.start_handler() #Only required for PicoTCPAdapter

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, context=context)
sock.bind(("10.40.0.55", "5500"))
#Code using standard socket API
...
...
...

ioth.stop_handler() #Only required for PicoTCPAdapter
```
### Implicitly, using pyco-wrapper ###

Write code as a normal Python script with sockets:

```python
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.bind(("0.0.0.0", 8090))
s.listen(3)

while True:
    try:
        connection, address = s.accept()
        buf = connection.recv(64)
        if len(buf) > 0:
            print buf
            break
    except:
        print "exception"
        break

s.close()
```

If the VDE is not setup yet, run it with administrator privileges and the -c flag:

```bash
pyco-wrapper <python-script> -a <address> -d <device_name> -f <vde_switch path> -l <library_name> -c # -c to create the VDE (requires permissions)
#Library name can be one of: picotcp, fdpicotcp, lwipv6 [defaults to picotcp]

#Example:
pyco-wrapper script.py -a 10.40.0.32 -d web0 -f /tmp/web0.ctl -l picotcp -c
```

Or start a vde manually using the pyco-vde-start utility that is installed with PycoTCP:

```bash
pyco-vde-setup start <address/maskbits> <vde_switch control file to create>

#Example:
pyco-vde-setup start 10.40.0.1/24 /tmp/vde.ctl
```

To stop an existing VDE, use vdeterm (will be simplified in the future through 'pyco-vde-setup stop')

```bash
vdeterm <path to .mgmt file (same path as the .ctl file)>

#Example:
>vdeterm /tmp/vde.mgmt
VDE switch V.2.3.2
(C) Virtual Square Team (coord. R. Davoli) 2005,2006,2007 - GPLv2
> shutdown
```

If a vde_switch already exists, run pyco-wrapper without -c and with the existing switch:

```bash
#Expects a vde_switch to exist at /tmp/vde.ctl
pyco-wrapper script.py -a 10.40.0.32 -d web0 -f /tmp/vde.ctl -l picotcp
```

### For existing python scripts that use Socket ###

Run this command (requires root permissions while using -c flag as it creates a working VDE for this process):

```bash
pyco-wrapper script.py -a <address> -d <dev_name> -f <dev_file_path> -l <library_name> -c #Where library name is picotcp, fdpicotcp or lwipv6 as usual
```

```bash
pyco-wrapper -h #For parameter explanations
```

