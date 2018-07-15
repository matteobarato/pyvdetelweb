#!/usr/bin/env python

class Adapter:

    NOT_IMPLEMENTED = "Not yet implemented"

    def __init__(self):
        print "initing adapter"
        pass

    def testfunc(self):
        print "I'm an adapter"

    def deleteLink4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def deleteLink6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def deleteSocketBox(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def createDevice(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def createMreq(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def deleteMreq(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def createMreqSource(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def deleteMreqSource(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def createKvVector(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def deleteKvVector(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def createRTree(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def stackTick(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def idle(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def getError(self): #was getPicoError
        raise NotImplementedError(NOT_IMPLEMENTED)

    def isNetmaskIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def isUnicastIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def findSourceIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def natEnableIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def natDisableIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def linkFindIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def linkGetIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def linkDelIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def routeAddIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def routeDelIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def portForwardIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def routeGetGatewayIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def pingStartIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def pingAbortIp4(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def isMulticastIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def isUnicastIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def isGlobalIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def isUniqueLocalIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def isSiteLocalIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def isLocalHostIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def isUnspecifiedIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def findSourceIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def linkFindIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def linkAddIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def linkAddIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def routeAddIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def routeDelIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def routingEnableIpv6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def routingDisableIpv6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def routeGetGatewayIp6(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketOpen(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketBind(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketConnect(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketSend(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketRecv(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketRecvFrom(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketRecvFromExt(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketWrite(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketRead(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketClose(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketShutdown(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketListen(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketAccept(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketSendTo(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketSendToExt(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketgetName(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketGetPeerName(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketSetOption(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketSetOptionMreq(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketSetOptionMreqSource(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def socketGetOption(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def dhcpClientInitiate(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def dhcpClientAbort(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def dhcpServerInitiate(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def dhcpServerDestroy(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def sntpSync(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def sntpGetTimeOfTheDay(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def slaacv4UnregisterIP(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def dnsNameServer(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def dnsGetAddr(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def dnsGetName(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def filterIpv4Add(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def filterIpv4Del(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def olsrAdd(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def aodvAdd(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def pppSetSerialread(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def pppSetSerialWrite(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def pppSetSerialSpeed(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def pppSetAPN(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def pppSetUsername(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def pppSetPassword(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def pppConnect(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def pppDisconnect(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def dnssdInit(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def dnssdRegisterService(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def dnssdKVVectorAdd(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def mdnsInit(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def mdnsGetHostname(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def mdnsSetHostname(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def mdnsClaim(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def mdnsGetRecord(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def mdnsRecordCreate(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def mdnsIsHostnameRecord(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpListen(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpRejectRequest(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpSessionSetup(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpSetOption(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpGetOption(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpParseRequestArgs(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpSend(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpCloseServer(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpAppSetup(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpAppStartRx(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpAppStartTx(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpGet(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpPut(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpStartTx(self):
        raise NotImplementedError(NOT_IMPLEMENTED)

    def tftpStartRx(self):
        raise NotImplementedError(NOT_IMPLEMENTED)
