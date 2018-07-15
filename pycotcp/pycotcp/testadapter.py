#!/usr/bin/env python
from picotcpadapter import PicoTCPAdapter
from lwipv6adapter import Lwipv6Adapter

pico = PicoTCPAdapter()
lwip = Lwipv6Adapter()

pico.testfunc()
lwip.testfunc()
