/**********************************************************************
Copyright (C) <2015>  <Fabio Franzoso>

This file is part of PycoTCP.

PycoTCP is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PycoTCP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PycoTCP.  If not, see <http://www.gnu.org/licenses/>
**********************************************************************/
#include <Python.h>
#include <fcntl.h>
#include <ctype.h>
//
#include <sys/types.h>
#include <sys/stat.h>
//
#include "pico_stack.h"
#include "pico_config.h"
#include "pico_ipv4.h"
#include "pico_icmp4.h"
#include "pico_ipv6.h"
#include "pico_icmp6.h"
#include "pico_device.h"
#include "pico_dev_vde.h"
#include "pico_dev_tap.h"
#include "pico_dev_tun.h"
#include "pico_socket.h"
#include "pico_nat.h"
#include "pycoutils.h"
#include "pico_dhcp_server.h"
#include "pico_dhcp_client.h"
#include "pico_sntp_client.h"
#include "pico_slaacv4.h"
#include "pico_ipfilter.h"
#include "pico_aodv.h"
#include "pico_olsr.h"
#include "pico_dns_client.h"
#include "pico_dev_ppp.h"
#include "pico_dns_sd.h"
#include "pico_tftp.h"
#include "pico_mdns.h"
#include "pico_dns_common.h"

static PyObject *py_ping_callback = NULL;
static PyObject *py_socket_callback = NULL;
static PyObject *py_dhcp_callback = NULL;
static PyObject *py_sntp_callback = NULL;
static PyObject *py_slaacv4_callback = NULL;
static PyObject *py_dns_callback = NULL;
static PyObject *py_ppprw_callback = NULL;
static PyObject *py_pppspeed_callback = NULL;
static PyObject *py_dnssd_callback = NULL;
static PyObject *py_mdns_callback = NULL;
static PyObject *py_tftplisten_callback = NULL;
static PyObject *py_tftprxtx_callback = NULL;

int deviceCount = 0;
int autoSocketCount = 0;
int autoLinkCount = 0;
int autoSessionCount = 0;
int autoNoteCount = 0;
uint32_t dhcpclient_xid; //da migliorare
#define TFTP_PAYLOAD_SIZE 512
uint16_t portext;
int fd;
//da migliorare?
int syncro;

extern dlist *head_dev;
extern dlist *pos_dev;
extern l4List *pos_link4;
extern l6List *pos_link6;
extern mList *pos_mreq;
extern msList *pos_mreqsrc;
extern kvvList *pos_kvv;
extern tftpList *pos_tftps;
extern mrList *pos_mdnsr;
extern rtList *pos_rtree;
extern nList *pos_note;

///////////////////////////// IPV4 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static PyObject* isNetmaskIp4(PyObject* self,PyObject *args){
    struct pico_ip4 mask;
    int result = 0;
    const char *addr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv4(addr, &mask.addr);

    result = pico_ipv4_valid_netmask(mask.addr);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* isUnicastIp4(PyObject* self,PyObject *args){
    struct pico_ip4 mask;
    int result = 0;
    const char *addr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv4(addr, &mask.addr);

    result = pico_ipv4_is_unicast(mask.addr);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* findSourceIp4(PyObject* self,PyObject *args){
    struct pico_ip4 ip;
    struct pico_ip4 *result;
    const char *addr;
    char *srcaddr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv4(addr, &ip.addr);
    result = pico_ipv4_source_find(&ip);
    if (result != NULL ){
        pico_ipv4_to_string(srcaddr,result->addr);
        return Py_BuildValue("s", srcaddr);
    }else{
        return Py_BuildValue("i", ERROR);
    }
}
static PyObject* linkFindIp4(PyObject* self,PyObject *args){
    struct pico_ip4 ip;
    struct pico_device *result;
    const char *addr;
    char *srcaddr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv4(addr, &ip.addr);
    result = pico_ipv4_link_find(&ip);
    if (result != NULL ){
        return Py_BuildValue("s", result->name);
    }else{
        return Py_BuildValue("i", ERROR);
    }
}
static PyObject* natEnableIp4(PyObject* self,PyObject *args){
    char *sname;
    int res;
    if (!PyArg_ParseTuple(args, "s", &sname))
        return Py_BuildValue("i", PARSE_FAIL);
    link4Find(sname);
    res = pico_ipv4_nat_enable(pos_link4->link);
    if (res == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* natDisableIp4(PyObject* self){
    int res;
    res = pico_ipv4_nat_disable();
    return Py_BuildValue("i", res);

}
static PyObject* portForwardIp4(PyObject* self,PyObject *args){

    int result = 0;
    char *spub_port;
    char *spriv_port;
    char *sproto;
    char *spersistant;
    char *spub_addr;
    char *spriv_addr;
    struct pico_ip4 pub_addr;
    struct pico_ip4 priv_addr;
    uint8_t proto = 0;
    uint8_t persistant = 0;

    if (!PyArg_ParseTuple(args, "ssssss", &spub_addr,&spub_port,&spriv_addr,&spriv_port,&sproto,&spersistant))
        return Py_BuildValue("i", PARSE_FAIL);
    if (strcmp(sproto,"icmp4")==IS_OK){
        proto = PICO_PROTO_ICMP4;
    }else{
        if (strcmp(sproto,"tcp") == IS_OK){
            proto = PICO_PROTO_TCP;
        }else{
            if (strcmp(sproto,"udp") == IS_OK){
                proto = PICO_PROTO_UDP;
            }
        }
    }
    if (strcmp(spersistant,"add")==IS_OK){
        persistant = PICO_NAT_PORT_FORWARD_ADD;
    }else{
        if (strcmp(spersistant,"del") == IS_OK){
            persistant = PICO_NAT_PORT_FORWARD_DEL;
        }
    }
    pico_string_to_ipv4(spub_addr, &pub_addr.addr);
    pico_string_to_ipv4(spriv_addr, &priv_addr.addr);

    result = pico_ipv4_port_forward(pub_addr,short_be(atoi(spub_port)),priv_addr,short_be(atoi(spriv_port)),proto,persistant);
    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static void dummy_ping_callback(struct pico_icmp4_stats *s){
    char dest[50];
    pico_ipv4_to_string(dest,s->dst.addr);
    PyObject *arglist = Py_BuildValue("(siiiii)",dest,s->size,s->seq,s->time,s->ttl,s->err);
    PyEval_CallObject(py_ping_callback,arglist);

    Py_DECREF(arglist);
}
static PyObject* pingStartIp4(PyObject *obj,PyObject *args){

    PyObject *pycompobj;

    int id=0;
    char *dest;
    int count;
    int interval;
    int timeout;
    int size;

    if (!PyArg_ParseTuple(args, "siiiiO", &dest,&count, &interval,&timeout,&size,&pycompobj))
        return NULL;

    if (!PyCallable_Check(pycompobj)) {
        PyErr_SetString(PyExc_TypeError, "no callback");
    }
    else {
        py_ping_callback = pycompobj;
    }
    id = pico_icmp4_ping(dest,count,interval,timeout,size,dummy_ping_callback);
    if (id == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", id);
    }
}
static PyObject* pingAbortIp4(PyObject* self,PyObject *args){
    struct pico_ip4 mask;
    int result = 0;
    const char *addr;
    int id;
    if (!PyArg_ParseTuple(args, "i", &id))
        return Py_BuildValue("i", PARSE_FAIL);
    result = pico_icmp4_ping_abort(id);
    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", id);
    }

}
static PyObject* linkAddIp4(PyObject* self,PyObject *args){
    static struct pico_device pico_dev;
    dlist devicePos;
    char *sname;
    char *saddr;
    char *smask;
    struct pico_ip4 ip;
    struct pico_ip4 mask;

    if (!PyArg_ParseTuple(args, "sss", &sname,&saddr,&smask))
        return Py_BuildValue("i", PARSE_FAIL);

    deviceFind(sname);
    pico_string_to_ipv4(saddr, &ip.addr);
    pico_string_to_ipv4(smask, &mask.addr);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    if (pico_ipv4_link_add(pos_dev->device, ip, mask) == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("s", pos_dev->name);
    }

}
static PyObject* linkGetIp4(PyObject* self,PyObject *args){
    char *saddr;
    char *sname;
    struct pico_ipv4_link *templink;
    struct pico_ip4 ip;
    int res;
    if (!PyArg_ParseTuple(args, "ss",&sname,&saddr))
        return Py_BuildValue("i", PARSE_FAIL);


    pico_string_to_ipv4(saddr, &ip.addr);
    templink = pico_ipv4_link_get(&ip);
    if (templink == NULL){
        return Py_BuildValue("i", ERROR);
    }
    link4Add(templink,sname);
    return Py_BuildValue("s", sname);
}
static PyObject* linkDelIp4(PyObject* self,PyObject *args){
    static struct pico_device pico_dev;
    dlist *devicePos;
    int result = 0;
    char *sname;
    char *saddr;
    struct pico_ip4 ip;

    if (!PyArg_ParseTuple(args, "ss", &sname,&saddr))
        return Py_BuildValue("i", PARSE_FAIL);

    deviceFind(sname);
    pico_string_to_ipv4(saddr, &ip.addr);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    result = pico_ipv4_link_del(pos_dev->device,ip);
    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* routeAddIp4(PyObject* self,PyObject *args){

    int result = 0;
    int metric;
    char *saddr;
    char *smask;
    char *sgateway;
    char *slink;
    struct pico_ip4 ip;
    struct pico_ip4 mask;
    struct pico_ip4 gateway;


    if (!PyArg_ParseTuple(args, "sssis", &saddr,&smask,&sgateway,&metric,&slink))
        return Py_BuildValue("i", PARSE_FAIL);


    pico_string_to_ipv4(saddr, &ip.addr);
    pico_string_to_ipv4(smask, &mask.addr);
    pico_string_to_ipv4(sgateway, &gateway.addr);
    link4Find(slink);
    if (pos_link4 == NULL){
        result = pico_ipv4_route_add(ip,mask,gateway,metric,NULL);
    }else{
        result = pico_ipv4_route_add(ip,mask,gateway,metric,pos_link4->link);
    }
    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", result);
    }

}
static PyObject* routeDelIp4(PyObject* self,PyObject *args){

    int result = 0;
    int metric;
    char *saddr;
    char *smask;
    char *sgateway;
    char *slink;
    struct pico_ip4 ip;
    struct pico_ip4 mask;
    struct pico_ip4 gateway;


    if (!PyArg_ParseTuple(args, "ssi", &saddr,&smask,&metric))
        return Py_BuildValue("i", PARSE_FAIL);


    pico_string_to_ipv4(saddr, &ip.addr);
    pico_string_to_ipv4(smask, &mask.addr);
    pico_string_to_ipv4(sgateway, &gateway.addr);

    result = pico_ipv4_route_del(ip,mask,metric);
    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", result);
    }

}
static PyObject* routeGetGatewayIp4(PyObject* self,PyObject *args){

    char saddr[IPDIM];
    struct pico_ip4 *ip;
    struct pico_ip4 resultip;

    if (!PyArg_ParseTuple(args, "s", &saddr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv4(&saddr, &ip->addr);
    resultip = pico_ipv4_route_get_gateway(ip);
    pico_ipv4_to_string(saddr,resultip.addr);
    if (strcmp("0.0.0.0",saddr) == 0){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("s", saddr);
    }

}
///////////////////////////// IPV6 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
static PyObject* isMulticastIp6(PyObject* self,PyObject *args){
    struct pico_ip6 ip;
    int result = 0;
    const char *addr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(addr, ip.addr);

    result = pico_ipv6_is_multicast(ip.addr);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* isUnicastIp6(PyObject* self,PyObject *args){
    struct pico_ip6 ip;
    int result = 0;
    const char *addr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(addr, ip.addr);

    result = pico_ipv6_is_unicast(ip.addr);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* isGlobalIp6(PyObject* self,PyObject *args){
    struct pico_ip6 ip;
    int result = 0;
    const char *addr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(addr, ip.addr);

    result = pico_ipv6_is_global(ip.addr);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* isUniqueLocalIp6(PyObject* self,PyObject *args){
    struct pico_ip6 ip;
    int result = 0;
    const char *addr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(addr, ip.addr);

    result = pico_ipv6_is_uniquelocal(ip.addr);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* isSiteLocalIp6(PyObject* self,PyObject *args){
    struct pico_ip6 ip;
    int result = 0;
    const char *addr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(addr, ip.addr);

    result = pico_ipv6_is_sitelocal(ip.addr);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* isLinkLocalIp6(PyObject* self,PyObject *args){
    struct pico_ip6 ip;
    int result = 0;
    const char *addr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(addr, ip.addr);

    result = pico_ipv6_is_linklocal(ip.addr);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* isLocalHostIp6(PyObject* self,PyObject *args){
    struct pico_ip6 ip;
    int result = 0;
    const char *addr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(addr, ip.addr);

    result = pico_ipv6_is_localhost(ip.addr);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* isUnspecifiedIp6(PyObject* self,PyObject *args){
    struct pico_ip6 ip;
    int result = 0;
    const char *addr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(addr, ip.addr);

    result = pico_ipv6_is_unspecified(ip.addr);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* findSourceIp6(PyObject* self,PyObject *args){
    struct pico_ip6 ip;
    struct pico_ip6 *result;
    const char *addr;
    char *srcaddr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(addr, ip.addr);
    result = pico_ipv6_source_find(&ip);
    if (result != NULL ){
        pico_ipv6_to_string(srcaddr,result->addr);
        return Py_BuildValue("s", srcaddr);
    }else{
        return Py_BuildValue("i", NOT_OK);
    }
}
static PyObject* linkAddIp6(PyObject* self,PyObject *args){
    static struct pico_device *pico_dev;
    char *sname;
    char *saddr;
    char *smask;
    char slname[50]="";
    struct pico_ip6 ip;
    struct pico_ip6 mask;
    struct pico_ipv6_link *templink;

    if (!PyArg_ParseTuple(args, "sss", &sname,&saddr,&smask))
        return Py_BuildValue("i", PARSE_FAIL);

    deviceFind(sname);
    pico_string_to_ipv6(saddr, ip.addr);
    pico_string_to_ipv6(smask, mask.addr);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    templink = pico_ipv6_link_add(pos_dev->device, ip, mask);
    if (templink == NULL){
        return Py_BuildValue("i", ERROR);
    }else{
        sprintf(slname, "autolink6_%d", autoLinkCount);
        autoLinkCount++;
        link6Add(templink,slname);
        return Py_BuildValue("ss", pos_dev->name,slname);
    }

}
static PyObject* linkDelIp6(PyObject* self,PyObject *args){
    static struct pico_device *pico_dev;
    int result = 0;
    char *sname;
    char *saddr;
    struct pico_ip6 ip;

    if (!PyArg_ParseTuple(args, "ss", &sname,&saddr))
        return Py_BuildValue("i", PARSE_FAIL);

    deviceFind(sname);
    pico_string_to_ipv6(saddr, ip.addr);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    result = pico_ipv6_link_del(pos_dev->device,ip);

    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* linkFindIp6(PyObject* self,PyObject *args){
    struct pico_ip6 ip;
    struct pico_device *result;
    const char *addr;
    char *srcaddr;

    if (!PyArg_ParseTuple(args, "s", &addr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(addr, ip.addr);
    result = pico_ipv6_link_find(&ip);
    if (result != NULL ){
        return Py_BuildValue("s", result->name);
    }else{
        return Py_BuildValue("i", ERROR);
    }
}
static PyObject* routeAddIp6(PyObject* self,PyObject *args){

    int result = 0;
    int metric;
    char *saddr;
    char *smask;
    char *sgateway;
    char *slink;
    struct pico_ip6 ip;
    struct pico_ip6 mask;
    struct pico_ip6 gateway;


    if (!PyArg_ParseTuple(args, "sssis", &saddr,&smask,&sgateway,&metric,&slink))
        return Py_BuildValue("i", PARSE_FAIL);


    pico_string_to_ipv6(saddr, ip.addr);
    pico_string_to_ipv6(smask, mask.addr);
    pico_string_to_ipv6(sgateway, gateway.addr);
    link6Find(slink);
    result = pico_ipv6_route_add(ip,mask,gateway,metric,pos_link6->link);
    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* routeDelIp6(PyObject* self,PyObject *args){

    int result = 0;
    int metric;
    char *saddr;
    char *smask;
    char *sgateway;
    char *slink;
    struct pico_ip6 ip;
    struct pico_ip6 mask;
    struct pico_ip6 gateway;


    if (!PyArg_ParseTuple(args, "sssis", &saddr,&smask,&sgateway,&metric,&slink))
        return Py_BuildValue("i", PARSE_FAIL);


    pico_string_to_ipv6(saddr, ip.addr);
    pico_string_to_ipv6(smask, mask.addr);
    pico_string_to_ipv6(sgateway, gateway.addr);
    link6Find(slink);
    result = pico_ipv6_route_del(ip,mask,gateway,metric,pos_link6->link);
    if (result == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* routeGetGatewayIp6(PyObject* self,PyObject *args){

    char saddr[IPDIM];
    struct pico_ip6 *ip;
    struct pico_ip6 resultip;

    if (!PyArg_ParseTuple(args, "s", &saddr))
        return Py_BuildValue("i", PARSE_FAIL);

    pico_string_to_ipv6(saddr, ip->addr);

    resultip = pico_ipv6_route_get_gateway(ip);

    pico_ipv6_to_string(saddr,resultip.addr);

    return Py_BuildValue("s", saddr);

}
static PyObject* routingEnableIpv6(PyObject* self,PyObject *args){

    char *sdev;
    int res;

    if (!PyArg_ParseTuple(args, "s", &sdev))
        return Py_BuildValue("i", PARSE_FAIL);

    deviceFind(sdev);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    res = pico_ipv6_dev_routing_enable(pos_dev->device);

    if (res == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
static PyObject* routingDisableIpv6(PyObject* self,PyObject *args){

    char *sdev;
    int res;

    if (!PyArg_ParseTuple(args, "s", &sdev))
        return Py_BuildValue("i", PARSE_FAIL);

    deviceFind(sdev);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    res = pico_ipv6_dev_routing_disable(pos_dev->device);
    if (res == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }

}
//////////////////////////// SOCKET  ///////////////////////////////////////////////////////////////////////////////////////////
static void dummy_wakeup_callback(uint16_t ev, struct pico_socket *s){
    char type[50];
    char socket[50];

    type[0] = 0;
    socket[0] = 0;

    socketFindAddress(s, socket);
    sbList* pos = socketBoxFind(socket);

    printf("WAKEUP %lu %s\n", s, socket);
    PyObject *arglist = Py_BuildValue("(is)", ev, socket);
    if(pos != NULL && pos->socbox->callback != NULL)
    {
        PyEval_CallObject(pos->socbox->callback, arglist);
    }

    Py_DECREF(arglist);
}
static PyObject* socketOpen(PyObject *obj,PyObject *args){
    PyObject *pycompobj;

    struct socket_box *tempbox;
    char *sname;
    char *snet;
    char *sproto;
    uint16_t net = 0;
    uint16_t proto = 0;
    int ret = 0;
    tempbox = calloc(1, sizeof(struct socket_box));
    if (!tempbox) {
        return Py_BuildValue("i", NOT_OK);
    }
    tempbox->s = NULL;

    if (!PyArg_ParseTuple(args, "sssO", &sname, &snet, &sproto, &pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (strcmp(snet,"ipv4")==IS_OK){
        net = PICO_PROTO_IPV4;
    }else{
        if (strcmp(snet,"ipv6") == IS_OK){
            net = PICO_PROTO_IPV6;
        }
    }
    if (strcmp(sproto,"udp")==IS_OK){
        proto = PICO_PROTO_UDP;
    }else{
        if (strcmp(sproto,"tcp") == IS_OK){
            proto = PICO_PROTO_TCP;
        }
    }
    if (!PyCallable_Check(pycompobj)) {
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_socket_callback = pycompobj;
    }
    tempbox->s = pico_socket_open(net, proto, dummy_wakeup_callback);
    if (tempbox->s == NULL){
        return Py_BuildValue("i", ERROR);
    }
    socketBoxAdd(tempbox, sname, pycompobj);
    return Py_BuildValue("i", IS_OK);
}
static PyObject* socketBind(PyObject* self,PyObject *args){
    char *ssocket;
    char *sladdr;
    char *slport;
    int ret = 0;
    struct pico_ip4 inaddr4 = {
        0
    };
    struct pico_ip6 inaddr6 = {{0}};
    uint16_t listen_port = 0;

    if (!PyArg_ParseTuple(args, "sss",&ssocket,&sladdr,&slport)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    listen_port = short_be(atoi(slport));
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    if (strchr (sladdr, ':') != NULL){
        pico_string_to_ipv6(sladdr, inaddr6.addr);
        //pico_ipv6_to_string(test,inaddr6.addr);
        ret = pico_socket_bind(pos->socbox->s, &inaddr6, &listen_port);
    }else{
        pico_string_to_ipv4(sladdr, inaddr4.addr);
        ret = pico_socket_bind(pos->socbox->s, &inaddr4, &listen_port);
    }
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}

static PyObject* socketConnect(PyObject* self,PyObject *args){
    char *ssocket;
    char *sraddr;
    char test[40];
    char *srport;
    int ret = 0;

    if (!PyArg_ParseTuple(args, "sss", &ssocket, &sraddr, &srport))
    {
        return Py_BuildValue("i", PARSE_FAIL);
    }

    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL)
    {
        return Py_BuildValue("i", NOT_OK);
    }

    pos->socbox->sport = short_be(atoi(srport));
    if (strchr(sraddr, ':') != NULL)
    {
        int err = pico_string_to_ipv6(sraddr, &pos->socbox->dst.ip6.addr);
        ret = pico_socket_connect(pos->socbox->s, &pos->socbox->dst.ip6, pos->socbox->sport);
    }
    else
    {
        int err = pico_string_to_ipv4(sraddr, &pos->socbox->dst.ip4.addr);
        ret = pico_socket_connect(pos->socbox->s, &pos->socbox->dst.ip4, pos->socbox->sport);


        pico_ipv4_to_string(test, pos->socbox->dst.ip4.addr);
    }

    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }

    return Py_BuildValue("i", IS_OK);
}

static PyObject* socketSend(PyObject* self,PyObject *args){
    const char *addr;
    char *ssocket;
    char *buffer;
    int len;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "ssi",&ssocket,&buffer,&len)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    fflush(stdout);
    ret = pico_socket_send(pos->socbox->s, buffer, len);
    return Py_BuildValue("i", ret);
}
static PyObject* socketRecv(PyObject* self,PyObject *args){
    const char *addr;
    char *ssocket;
    char *buffer = NULL;
    char returnstring[2000]="";
    int len;
    int read = 0;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "si",&ssocket,&len)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    buffer = calloc(1,len);
    do{
        read = pico_socket_recv(pos->socbox->s,buffer,len);
        if (read > IS_OK){
            strcat(returnstring,buffer);
        }
    }while(read >IS_OK);
    free(buffer);
    return Py_BuildValue("s", returnstring);

}
static PyObject* socketRead(PyObject* self,PyObject *args){
    const char *addr;
    char *ssocket;
    char *buffer = NULL;
    char returnstring[2000]="";
    int len;
    int read = 0;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "si",&ssocket,&len)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    buffer = calloc(1,len);
    do{
        read = pico_socket_read(pos->socbox->s,buffer,len);
        if (read > 0 ){
            strcat(returnstring,buffer);
        }
    }while(read > 0 );
    free(buffer);
    return Py_BuildValue("s", returnstring);

}

static PyObject* socketWrite(PyObject* self,PyObject *args){
    char *ssocket;
    char *buffer;
    int len;
    int write = 0;

    if (!PyArg_ParseTuple(args, "ssi", &ssocket, &buffer, &len)){
        return Py_BuildValue("i", PARSE_FAIL);
    }

    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }

    write = pico_socket_write(pos->socbox->s, buffer, len);
    return Py_BuildValue("i", write);
}

static PyObject* socketSendTo(PyObject* self,PyObject *args){
    char *sraddr;
    char *ssocket;
    char *buffer;
    char *srport;;
    struct pico_ip4 raddr4 = {
        0
    };
    struct pico_ip6 raddr6 = {{0}};
    int len = 100;
    uint16_t rport = 0;
    int sendto = 0;

    if (!PyArg_ParseTuple(args, "ssiss",&ssocket,&buffer,&len,&sraddr,&srport)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    rport = short_be(atoi(srport));
    if (strchr (sraddr, ':') != NULL){
        pico_string_to_ipv6(sraddr,&raddr6.addr);
        sendto = pico_socket_sendto(pos->socbox->s,buffer,len,&raddr6.addr,rport);
    }else{
        pico_string_to_ipv4(sraddr,&raddr4.addr);
        sendto = pico_socket_sendto(pos->socbox->s,buffer,len,&raddr4.addr,rport);
    }
    if (sendto == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", sendto);
    }
}
static PyObject* socketRecvFrom(PyObject* self,PyObject *args){
    const char *addr;
    char *ssocket;
    char *stype;
    char *buffer = NULL;
    char speer[IPDIM];
    uint16_t rport = 0;
    char returnstring[2000]="";
    int len;
    struct pico_ip4 raddr4 = {
        0
    };
    struct pico_ip6 raddr6 = {{0}};
    int read = 0;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sis",&ssocket,&len,&stype)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    buffer = calloc(1,len);
    if (strcmp(stype,IPV6) == 0){
        do{
            read = pico_socket_recvfrom(pos->socbox->s,buffer,len,&raddr6.addr,&rport);
            if (read > IS_OK){
                strcat(returnstring,buffer);
            }
        }while(read >IS_OK);
        pico_ipv6_to_string(speer,raddr6.addr);
    }else{
        if (strcmp(stype,IPV4) == 0){
            do{
                read = pico_socket_recvfrom(pos->socbox->s,buffer,len,&raddr4.addr,&rport);
                if (read > IS_OK){
                    strcat(returnstring,buffer);
                }
            }while(read >IS_OK);
            pico_ipv4_to_string(speer,raddr4.addr);
        }else{
            return Py_BuildValue("i", NOT_OK);
        }
    }
    free(buffer);
    return Py_BuildValue("ssi", returnstring,speer,short_be(rport));

}
static PyObject* socketSendToExt(PyObject* self,PyObject *args){
    char *sraddr;
    char *ssocket;
    char *buffer;
    char *srport;;
    struct pico_ip4 inaddr4 = {
        0
    };
    struct pico_ip6 inaddr6 = {{0}};
    struct pico_msginfo info = { };
    int len = 100;
    int ttl = 0;
    int qos = 0;
    uint16_t rport = 0;
    int sendto = 0;

    if (!PyArg_ParseTuple(args, "ssissii",&ssocket,&buffer,&len,&sraddr,&srport,&ttl,&qos)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (ttl >= IS_OK){
       info.ttl = ttl;
    }
    if (qos >= IS_OK){
        info.tos = qos;
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    rport = short_be(atoi(srport));
    if (strchr (sraddr, ':') != NULL){
        pico_string_to_ipv6(sraddr,&inaddr6.addr);
        sendto = pico_socket_sendto_extended(pos->socbox->s,buffer,len,&inaddr6.addr,rport,&info);
    }else{
        pico_string_to_ipv4(sraddr,&inaddr4.addr);
        sendto = pico_socket_sendto_extended(pos->socbox->s,buffer,len,&inaddr4.addr,rport,&info);
    }
    if (sendto == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", sendto);
    }
}
static PyObject* socketRecvFromExt(PyObject* self,PyObject *args){
    const char *addr;
    char *ssocket;
    char *stype;
    char *buffer = NULL;
    char speer[IPDIM];
    uint16_t rport = 0;
    char returnstring[2000]="";
    int len;
    struct pico_ip4 inaddr4 = {
        0
    };
    struct pico_ip6 inaddr6 = {{0}};
    int read = 0;
    struct pico_msginfo info = { };
    if (!PyArg_ParseTuple(args, "sis",&ssocket,&len,&stype)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    buffer = calloc(1,200);
    if (strcmp(stype,IPV6) == 0){
        do{
            read = pico_socket_recvfrom_extended(pos->socbox->s,buffer,len,&inaddr6.addr,&rport,&info);
            if (read > IS_OK){
                strcat(returnstring,buffer);
            }
        }while(read >IS_OK);
        pico_ipv6_to_string(speer,inaddr6.addr);
    }else{
        if (strcmp(stype,IPV4) == 0){
            do{
                read = pico_socket_recvfrom_extended(pos->socbox->s,buffer,len,&inaddr4.addr,&rport,&info);
                if (read > IS_OK){
                    strcat(returnstring,buffer);
                }
            }while(read >IS_OK);
            pico_ipv4_to_string(speer,inaddr4.addr);
        }else{
            return Py_BuildValue("i", NOT_OK);
        }
    }
    free(buffer);
    return Py_BuildValue("ssisii", returnstring,speer,short_be(rport),info.dev->name,info.ttl,info.tos);

}
static PyObject* socketClose(PyObject* self,PyObject *args){
    int ret = 0;
    char *ssocket;
    if (!PyArg_ParseTuple(args, "s",&ssocket)){
        return Py_BuildValue("i", NOT_OK);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_socket_close(pos->socbox->s);
    socketBoxDelete(ssocket);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* socketShutdown(PyObject* self,PyObject *args){
    int ret = 0;
    char *ssocket;
    char *smode;
    int mode;
    if (!PyArg_ParseTuple(args, "ss",&ssocket,&smode)){
        return Py_BuildValue("i", NOT_OK);
    }
    if (strcmp(smode,"rw") == IS_OK){
        mode = PICO_SHUT_RDWR;
    }
    if (strcmp(smode,"r") == IS_OK){
        mode = PICO_SHUT_RD;
    }
    if (strcmp(smode,"w") == IS_OK){
        mode = PICO_SHUT_WR;
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_socket_shutdown(pos->socbox->s,mode);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* socketListen(PyObject* self,PyObject *args){
    int ret = 0;
    char *ssocket;
    int backlog;
    if (!PyArg_ParseTuple(args, "si",&ssocket,&backlog)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_socket_listen(pos->socbox->s,backlog);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* socketAccept(PyObject* self,PyObject *args){
    PyObject *pycompobj;
    int ret = 0;
    char *ssocket;
    int backlog;
    uint16_t rport = 0;
    char speer[IPDIM];
    char countbuff[100];
    char name[50];
    struct socket_box *tempbox;
    struct pico_ip4 peer = {
        0
    };
    if (!PyArg_ParseTuple(args, "sO", &ssocket, &pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    tempbox = calloc(1, sizeof(struct socket_box));
    if (!tempbox) {
        return Py_BuildValue("i", NOT_OK);
    }
    tempbox->s = NULL;

    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    tempbox->s = pico_socket_accept(pos->socbox->s,&peer.addr,&rport);
    if (tempbox->s == NULL){
        return Py_BuildValue("i", ERROR);
    }
    sprintf(name, "__autosocket_%d", autoSocketCount);
    autoSocketCount++;
    socketBoxAdd(tempbox, name, pycompobj);
    pico_ipv4_to_string(speer,peer.addr);
    return Py_BuildValue("ssi", name, speer, short_be(rport));
}
static PyObject* socketGetName(PyObject* self,PyObject *args){
    int ret = 0;
    char *ssocket;
    int backlog;
    uint16_t lport = 0;
    uint16_t proto = 0;
    char laddrs[IPDIM];
    char stype[10];
    char *stypesel;
    struct pico_ip4 peer4 = {
        0
    };
    struct pico_ip6 peer6 = {{0}};
    if (!PyArg_ParseTuple(args, "ss",&ssocket,&stypesel)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    if (strcmp(stypesel,"ipv6")==0){
        pico_socket_getname(pos->socbox->s,&peer6.addr,&lport,&proto);
        pico_ipv6_to_string(laddrs,peer6.addr);
    }else{
        if(strcmp(stypesel,"ipv4")==0){
            pico_socket_getname(pos->socbox->s,&peer4.addr,&lport,&proto);
            pico_ipv4_to_string(laddrs,peer4.addr);
        }else{
            return Py_BuildValue("i", NOT_OK);
        }
    }
    switch(proto){
        case PICO_PROTO_IPV4:
            strcpy(stype,"ipv4");
            break;
        case PICO_PROTO_IPV6:
            strcpy(stype,"ipv6");
            break;
        default:
            strcpy(stype,"unknown");
            break;
    }
    return Py_BuildValue("sis",laddrs,short_be(lport),stype);
}
static PyObject* socketGetPeerName(PyObject* self,PyObject *args){
    int ret = 0;
    char *ssocket;
    char *stypesel;
    int backlog;
    uint16_t rport = 0;
    uint16_t proto = 0;
    char raddrs[IPDIM];
    char stype[10];
    struct pico_ip4 peer4 = {
        0
    };
    struct pico_ip6 peer6 = {{0}};
    if (!PyArg_ParseTuple(args, "ss",&ssocket,&stypesel)){
        return Py_BuildValue("i", PARSE_FAIL);
    }

    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    if (strcmp(stypesel,"ipv6")==0){
        ret = pico_socket_getpeername(pos->socbox->s,&peer6.addr,&rport,&proto);
        if (ret == NOT_OK){
            return Py_BuildValue("i", ERROR);
        }
        pico_ipv6_to_string(raddrs,peer6.addr);
    }else{
        if(strcmp(stypesel,"ipv4")==0){
            ret = pico_socket_getpeername(pos->socbox->s,&peer4.addr,&rport,&proto);
            if (ret == NOT_OK){
                return Py_BuildValue("i", ERROR);
            }
            pico_ipv4_to_string(raddrs,peer4.addr);
        }else{
            return Py_BuildValue("i", NOT_OK);
        }
    }
    switch(proto){
        case PICO_PROTO_IPV4:
            strcpy(stype,"ipv4");
            break;
        case PICO_PROTO_IPV6:
            strcpy(stype,"ipv6");
            break;
        default:
            strcpy(stype,"unknown");
            break;
    }
    return Py_BuildValue("sis",raddrs,short_be(rport),stype);
}
static PyObject* socketSetOption(PyObject* self,PyObject *args){

    int ret = 0;
    char *ssocket;
    int option;
    int value;
    if (!PyArg_ParseTuple(args, "sii",&ssocket,&option,&value)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_socket_setoption(pos->socbox->s,option,&value);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
/*alcune opzioni condividono lo stesso intero, per dividerle credo funzioni
 * specifiche per mreq*/
static PyObject* socketSetOptionMreq(PyObject* self,PyObject *args){

    int ret = 0;
    char *ssocket;
    int option;
    char *value;
    if (!PyArg_ParseTuple(args, "sis",&ssocket,&option,&value)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    mreqFind(value);
    if (pos_mreq == NULL){
        return Py_BuildValue("i", NOT_OK);
    }else{
        ret = pico_socket_setoption(pos->socbox->s,option,pos_mreq->mreq);
        if (ret == NOT_OK){
            return Py_BuildValue("i", ERROR);
        }
    }
    return Py_BuildValue("i", IS_OK);

}
/*alcune opzioni condividono lo stesso intero, per dividerle credo funzioni
 * specifiche per mreq_source*/
static PyObject* socketSetOptionMreqSource(PyObject* self,PyObject *args){

    int ret = 0;
    char *ssocket;
    int option;
    char *value;
    if (!PyArg_ParseTuple(args, "sis",&ssocket,&option,&value)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    mreqsrcFind(value);
    if (pos_mreqsrc == NULL){
        return Py_BuildValue("i", NOT_OK);
    }else{
        ret = pico_socket_setoption(pos->socbox->s,option,pos_mreqsrc->mreqsrc);
        if (ret == NOT_OK){
            return Py_BuildValue("i", ERROR);
        }
    }
    return Py_BuildValue("i", IS_OK);

}
static PyObject* socketGetOption(PyObject* self,PyObject *args){
    int ret = 0;
    char *ssocket;
    int option;
    int value;
    if (!PyArg_ParseTuple(args, "si",&ssocket,option)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    sbList* pos = socketBoxFind(ssocket);
    if (pos == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_socket_getoption(pos->socbox->s,option,&value);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", value);
    }
}
////////////////////////////// DHCP CLIENT //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void dummy_dhcp_callback(void *arg, int code){
    struct pico_ip4 address, gateway, nameserver;
    char saddress[IPDIM]="";
    char sgateway[IPDIM]="";
    char snameserver[IPDIM]="";
    int type;
    type == code;
    switch(code){
        case PICO_DHCP_SUCCESS:
            address = pico_dhcp_get_address(arg);
            gateway = pico_dhcp_get_gateway(arg);
            nameserver = pico_dhcp_get_nameserver(arg,&index);
            pico_ipv4_to_string(saddress, address.addr);
            pico_ipv4_to_string(sgateway, gateway.addr);
            pico_ipv4_to_string(snameserver, nameserver.addr);
        break;
        case PICO_DHCP_ERROR:
            type == NOT_OK;
        break;
    }
    PyObject *arglist = Py_BuildValue("(isssk)",type,saddress,sgateway,snameserver,dhcpclient_xid);
    PyEval_CallObject(py_dhcp_callback,arglist);
    Py_DECREF(arglist);
}
static PyObject* dhcpClientInitiate(PyObject* self,PyObject *args){
    char *sdevice;
    PyObject *pycompobj;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&sdevice,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_dhcp_callback = pycompobj;
    }
    deviceFind(sdevice);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }


    ret = pico_dhcp_initiate_negotiation(pos_dev->device,dummy_dhcp_callback,&dhcpclient_xid);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", ret);
    }
}
static PyObject* dhcpClientAbort(PyObject* self,PyObject *args){
    uint32_t xid;
    PyObject *pycompobj;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "k",&xid)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = pico_dhcp_client_abort(xid);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
////////////////////////////// DHCP SERVER //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static PyObject* dhcpServerInitiate(PyObject* self,PyObject *args){
    char *sdevice;
    char *saddr;
    char *smask;
    char *spools;
    char *spoole;
    char *sleaset;
    int ret = 0;
    struct pico_dhcp_server_setting settings = { 0 };

    if (!PyArg_ParseTuple(args, "ssssss",&sdevice,&saddr,&smask,&spools,&spoole,&sleaset)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    deviceFind(sdevice);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    settings.dev = pos_dev->device;
    pico_string_to_ipv4(saddr,&settings.server_ip.addr);
    pico_string_to_ipv4(smask,&settings.netmask.addr);
    settings.pool_start = (settings.server_ip.addr & settings.netmask.addr) | long_be(atoi(spools));
    settings.pool_end = (settings.server_ip.addr & settings.netmask.addr) | long_be(atoi(spoole));
    settings.lease_time = long_be(atoi(sleaset));
    ret = pico_dhcp_server_initiate(&settings);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* dhcpServerDestroy(PyObject* self,PyObject *args){
    char *sdevice;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&sdevice)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    deviceFind(sdevice);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_dhcp_server_initiate(pos_dev->device);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
////////////////////////////// SNTP //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void dummy_sntp_callback(pico_err_t status){
    char type[15];
    switch(status){
        case PICO_ERR_ENETDOWN:
            strcpy(type,"netdown");
        break;
        case PICO_ERR_ETIMEDOUT:
            strcpy(type,"timedout");
        break;
        case PICO_ERR_EINVAL:
            strcpy(type,"inval");
        break;
        case PICO_ERR_ENOTCONN:
            strcpy(type,"notconn");
        break;
        case PICO_ERR_NOERR:
            strcpy(type,"ok");
        break;
        default:
            strcpy(type,"error");
        break;
    }
    PyObject *arglist = Py_BuildValue("(s)",type);
    PyEval_CallObject(py_sntp_callback,arglist);
    Py_DECREF(arglist);
}
static PyObject* sntpSync(PyObject* self,PyObject *args){
    char *ssntp;
    PyObject *pycompobj;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&ssntp,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_sntp_callback = pycompobj;
    }
    ret = pico_sntp_sync(ssntp,dummy_sntp_callback);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* sntpGetTimeOfTheDay(PyObject* self,PyObject *args){
    struct pico_timeval nptv;
    int ret = 0;

    ret = pico_sntp_gettimeofday(&nptv);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }
    return Py_BuildValue("kk",nptv.tv_sec ,nptv.tv_msec);
}
////////////////////////////// SLAACV4 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void dummy_slaacv4_callback(struct pico_ip4 *ip, uint8_t code){
    char type[15];
    char saddr[IPDIM]="0";
    switch(code){
        case PICO_SLAACV4_SUCCESS:
            strcpy(type,"success");
        break;
        case PICO_SLAACV4_ERROR:
            strcpy(type,"error");
        break;
        default:
            strcpy(type,"undefined");
        break;
    }
    pico_ipv4_to_string(saddr,ip->addr);
    PyObject *arglist = Py_BuildValue("(ss)",type,saddr);
    PyEval_CallObject(py_slaacv4_callback,arglist);
    Py_DECREF(arglist);
}
static PyObject* slaacv4ClaimIP(PyObject* self,PyObject *args){
    char *sdevice;
    PyObject *pycompobj;
    int ret = 0;

    struct pico_device *devo;

    if (!PyArg_ParseTuple(args, "sO",&sdevice,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_slaacv4_callback = pycompobj;
    }
    deviceFind(sdevice);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_slaacv4_claimip(pos_dev->device,dummy_slaacv4_callback);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* slaacv4UnregisterIP(PyObject* self,PyObject *args){
    int ret = 0;
    pico_slaacv4_unregisterip();
    return Py_BuildValue("i", IS_OK);
}
////////////////////////////// DNS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void dummy_dns_callback(char *string, void *arg){
    char buffer[IPDIM];
    uint8_t *id = (uint8_t *) arg;
    if (string == NULL){
        strcpy(buffer,"None");
    }else{
        strcpy(buffer,string);
    }
    PyObject *arglist = Py_BuildValue("(si)",buffer,id);
    PyEval_CallObject(py_dns_callback,arglist);
    Py_DECREF(arglist);
}
static PyObject* dnsNameServer(PyObject* self,PyObject *args){
    char *snameserver;
    char *sflag;
    uint8_t flag;
    struct pico_ip4 nameserver;
    int ret = -1;
    if (!PyArg_ParseTuple(args, "ss",&snameserver,&sflag)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    pico_string_to_ipv4(snameserver,&nameserver.addr);
    if (strcmp(sflag,"add")==0){
        flag = PICO_DNS_NS_ADD;
    }else{
        if (strcmp(sflag,"del")==0){
            flag = PICO_DNS_NS_DEL;
        }else{
           return Py_BuildValue("i", NOT_OK);
        }
    }
    ret = pico_dns_client_nameserver(&nameserver,PICO_DNS_NS_ADD);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* dnsGetAddr(PyObject* self,PyObject *args){
    int ret = 0;
    char *surl;
    PyObject *pycompobj;
    uint8_t getaddr_id = 1;
    if (!PyArg_ParseTuple(args, "sO",&surl,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_dns_callback = pycompobj;
    }
    ret = pico_dns_client_getaddr(surl,dummy_dns_callback,getaddr_id);

    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* dnsGetName(PyObject* self,PyObject *args){
    int ret = 0;
    char *saddr;
    PyObject *pycompobj;
    uint8_t getname_id = 2;
    struct pico_ip4 peer;
    if (!PyArg_ParseTuple(args, "sO",&saddr,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_dns_callback = pycompobj;
    }
    pico_string_to_ipv4(saddr,&peer.addr);
    ret = pico_dns_client_getname(&peer,dummy_dns_callback,getname_id);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
////////////////////////////// IP FILTER //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static PyObject* filterIpv4Add(PyObject* self,PyObject *args){
    char *sdevice;
    char *sproto;

    char *soutaddr;
    char *soutmask;
    char *sinaddr;
    char *sinmask;

    char *soutport;
    char *sinport;
    char *stos;
    char *saction;
    int ret = 0;

    struct pico_ip4 *outaddr;
    struct pico_ip4 *outmask;
    struct pico_ip4 *inaddr;
    struct pico_ip4 *inmask;

    uint8_t proto;
    int8_t priority;
    uint8_t tos;
    uint16_t outport;
    uint16_t inport;
    int action;

    if (!PyArg_ParseTuple(args, "ssssssssiss",
    &sdevice,&sproto,&soutaddr,&soutmask,&sinaddr,&sinmask,
    &soutport,&sinport,&priority,&stos,&saction)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    deviceFind(sdevice);
    if (pos_dev == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    if (strcmp("null",soutaddr)){
        outaddr = NULL;
    }else{
        pico_string_to_ipv4(soutaddr,outaddr->addr);
    }
    if (strcmp("null",soutmask)){
        outmask = NULL;
    }else{
        pico_string_to_ipv4(soutmask,outmask->addr);
    }
    if (strcmp("null",sinaddr)){
        inaddr = NULL;
    }else{
        pico_string_to_ipv4(sinaddr,inaddr->addr);
    }
    if (strcmp("null",sinmask)){
        inmask = NULL;
    }else{
       pico_string_to_ipv4(sinmask,inmask->addr);
    }


    tos=short_be(atoi(stos));
    outport=short_be(atoi(soutport));
    inport=short_be(atoi(sinport));
    /*if (strcmp(saction,"ACCEPT")==0){
        //manca nell'enumerazione?
        //action = filter_action.FILTER
    }*/
    if (strcmp(saction,"PRIORITY")==0){
        action = FILTER_PRIORITY;
    }
    if (strcmp(saction,"REJECT")==0){
        action = FILTER_REJECT;
    }
    if (strcmp(saction,"DROP")==0){
        action = FILTER_DROP;
    }
    if (strcmp(saction,"COUNT")==0){
        action = FILTER_COUNT;
    }
    ret = pico_ipv4_filter_add(pos_dev->device,atoi(sproto),outaddr,
            outmask,inaddr,inmask,outport,inport,priority,tos,action);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", ret);
    }
}
static PyObject* filterIpv4Del(PyObject* self,PyObject *args){
    int ret = 0;
    int id;

    if (!PyArg_ParseTuple(args, "i",&id)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = pico_ipv4_filter_del(id);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
/////////////////////////////OLRS///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//da testare
#ifdef PICO_SUPPORT_OLSR
static PyObject* olsrAdd(PyObject* self,PyObject *args){
    int ret = 0;
    char *sdevice;

    if (!PyArg_ParseTuple(args, "s",&sdevice)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    deviceFind(sdevice);
    ret = pico_olsr_add(pos_dev->device);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
#endif
/////////////////////////////AODV///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//da testare
static PyObject* aodvAdd(PyObject* self,PyObject *args){
    int ret = 0;
    char *sdevice;

    if (!PyArg_ParseTuple(args, "s",&sdevice)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    deviceFind(sdevice);
    ret = pico_aodv_add(pos_dev->device);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
/////////////////////////////PPP///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void dummy_ppprw_callback(struct pico_device *device, void *buf, int len){
    PyObject *arglist = Py_BuildValue("(ssd)",device->name,buf,len);
    PyEval_CallObject(py_ppprw_callback,arglist);
    Py_DECREF(arglist);
}
static void dummy_pppspeed_callback(struct pico_device *device, uint32_t speed){
    PyObject *arglist = Py_BuildValue("(sd)",device->name,speed);
    PyEval_CallObject(py_pppspeed_callback,arglist);
    Py_DECREF(arglist);
}
//da testare
static PyObject* pppSetSerialRead(PyObject* self,PyObject *args){
    char *sdevice;
    PyObject *pycompobj;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&sdevice,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_ppprw_callback = pycompobj;
    }
    deviceFind(sdevice);
    if (pos_dev == NULL){
       return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_ppp_set_serial_read(pos_dev->device,dummy_ppprw_callback);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
//da testare
static PyObject* pppSetSerialWrite(PyObject* self,PyObject *args){
    char *sdevice;
    PyObject *pycompobj;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&sdevice,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_ppprw_callback = pycompobj;
    }
    deviceFind(sdevice);
    if (pos_dev == NULL){
       return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_ppp_set_serial_write(pos_dev->device,dummy_ppprw_callback);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
//da testare
static PyObject* pppSetSerialSpeed(PyObject* self,PyObject *args){
    char *sdevice;
    PyObject *pycompobj;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&sdevice,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_pppspeed_callback = pycompobj;
    }
    deviceFind(sdevice);
    if (pos_dev == NULL){
       return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_ppp_set_serial_write(pos_dev->device,dummy_pppspeed_callback);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
//da testare
static PyObject* pppSetAPN(PyObject* self,PyObject *args){
    char *sdevice;
    char *sapn;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "ss",&sdevice,&sapn)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    deviceFind(sdevice);
    ret = pico_ppp_set_apn(pos_dev->device,sapn);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
//da testare
static PyObject* pppSetUsername(PyObject* self,PyObject *args){
    char *sdevice;
    char *susername;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&sdevice,&susername)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    deviceFind(sdevice);
    ret = pico_ppp_set_username(pos_dev->device,susername);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
//da testare
static PyObject* pppSetPassword(PyObject* self,PyObject *args){
    char *sdevice;
    char *spassoword;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&sdevice,&spassoword)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    deviceFind(sdevice);
    ret = pico_ppp_set_password(pos_dev->device,spassoword);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
//da testare
static PyObject* pppConnect(PyObject* self,PyObject *args){
    char *sdevice;
    char *spassoword;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&sdevice)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    deviceFind(sdevice);
    ret = pico_ppp_connect(pos_dev->device);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
//da testare
static PyObject* pppDisconnect(PyObject* self,PyObject *args){
    char *sdevice;
    char *spassoword;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&sdevice)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    deviceFind(sdevice);
    ret = pico_ppp_disconnect(pos_dev->device);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
/////////////////////////////MDNS///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void dummy_mdns_callback(struct pico_mdns_rtree *tree, char *str, void *arg){
    char *srtree;
    rtreeFindAddress(tree,srtree);
    PyObject *arglist = Py_BuildValue("(ss)",str,srtree);
    PyEval_CallObject(py_mdns_callback,arglist);
    Py_DECREF(arglist);
}
static PyObject* mdnsInit(PyObject* self,PyObject *args){
    char *shostname;
    char *saddr;
    struct pico_ip4 address;
    PyObject *pycompobj;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "ssO",&shostname,&saddr,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_mdns_callback = pycompobj;
    }
    pico_string_to_ipv4(saddr,&address.addr);
    ret = pico_mdns_init(shostname,address,dummy_mdns_callback,NULL);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* mdnsGetHostname(PyObject* self,PyObject *args){
    char *shostname;
    shostname = pico_mdns_get_hostname();
    if (shostname == NULL){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("s", shostname);
    }
}
static PyObject* mdnsSetHostname(PyObject* self,PyObject *args){
    char *surl;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&surl)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = pico_mdns_set_hostname(surl,NULL);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
//da testare
static PyObject* mdnsClaim(PyObject* self,PyObject *args){
    char *srtree;
    PyObject *pycompobj;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&srtree,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_mdns_callback = pycompobj;
    }
    rtreeFind(srtree);
    if (pos_rtree == NULL){
        return Py_BuildValue("i", NOT_OK);
    }

    //migliorabile?
    PICO_MDNS_RTREE_DECLARE(*tmptree);
    PICO_MDNS_RTREE_DECLARE(tmptree2);
    tmptree = pos_rtree->rtree;
    tmptree2 = *tmptree;
    ret = pico_mdns_claim(tmptree2,dummy_mdns_callback,NULL);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* mdnsGetRecord(PyObject* self,PyObject *args){
    char *surl;
    char *stype;
    PyObject *pycompobj;
    uint16_t type;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "ssO",&surl,&stype,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_mdns_callback = pycompobj;
    }
    if (strcmp(stype,"a")==0){
        type = PICO_DNS_TYPE_A;
    }else if(strcmp(stype,"cname")==0){
        type = PICO_DNS_TYPE_CNAME;
    }else if (strcmp(stype,"ptr")==0){
        type = PICO_DNS_TYPE_PTR;
    }else if (strcmp(stype,"txt")==0){
        type = PICO_DNS_TYPE_TXT;
    }else if (strcmp(stype,"aaaa")==0){
        type = PICO_DNS_TYPE_AAAA;
    }else if (strcmp(stype,"srv")==0){
        type = PICO_DNS_TYPE_SRV;
    }else if (strcmp(stype,"nsec")==0){
        type = PICO_DNS_TYPE_NSEC;
    }else if (strcmp(stype,"any")==0){
        type = PICO_DNS_TYPE_ANY;
    }else{
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_mdns_getrecord(surl,type,dummy_mdns_callback,NULL);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
//da trovare sistema
static PyObject* mdnsRecordCreate(PyObject* self,PyObject *args){
    char *sname;
    char *surl;
    char *srdata;
    char *sdatalen;
    char *srtype;
    char *srttl;
    char *sflag;
    struct pico_mdns_record *tmp_mdns = NULL;
    uint16_t datalen;
    uint16_t rtype;
    uint8_t flag;
    uint32_t rttl;
    struct pico_ip4 data4;
    struct pico_ip6 data6;
    if (!PyArg_ParseTuple(args, "sssssss",&sname,&surl,&srdata,&sdatalen,&srtype,&srttl,&sflag)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (strcmp(sflag,"shared")==0){
        flag = PICO_MDNS_RECORD_SHARED;
    }else if (strcmp(sflag,"unique")==0){
        flag = PICO_MDNS_RECORD_UNIQUE;
    }

    rttl = long_be(atoi(srttl));

    if (strcmp(srtype,"a")==0){
        rtype = PICO_DNS_TYPE_A;
        if (strchr (srdata, ':') != NULL){
            pico_string_to_ipv6(srdata, &data6.addr);
            tmp_mdns = pico_mdns_record_create(surl,(data6.addr),PICO_SIZE_IP4,PICO_DNS_TYPE_A,rttl,flag);
        }else{
            pico_string_to_ipv4(srdata, &data4.addr);
            tmp_mdns = pico_mdns_record_create(surl,&(data4.addr),PICO_SIZE_IP4,PICO_DNS_TYPE_A,rttl,flag);
        }
    }else if(strcmp(srtype,"cname")==0){
        rtype = PICO_DNS_TYPE_CNAME;
    }else if (strcmp(srtype,"ptr")==0){
        tmp_mdns = pico_mdns_record_create(surl,srdata,pico_dns_strlen(srdata),PICO_DNS_TYPE_PTR,rttl,flag);
    }else if (strcmp(srtype,"txt")==0){
        return Py_BuildValue("i", NOT_SUPPORTED);
    }else if (strcmp(srtype,"aaaa")==0){
        return Py_BuildValue("i", NOT_SUPPORTED);
    }else if (strcmp(srtype,"srv")==0){
        return Py_BuildValue("i", NOT_SUPPORTED);
    }else if (strcmp(srtype,"nsec")==0){
        return Py_BuildValue("i", NOT_SUPPORTED);
    }else if (strcmp(srtype,"any")==0){
        return Py_BuildValue("i", NOT_SUPPORTED);
    }else{
        return Py_BuildValue("i", NOT_OK);
    }

    if (tmp_mdns == NULL){
        return Py_BuildValue("i", NOT_OK);
    }

    mdnsrAdd(tmp_mdns,sname);
    mdnsrFind(sname);
    if (pos_mdnsr == NULL){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("s", pos_mdnsr->name);
    }
}
static PyObject* mdnsIsHostnameRecord(PyObject* self,PyObject *args){
    char *sname;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&sname)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    mdnsrFind(sname);
    if (pos_mdnsr == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = IS_HOSTNAME_RECORD(pos_mdnsr->record);
    if (ret == 1){
        return Py_BuildValue("i", IS_OK);
    }else{
        return Py_BuildValue("i", NOT_OK);
    }
}
/////////////////////////////DNS SD///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void dummy_dnssd_callback(struct pico_mdns_rtree *tree, char *str, void *arg){
    char *srtree;
    rtreeFindAddress(tree,srtree);
    PyObject *arglist = Py_BuildValue("(ss)",srtree,str);
    PyEval_CallObject(py_dnssd_callback,arglist);

    Py_DECREF(arglist);
}
static PyObject* dnssdInit(PyObject* self,PyObject *args){
    char *shostname;
    char *saddr;
    struct pico_ip4 address;
    PyObject *pycompobj;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "ssO",&shostname,&saddr,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_dnssd_callback = pycompobj;
    }
    pico_string_to_ipv4(saddr,&address.addr);
    ret = pico_dns_sd_init(shostname,address,dummy_dnssd_callback,NULL);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* dnssdRegisterService(PyObject* self,PyObject *args){
    char *sname;
    char *stype;
    char *sport;
    char *sttl;
    char *skvvname;
    uint16_t port;
    uint16_t ttl;
    struct pico_ip4 *address;
    PyObject *pycompobj;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sssssO",&sname,&stype,&sport,&skvvname,&sttl,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_dnssd_callback = pycompobj;
    }
    port = short_be(atoi(sport));
    ttl = short_be(atoi(sttl));
    kvvFind(skvvname);
    if (pos_kvv == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_dns_sd_register_service(sname,stype,port,pos_kvv->vector,ttl,dummy_dnssd_callback,NULL);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* dnssdKVVectorAdd(PyObject* self,PyObject *args){
    struct pico_ip_mreq_source *newmreqsrc;
    PICO_DNS_SD_KV_VECTOR_DECLARE(*tempvector);
    char *sname;
    char *skey;
    char *svalue;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sss",&sname,&skey,&svalue)){
           return Py_BuildValue("i", NOT_OK);
        }
    kvvFind(sname);
    if (pos_kvv == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_dns_sd_kv_vector_add(&pos_kvv->vector,skey,svalue);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
/////////////////////////////TFTP///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void dummy_tftprxtx_callback(struct pico_tftp_session *session, uint16_t event, uint8_t *block, int32_t len, void *arg){
    char ssession[30];
    nList *note = (nList *) arg;
    char *stringblock = (char *) block;
    tftpsFindAddress(session,&ssession);
    if (pos_tftps == NULL){
        strcpy(ssession,"none");
    }
    PyObject *arglist = Py_BuildValue("(sHsis)",ssession,event,stringblock,len,note->name);
    PyEval_CallObject(py_tftprxtx_callback,arglist);
    Py_DECREF(arglist);
}
static void dummy_tftplisten_callback(union pico_address *addr, uint16_t port, uint16_t opcode, char *filename, int32_t len){
    char peer[IPDIM];
    pico_ipv4_to_string(&peer,addr->ip4.addr);
    PyObject *arglist = Py_BuildValue("(sHHsi)",peer,port,opcode,filename,len);
    PyEval_CallObject(py_tftplisten_callback,arglist);
    Py_DECREF(arglist);
}
static PyObject* tftpListen(PyObject* self,PyObject *args){
    char *sfamily;
    PyObject *pycompobj;
    uint16_t family;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sO",&sfamily,&pycompobj)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_tftplisten_callback = pycompobj;
    }

    if (strcmp(sfamily,"ipv4")==0){
        family = PICO_PROTO_IPV4;
    }else if (strcmp(sfamily,"ipv6")==0){
        family = PICO_PROTO_IPV6;
    }
    ret = pico_tftp_listen(family,dummy_tftplisten_callback);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* tftpRejectRequest(PyObject* self,PyObject *args){
    char *saddr;
    char *sport;
    char *serror;
    char *smessage;
    uint16_t port;
    uint16_t error;
    struct pico_ip4 address;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "skss",&saddr,&port,&serror,&smessage)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    pico_string_to_ipv4(saddr,&address.addr);
    if (strcmp(serror,"undef")==0){
        error = TFTP_ERR_UNDEF;
    }else if (strcmp(serror,"enoent")==0){
        error = TFTP_ERR_ENOENT;
    }else if (strcmp(serror,"eacc")==0){
        error = TFTP_ERR_EACC;
    }else if (strcmp(serror,"exceeded")==0){
        error = TFTP_ERR_EXCEEDED;
    }else if (strcmp(serror,"eill")==0){
        error = TFTP_ERR_EILL;
    }else if (strcmp(serror,"etid")==0){
        error = TFTP_ERR_ETID;
    }else if (strcmp(serror,"eexist")==0){
        error = TFTP_ERR_EEXIST;
    }else if (strcmp(serror,"eusr")==0){
        error = TFTP_ERR_EUSR;
    }else if (strcmp(serror,"eopt")==0){
        error = TFTP_ERR_EOPT;
    }else{
       return Py_BuildValue("i", NOT_OK);
    }

    ret = pico_tftp_reject_request(&address,port,error,smessage);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* tftpSessionSetup(PyObject* self,PyObject *args){
    char *saddr;
    char *sfamily;
    char *sfilename;
    uint16_t family;
    int ret = 0;
    char sessioname[50];
    char notename[50];
    struct note_t *note;
    union pico_address naddr;
    struct pico_tftp_session *tmpsession;
    if (!PyArg_ParseTuple(args, "sss",&sfamily,&sfilename,&saddr)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (strcmp(sfamily,"ipv4")==0){
        family = PICO_PROTO_IPV4;
    }else if (strcmp(sfamily,"ipv6")==0){
        family = PICO_PROTO_IPV6;
    }else{
       return Py_BuildValue("i", NOT_OK);
    }
    pico_string_to_ipv4(saddr,&naddr.ip4.addr);
    note = transfer_prepare(&tmpsession, 't', sfilename, &naddr, PICO_PROTO_IPV4);
    sprintf(sessioname, "autosession_%d", autoSessionCount);
    autoSessionCount++;
    sprintf(notename, "autonote_%d", autoNoteCount);
    autoNoteCount++;
    tftpsAdd(tmpsession,sessioname);
    noteAdd(note,notename);
    tftpsFind(sessioname);
    noteFind(notename);
    if (pos_tftps == NULL && pos_note == NULL){
        return Py_BuildValue("i", ERROR);
    }else{
        strcpy(pos_note->name,notename);
        return Py_BuildValue("ss", pos_tftps->name,pos_note->name);
    }
}
static PyObject* tftpParseRequestArgs(PyObject* self,PyObject *args){
    char *sfilename;
    char *snote;
    int len;

    int option;
    int ret;

    if (!PyArg_ParseTuple(args, "sis",&sfilename,&len,&snote)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    noteFind(snote);
    if (pos_note == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_tftp_parse_request_args(sfilename, len, &pos_note->note->options, &pos_note->note->opttimeoutopt, &pos_note->note->optfilesize);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* tftpSetOption(PyObject* self,PyObject *args){
    char *ssession;
    char *stype;
    char *snote;
    uint8_t type;
    int value = -5;
    int ret = 0;
    int isNote = -1;
    if (!PyArg_ParseTuple(args, "sss",&ssession,&stype,&snote)){
        if (!PyArg_ParseTuple(args, "ssi",&ssession,&stype,&value)){
            return Py_BuildValue("i", PARSE_FAIL);
        }
    }else{
        noteFind(snote);
        if(pos_note == NULL){
            return Py_BuildValue("i", NOT_OK);
        }
        isNote = 0;
    }
    if (strcmp(stype,"file")==0){
        type = PICO_TFTP_OPTION_FILE;
    }else if (strcmp(stype,"time")==0){
        type = PICO_TFTP_OPTION_TIME;
    }else{
       return Py_BuildValue("i", NOT_OK);
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    if (isNote == 0){
        if (type == PICO_TFTP_OPTION_FILE ){
            pico_tftp_set_option(pos_tftps->session,type,pos_note->note->optfilesize);
        }else{
            pico_tftp_set_option(pos_tftps->session,type,pos_note->note->opttimeoutopt);
        }
    }else{
        pico_tftp_set_option(pos_tftps->session,type,value);
    }
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* tftpGetOption(PyObject* self,PyObject *args){
    char *ssession;
    char *stype;
    uint8_t type;
    int value;
    int ret = 0;
    struct pico_tftp_session *tmpsession;
    if (!PyArg_ParseTuple(args, "ss",&ssession,&stype)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (strcmp(stype,"file")==0){
        type = PICO_TFTP_OPTION_FILE;
    }else if (strcmp(stype,"time")==0){
        type = PICO_TFTP_OPTION_TIME;
    }else{
       return Py_BuildValue("i", NOT_OK);
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_tftp_get_option(pos_tftps->session,type,&value);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", value);
    }
}
static PyObject* tftpSend(PyObject* self,PyObject *args){
    char *ssession;
    char *snote;
    char *sfilename;
    int len;
    int ret = 0;
    unsigned char txbuf[TFTP_PAYLOAD_SIZE];
    if (!PyArg_ParseTuple(args, "ss",&ssession,&snote)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    noteFind(snote);
    if(pos_note == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    len = read(pos_note->note->fd, txbuf, PICO_TFTP_PAYLOAD_SIZE);
    ret = pico_tftp_send(pos_tftps->session,txbuf,len);
    pos_note->note->filesize = pos_note->note->filesize+ret;
    if (len < PICO_TFTP_PAYLOAD_SIZE){
        close(pos_note->note->fd);
    }
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("ii", ret,pos_note->note->filesize);
    }
}
static PyObject* tftpGetFileSize(PyObject* self,PyObject *args){
    char *ssession;
    int filesize;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&ssession)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_tftp_get_file_size(pos_tftps->session,filesize);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", filesize);
    }
}
static PyObject* tftpAbort(PyObject* self,PyObject *args){
    char *ssession;
    char *serror;
    char *smessage;
    uint16_t error;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "sss",&ssession,&serror,&smessage)){
        return Py_BuildValue("i", PARSE_FAIL);
    }

    if (strcmp(serror,"undef")==0){
        error = TFTP_ERR_UNDEF;
    }else if (strcmp(serror,"enoent")==0){
        error = TFTP_ERR_ENOENT;
    }else if (strcmp(serror,"eacc")==0){
        error = TFTP_ERR_EACC;
    }else if (strcmp(serror,"exceeded")==0){
        error = TFTP_ERR_EXCEEDED;
    }else if (strcmp(serror,"eill")==0){
        error = TFTP_ERR_EILL;
    }else if (strcmp(serror,"etid")==0){
        error = TFTP_ERR_ETID;
    }else if (strcmp(serror,"eexist")==0){
        error = TFTP_ERR_EEXIST;
    }else if (strcmp(serror,"eusr")==0){
        error = TFTP_ERR_EUSR;
    }else if (strcmp(serror,"eopt")==0){
        error = TFTP_ERR_EOPT;
    }else{
       return Py_BuildValue("i", NOT_OK);
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_tftp_abort(pos_tftps->session,error,smessage);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* tftpCloseServer(PyObject* self,PyObject *args){
    int ret = 0;
    ret = pico_tftp_close_server();
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* tftpAppSetup(PyObject* self,PyObject *args){
    char sessioname[50];
    char *saddr;
    char *sport;
    char *sfamily;
    uint16_t port;
    uint16_t family;
    int ret = 0;
    union pico_address naddr;
    int derp;

    struct pico_tftp_session *tmpsession;
    if (!PyArg_ParseTuple(args, "sHs",&saddr,&port,&sfamily)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (strcmp(sfamily,"ipv4")==0){
        family = PICO_PROTO_IPV4;
    }else if (strcmp(sfamily,"ipv6")==0){
        family = PICO_PROTO_IPV6;
    }else{
       return Py_BuildValue("i", NOT_OK);
    }
    pico_string_to_ipv4(saddr,&naddr.ip4.addr);
    tmpsession = pico_tftp_app_setup(&naddr,port,family,&syncro);
    if (tmpsession == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    sprintf(sessioname, "autosession_%d", autoSessionCount);
    autoSessionCount++;
    tftpsAdd(tmpsession,sessioname);
    tftpsFind(sessioname);
    if (pos_tftps == NULL){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("s", pos_tftps->name);
    }
}
static PyObject* tftpAppStartRx(PyObject* self,PyObject *args){
    char *ssession;
    char *sname;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "ss",&ssession,&sname)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_tftp_app_start_rx(pos_tftps->session,sname);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* tftpAppStartTx(PyObject* self,PyObject *args){
    char *ssession;
    char *sfilename;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "ss",&ssession,&sfilename)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_tftp_app_start_tx(pos_tftps->session,sfilename);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* tftpGet(PyObject* self,PyObject *args){
    char *ssession;
    int ret = 0;
    unsigned char txbuf[TFTP_PAYLOAD_SIZE];
    if (!PyArg_ParseTuple(args, "s",&ssession)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_tftp_get(pos_tftps->session,txbuf,TFTP_PAYLOAD_SIZE);
    return Py_BuildValue("i", ret,txbuf);
}
static PyObject* tftpPut(PyObject* self,PyObject *args){
    char *ssession;
    char *buffer;
    int ret = 0;
    unsigned char *txbuf[TFTP_PAYLOAD_SIZE];

    if (!PyArg_ParseTuple(args, "ss",&ssession,&buffer)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_tftp_put(pos_tftps->session,buffer,TFTP_PAYLOAD_SIZE);
    return Py_BuildValue("i", ret);
}
static PyObject* tftpStartTx(PyObject* self,PyObject *args){
    char *ssession;
    char *sfile;
    char *snote;
    PyObject *pycompobj;
    uint16_t port;
    int ret = 0;

    if (!PyArg_ParseTuple(args, "HsOss",&port,&sfile,&pycompobj,&ssession,&snote)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_tftprxtx_callback = pycompobj;
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", -6);
    }
    noteFind(snote);
    if(pos_note == NULL){
        return Py_BuildValue("i", -6);
    }
    ret = pico_tftp_start_tx(pos_tftps->session,port,sfile,dummy_tftprxtx_callback,pos_note);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}
static PyObject* tftpStartRx(PyObject* self,PyObject *args){
    char *ssession;
    char *sport;
    char *sfile;
    char *snote;
    PyObject *pycompobj;
    uint16_t port;
    int ret = 0;

    char *stest;

    //Non ha senso
    if (!PyArg_ParseTuple(args, "HsOss",&port,&sfile,&pycompobj,&ssession,&snote)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    if (!PyCallable_Check(pycompobj)) {
        dbg("NO CALLBACK\n");
        PyErr_SetString(PyExc_TypeError, "no callback");
    }else{
        py_tftprxtx_callback = pycompobj;
    }
    tftpsFind(ssession);
    if(pos_tftps == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    noteFind(snote);
    if(pos_note == NULL){
        return Py_BuildValue("i", NOT_OK);
    }
    ret = pico_tftp_start_rx(pos_tftps->session,port,sfile,dummy_tftprxtx_callback,pos_note);
    if (ret == NOT_OK){
        return Py_BuildValue("i", ERROR);
    }else{
        return Py_BuildValue("i", IS_OK);
    }
}

///////////////////////////// FEDERICO EDITS ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PyObject* fede_init(PyObject* self, PyObject *args) {
    pico_stack_init();
    return Py_BuildValue("i", IS_OK);
}

/////////////////////////////HANDLE///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//TODO is this the best way to prevent reiniting of picotcp?
int doneinit = 0;

static PyObject* init(PyObject* self,PyObject *args){
    if(doneinit == 0)
    {
        pico_stack_init();
        doneinit = 1;
    }
    return Py_BuildValue("i", IS_OK);
}
static PyObject* deleteDevice(PyObject* self,PyObject *args){
    char *sname;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&sname)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = deviceDelete(sname);
    return Py_BuildValue("i", ret);
}
static PyObject* deleteSocketBox(PyObject* self,PyObject *args){
    char *sname;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&sname)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = socketBoxDelete(sname);
    return Py_BuildValue("i", ret);
}
static PyObject* deleteLink4(PyObject* self,PyObject *args){
    char *sname;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&sname)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = link4Delete(sname);
    return Py_BuildValue("i", ret);
}
static PyObject* deleteLink6(PyObject* self,PyObject *args){
    char *sname;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&sname)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = link6Delete(sname);
    return Py_BuildValue("i", ret);
}
static PyObject* deleteMreq(PyObject* self,PyObject *args){
    char *sname;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&sname)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = mreqDelete(sname);
    return Py_BuildValue("i", ret);
}
static PyObject* deleteMreqSource(PyObject* self,PyObject *args){
    char *sname;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&sname)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = mreqsrcDelete(sname);
    return Py_BuildValue("i", ret);
}
static PyObject* createDevice(PyObject* self,PyObject *args){
    static struct pico_device *pico_dev;
    static struct pico_device pico_dev_test;
    char *stype;
    char *sname;
    char *spath;

    unsigned char macaddr[6] = {
        0, 0, 0, 0xa, 0xb, 0x0
    };
    uint16_t *macaddr_low = (uint16_t *) (macaddr + 2 + deviceCount);
    *macaddr_low = (uint16_t)(*macaddr_low ^ (uint16_t)((uint16_t)getpid() & (uint16_t)0xFFFFU));
    dbg("My macaddr base is: %02x %02x\n", macaddr[2], macaddr[3]);
    dbg("My macaddr is: %02x %02x %02x %02x %02x %02x\n", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

    if (PyArg_ParseTuple(args, "sss",&stype,&sname,&spath)){
        if (strcmp("vde",stype) == IS_OK){
            macaddr[4] ^= (uint8_t)(getpid() >> 8);
            macaddr[5] ^= (uint8_t) (getpid() & 0xFF);
            pico_dev = (struct pico_device *) pico_vde_create(spath,sname,macaddr);
            NXT_MAC(macaddr);
        }else{
            return Py_BuildValue("i", NOT_OK);
        }
    }else{
        if (PyArg_ParseTuple(args, "ss",&stype,&sname)){
            if (strcmp("tun",stype) == IS_OK){
                pico_dev = (struct pico_device *) pico_tun_create(sname);
            }else{
                if (strcmp("tap",stype) == IS_OK){
                    pico_dev = (struct pico_device *) pico_tap_create(sname);
                }
                else{
                    return Py_BuildValue("i", NOT_OK);
                }
            }
        }else{
            if (PyArg_ParseTuple(args, "s",&stype)){
                if(strcmp("ppp",stype) == IS_OK){
                    pico_dev = (struct pico_device *) pico_ppp_create();
                }else{
                    return Py_BuildValue("i", NOT_OK);
                }
            }else{
                return Py_BuildValue("i", NOT_OK);
            }
        }
    }
    if (pico_dev == NULL){
        return Py_BuildValue("i", ERROR);
    }
    deviceAdd(pico_dev);
    deviceCount++;
    return Py_BuildValue("s", pico_dev->name);

}
static PyObject* stackTick(PyObject* self,PyObject *args){
    pico_stack_tick();
    return Py_BuildValue("i", IS_OK);
}
static PyObject* idle(PyObject* self,PyObject *args){
    PICO_IDLE();
    return Py_BuildValue("i", IS_OK);
}
static PyObject* createMreq(PyObject* self,PyObject *args){
    struct pico_ip_mreq *newmreq;
    char *sname;
    char *stype;
    char *saddrdst;
    char *saddrlink;
    if (!PyArg_ParseTuple(args, "ssss",&sname,&saddrdst,&saddrlink,&stype)){
           return Py_BuildValue("i", NOT_OK);
        }
    if (strcmp(stype,"ipv6")){
        struct pico_ip6 addrdst6;
        struct pico_ip6 addrlink6;
        pico_string_to_ipv6(saddrdst,addrdst6.addr);
        pico_string_to_ipv6(saddrlink,addrlink6.addr);
        newmreq->mcast_group_addr.ip6 = addrdst6;
        newmreq->mcast_link_addr.ip6 = addrlink6;
    }else{
        if (strcmp(stype,"ipv4")){
            struct pico_ip4 addrdst4;
            struct pico_ip4 addrlink4;
            pico_string_to_ipv4(saddrdst,&addrdst4.addr);
            pico_string_to_ipv4(saddrlink,&addrlink4.addr);
            newmreq->mcast_group_addr.ip4 = addrdst4;
            newmreq->mcast_link_addr.ip4 = addrlink4;
        }else{
            return Py_BuildValue("i", NOT_OK);
        }
    }
    mreqAdd(newmreq,sname);
    return Py_BuildValue("i", IS_OK);
}
static PyObject* createMreqSource(PyObject* self,PyObject *args){
    struct pico_ip_mreq_source *newmreqsrc;
    char *sname;
    char *stype;
    char *saddrdst;
    char *saddrlink;
    char *saddrsource;
    if (!PyArg_ParseTuple(args, "sssss",&sname,&saddrdst,&saddrsource,&saddrlink,&stype)){
           return Py_BuildValue("i", NOT_OK);
        }
    if (strcmp(stype,"ipv6")){
        struct pico_ip6 addrdst6;
        struct pico_ip6 addrlink6;
        struct pico_ip6 addrsource6;
        pico_string_to_ipv6(saddrdst,addrdst6.addr);
        pico_string_to_ipv6(saddrlink,addrlink6.addr);
        pico_string_to_ipv6(saddrsource,addrsource6.addr);
        newmreqsrc->mcast_group_addr.ip6 = addrdst6;
        newmreqsrc->mcast_link_addr.ip6 = addrlink6;
        newmreqsrc->mcast_source_addr.ip6 = addrsource6;
    }else{
        if (strcmp(stype,"ipv4")){
            struct pico_ip4 addrdst4;
            struct pico_ip4 addrlink4;
            struct pico_ip4 addrsource4;
            pico_string_to_ipv4(saddrdst,&addrdst4.addr);
            pico_string_to_ipv4(saddrlink,&addrlink4.addr);
            pico_string_to_ipv4(saddrsource,&addrsource4.addr);
            newmreqsrc->mcast_group_addr.ip4 = addrdst4;
            newmreqsrc->mcast_link_addr.ip4 = addrlink4;
            newmreqsrc->mcast_source_addr.ip4 = addrsource4;
        }else{
            return Py_BuildValue("i", NOT_OK);
        }
    }
    mreqsrcAdd(newmreqsrc,sname);
    return Py_BuildValue("i", IS_OK);
}
static PyObject* createKvVector(PyObject* self,PyObject *args){
    PICO_DNS_SD_KV_VECTOR_DECLARE(*tempvector);
    char *sname;
    if (!PyArg_ParseTuple(args, "s",&sname)){
           return Py_BuildValue("i", PARSE_FAIL);
        }
    kvvAdd(tempvector,sname);
    kvvFind(sname);
    if (pos_kvv == NULL){
        dbg("pos_kvv fail\n");
        return Py_BuildValue("i", NOT_OK);
    }

    return Py_BuildValue("s", pos_kvv->name);
}
static PyObject* deleteKvVector(PyObject* self,PyObject *args){
    char *sname;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&sname)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = kvvDelete(sname);
    return Py_BuildValue("i", ret);
}
static PyObject* createRTree(PyObject* self,PyObject *args){
    PICO_MDNS_RTREE_DECLARE(*newrtree);
    char *sname;
    if (!PyArg_ParseTuple(args, "s",&sname)){
           return Py_BuildValue("i", PARSE_FAIL);
        }
    rtreeAdd(newrtree,sname);
    rtreeFind(sname);
    if (pos_rtree == NULL){
        dbg("pos_kvv fail\n");
        return Py_BuildValue("i", NOT_OK);
    }

    return Py_BuildValue("s", pos_rtree->name);
}
static PyObject* deleteRTree(PyObject* self,PyObject *args){
    char *sname;
    int ret = 0;
    if (!PyArg_ParseTuple(args, "s",&sname)){
        return Py_BuildValue("i", PARSE_FAIL);
    }
    ret = rtreeDelete(sname);
    return Py_BuildValue("i", ret);
}
static PyObject* getPicoError(PyObject* self,PyObject *args){
    char serror[20];
    switch(pico_err){
        case PICO_ERR_NOERR:
            strcpy(serror,"NOERR");
            break;
        case PICO_ERR_EPERM:
            strcpy(serror,"EPERM");
            break;
        case PICO_ERR_ENOENT:
            strcpy(serror,"ENOENT");
            break;
        case PICO_ERR_EINTR:
            strcpy(serror,"EINTR");
            break;
        case PICO_ERR_EIO:
            strcpy(serror,"EIO");
            break;
        case PICO_ERR_ENXIO:
            strcpy(serror,"ENXIO");
            break;
        case PICO_ERR_EAGAIN:
            strcpy(serror,"EAGAIN");
            break;
        case PICO_ERR_ENOMEM:
            strcpy(serror,"ENOMEM");
            break;
        case PICO_ERR_EACCESS:
            strcpy(serror,"EACCESS");
            break;
        case PICO_ERR_EFAULT:
            strcpy(serror,"EFAULT");
            break;
        case PICO_ERR_EBUSY:
            strcpy(serror,"EBUSY");
            break;
        case PICO_ERR_EEXIST:
            strcpy(serror,"EEXIS");
            break;
        case PICO_ERR_EINVAL:
            strcpy(serror,"EINVAL");
            break;
        case PICO_ERR_ENONET:
            strcpy(serror,"ENONET");
            break;
        case PICO_ERR_EPROTO:
            strcpy(serror,"EPROTO");
            break;
        case PICO_ERR_ENOPROTOOPT:
            strcpy(serror,"ENOPROTOOPT");
            break;
        case PICO_ERR_EPROTONOSUPPORT:
            strcpy(serror,"EPROTONOSUPPORT");
            break;
        case PICO_ERR_EOPNOTSUPP:
            strcpy(serror,"EOPNOTSUPP");
            break;
        case PICO_ERR_EADDRINUSE:
            strcpy(serror,"EADDRINUSE");
            break;
        case PICO_ERR_EADDRNOTAVAIL:
            strcpy(serror,"EADDRNOTAVAIL");
            break;
        case PICO_ERR_ENETDOWN:
            strcpy(serror,"ENETDOWN");
            break;
        case PICO_ERR_ENETUNREACH:
            strcpy(serror,"ENETUNREACH");
            break;
        case PICO_ERR_ECONNRESET:
            strcpy(serror,"ECONNRESET");
            break;
        case PICO_ERR_EISCONN:
            strcpy(serror,"EISCONN");
            break;
        case PICO_ERR_ENOTCONN:
            strcpy(serror,"ENOTCONN");
            break;
        case PICO_ERR_ESHUTDOWN:
            strcpy(serror,"ESHUTDOWN");
            break;
        case PICO_ERR_ETIMEDOUT:
            strcpy(serror,"ETIMEDOUT");
            break;
        case PICO_ERR_ECONNREFUSED:
            strcpy(serror,"ECONNREFUSED");
            break;
        case PICO_ERR_EHOSTDOWN:
            strcpy(serror,"EHOSTDOWN:");
            break;
        case PICO_ERR_EHOSTUNREACH:
            strcpy(serror,"EHOSTUNREACH");
            break;
    }
    return Py_BuildValue("s", serror);
}
////////////////////////////// TEST //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PyObject* testDevice(PyObject* self,PyObject *args){
    char buffer[100]="";
    dlist * current = head_dev;
    int i = 0;
    while (current != NULL){
        strcat(buffer,",");
        strcat(buffer,current->device->name);
        current = current->next;
    }
    return Py_BuildValue("s", buffer);
}

///////////////////////////// INIT ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static char pycotcp_docs[] ="Documentazione FALSA";
static PyMethodDef pycotcp_funcs[] = {
    //{"testDevice", (PyCFunction)testDevice, METH_NOARGS},

    {"init", (PyCFunction)init, METH_NOARGS, pycotcp_docs},
    {"fede_init", (PyCFunction)fede_init, METH_NOARGS, pycotcp_docs},
    {"deleteLink4", (PyCFunction)deleteLink4, METH_VARARGS, pycotcp_docs},
    {"deleteLink6", (PyCFunction)deleteLink6, METH_VARARGS, pycotcp_docs},
    {"deleteSocketBox", (PyCFunction)deleteSocketBox, METH_VARARGS, pycotcp_docs},
    {"createDevice", (PyCFunction)createDevice, METH_VARARGS, pycotcp_docs},
    {"deleteDevice", (PyCFunction)deleteDevice, METH_VARARGS, pycotcp_docs},
    {"createMreq", (PyCFunction)createMreq, METH_VARARGS, pycotcp_docs},
    {"deleteMreq", (PyCFunction)deleteMreq, METH_VARARGS, pycotcp_docs},
    {"createMreqSource", (PyCFunction)createMreqSource, METH_VARARGS, pycotcp_docs},
    {"deleteMreqSource", (PyCFunction)deleteMreqSource, METH_VARARGS, pycotcp_docs},
    {"createKvVector", (PyCFunction)createKvVector, METH_VARARGS, pycotcp_docs},
    {"deleteKvVector", (PyCFunction)deleteKvVector, METH_VARARGS, pycotcp_docs},
    {"createRTree", (PyCFunction)createRTree, METH_VARARGS, pycotcp_docs},
    {"deleteRTree", (PyCFunction)deleteRTree, METH_VARARGS, pycotcp_docs},
    {"stackTick", (PyCFunction)stackTick, METH_NOARGS, pycotcp_docs},
    {"idle", (PyCFunction)idle, METH_NOARGS, pycotcp_docs},
    {"getPicoError", (PyCFunction)getPicoError, METH_NOARGS, pycotcp_docs},
    //ipv4
    {"isNetmaskIp4", (PyCFunction)isNetmaskIp4, METH_VARARGS, pycotcp_docs},
    {"isUnicastIp4", (PyCFunction)isUnicastIp4, METH_VARARGS, pycotcp_docs},
    {"findSourceIp4", (PyCFunction)findSourceIp4, METH_VARARGS, pycotcp_docs},
    {"natEnableIp4", (PyCFunction)natEnableIp4, METH_VARARGS, pycotcp_docs},
    {"natDisableIp4", (PyCFunction)natDisableIp4, METH_NOARGS, pycotcp_docs},
    {"linkFindIp4", (PyCFunction)linkFindIp4, METH_VARARGS, pycotcp_docs},
    {"linkAddIp4", (PyCFunction)linkAddIp4, METH_VARARGS, pycotcp_docs},
    {"linkGetIp4", (PyCFunction)linkGetIp4, METH_VARARGS, pycotcp_docs},
    {"linkDelIp4", (PyCFunction)linkDelIp4, METH_VARARGS, pycotcp_docs},
    {"routeAddIp4", (PyCFunction)routeAddIp4, METH_VARARGS, pycotcp_docs},
    {"routeDelIp4", (PyCFunction)routeDelIp4, METH_VARARGS, pycotcp_docs},
    {"portForwardIp4", (PyCFunction)portForwardIp4, METH_VARARGS, pycotcp_docs},
    {"routeGetGatewayIp4", (PyCFunction)routeGetGatewayIp4, METH_VARARGS, pycotcp_docs},
    {"pingStartIp4", (PyCFunction)pingStartIp4, METH_VARARGS, pycotcp_docs},
    {"pingAbortIp4", (PyCFunction)pingAbortIp4, METH_VARARGS, pycotcp_docs},
    //ipv6
    {"isMulticastIp6", (PyCFunction)isMulticastIp6, METH_VARARGS, pycotcp_docs},
    {"isUnicastIp6", (PyCFunction)isUnicastIp6, METH_VARARGS, pycotcp_docs},
    {"isGlobalIp6", (PyCFunction)isGlobalIp6, METH_VARARGS, pycotcp_docs},
    {"isUniqueLocalIp6", (PyCFunction)isUniqueLocalIp6, METH_VARARGS, pycotcp_docs},
    {"isSiteLocalIp6", (PyCFunction)isSiteLocalIp6, METH_VARARGS, pycotcp_docs},
    {"isLinkLocalIp6", (PyCFunction)isLinkLocalIp6, METH_VARARGS, pycotcp_docs},
    {"isLocalHostIp6", (PyCFunction)isLocalHostIp6, METH_VARARGS, pycotcp_docs},
    {"isUnspecifiedIp6", (PyCFunction)isUnspecifiedIp6, METH_VARARGS, pycotcp_docs},
    {"findSourceIp6", (PyCFunction)findSourceIp6, METH_VARARGS, pycotcp_docs},
    {"linkFindIp6", (PyCFunction)linkFindIp6, METH_VARARGS, pycotcp_docs},
    {"linkAddIp6", (PyCFunction)linkAddIp6, METH_VARARGS, pycotcp_docs},
    {"linkDelIp6", (PyCFunction)linkDelIp6, METH_VARARGS, pycotcp_docs},
    {"routeAddIp6", (PyCFunction)routeAddIp6, METH_VARARGS, pycotcp_docs},
    {"routeDelIp6", (PyCFunction)routeDelIp6, METH_VARARGS, pycotcp_docs},
    {"routingEnableIpv6", (PyCFunction)routingEnableIpv6, METH_VARARGS, pycotcp_docs},
    {"routingDisableIpv6", (PyCFunction)routingDisableIpv6, METH_VARARGS, pycotcp_docs},
    {"routeGetGatewayIp6", (PyCFunction)routeGetGatewayIp6, METH_VARARGS, pycotcp_docs},
    //SOCKET
    {"socketOpen", (PyCFunction)socketOpen, METH_VARARGS, pycotcp_docs},
    {"socketBind", (PyCFunction)socketBind, METH_VARARGS, pycotcp_docs},
    {"socketConnect", (PyCFunction)socketConnect, METH_VARARGS, pycotcp_docs},
    {"socketSend", (PyCFunction)socketSend, METH_VARARGS, pycotcp_docs},
    {"socketRecv", (PyCFunction)socketRecv, METH_VARARGS, pycotcp_docs},
    {"socketRecvFrom", (PyCFunction)socketRecvFrom, METH_VARARGS, pycotcp_docs},
    {"socketRecvFromExt", (PyCFunction)socketRecvFromExt, METH_VARARGS, pycotcp_docs},
    {"socketWrite", (PyCFunction)socketWrite, METH_VARARGS, pycotcp_docs},
    {"socketRead", (PyCFunction)socketRead, METH_VARARGS, pycotcp_docs},
    {"socketClose", (PyCFunction)socketClose, METH_VARARGS, pycotcp_docs},
    {"socketShutdown", (PyCFunction)socketShutdown, METH_VARARGS, pycotcp_docs},
    {"socketListen", (PyCFunction)socketListen, METH_VARARGS, pycotcp_docs},
    {"socketAccept", (PyCFunction)socketAccept, METH_VARARGS, pycotcp_docs},
    {"socketSendTo", (PyCFunction)socketSendTo, METH_VARARGS, pycotcp_docs},
    {"socketSendToExt", (PyCFunction)socketSendToExt, METH_VARARGS, pycotcp_docs},
    {"socketGetName", (PyCFunction)socketGetName, METH_VARARGS, pycotcp_docs},
    {"socketGetPeerName", (PyCFunction)socketGetPeerName, METH_VARARGS, pycotcp_docs},
    {"socketSetOption", (PyCFunction)socketSetOption, METH_VARARGS, pycotcp_docs},
    {"socketSetOptionMreq", (PyCFunction)socketSetOptionMreq, METH_VARARGS, pycotcp_docs},
    {"socketSetOptionMreqSource", (PyCFunction)socketSetOptionMreqSource, METH_VARARGS, pycotcp_docs},
    {"createMreqSource", (PyCFunction)createMreqSource, METH_VARARGS, pycotcp_docs},
    {"socketGetOption", (PyCFunction)socketGetOption, METH_VARARGS, pycotcp_docs},
    //DHCP CLIENT
    {"dhcpClientInitiate", (PyCFunction)dhcpClientInitiate, METH_VARARGS, pycotcp_docs},
    {"dhcpClientAbort", (PyCFunction)dhcpClientAbort, METH_VARARGS, pycotcp_docs},
    //DHCP SERVER
    {"dhcpServerInitiate", (PyCFunction)dhcpServerInitiate, METH_VARARGS, pycotcp_docs},
    {"dhcpServerDestroy", (PyCFunction)dhcpServerDestroy, METH_VARARGS, pycotcp_docs},
    //SNTP SERVER
    {"sntpSync", (PyCFunction)sntpSync, METH_VARARGS, pycotcp_docs},
    {"sntpGetTimeOfTheDay", (PyCFunction)sntpGetTimeOfTheDay, METH_NOARGS, pycotcp_docs},
    //slaacv4
    {"slaacv4ClaimIP", (PyCFunction)slaacv4ClaimIP, METH_VARARGS, pycotcp_docs},
    {"slaacv4UnregisterIP", (PyCFunction)slaacv4UnregisterIP, METH_NOARGS, pycotcp_docs},
    //DNS
    {"dnsNameServer", (PyCFunction)dnsNameServer, METH_VARARGS, pycotcp_docs},
    {"dnsGetAddr", (PyCFunction)dnsGetAddr, METH_VARARGS, pycotcp_docs},
    {"dnsGetName", (PyCFunction)dnsGetName, METH_VARARGS, pycotcp_docs},
    //IP filter
    {"filterIpv4Add", (PyCFunction)filterIpv4Add, METH_VARARGS, pycotcp_docs},
    {"filterIpv4Del", (PyCFunction)filterIpv4Del, METH_VARARGS, pycotcp_docs},
    //OLSR
    #ifdef PICO_SUPPORT_OLSR
    {"olsrAdd", (PyCFunction)olsrAdd, METH_VARARGS, pycotcp_docs},
    #endif
    //AODV
    {"aodvAdd", (PyCFunction)aodvAdd, METH_VARARGS, pycotcp_docs},
    //PPP
    {"pppSetSerialRead", (PyCFunction)pppSetSerialRead, METH_VARARGS, pycotcp_docs},
    {"pppSetSerialWrite", (PyCFunction)pppSetSerialWrite, METH_VARARGS, pycotcp_docs},
    {"pppSetSerialSpeed", (PyCFunction)pppSetSerialSpeed, METH_VARARGS, pycotcp_docs},
    {"pppSetAPN", (PyCFunction)pppSetAPN, METH_VARARGS, pycotcp_docs},
    {"pppSetUsername", (PyCFunction)pppSetUsername, METH_VARARGS, pycotcp_docs},
    {"pppSetPassword", (PyCFunction)pppSetPassword, METH_VARARGS, pycotcp_docs},
    {"pppConnect", (PyCFunction)pppConnect, METH_VARARGS, pycotcp_docs},
    {"pppDisconnect", (PyCFunction)pppDisconnect, METH_VARARGS, pycotcp_docs},
    //DNS SD
    {"dnssdInit", (PyCFunction)dnssdInit, METH_VARARGS, pycotcp_docs},
    {"dnssdRegisterService", (PyCFunction)dnssdRegisterService, METH_VARARGS, pycotcp_docs},
    {"dnssdKVVectorAdd", (PyCFunction)dnssdKVVectorAdd, METH_VARARGS, pycotcp_docs},
    //MDNS
    {"mdnsInit", (PyCFunction)mdnsInit, METH_VARARGS, pycotcp_docs},
    {"mdnsGetHostname", (PyCFunction)mdnsGetHostname, METH_NOARGS, pycotcp_docs},
    {"mdnsSetHostname", (PyCFunction)mdnsSetHostname, METH_VARARGS, pycotcp_docs},
    {"mdnsClaim", (PyCFunction)mdnsClaim, METH_VARARGS, pycotcp_docs},
    {"mdnsGetRecord", (PyCFunction)mdnsGetRecord, METH_VARARGS, pycotcp_docs},
    {"mdnsRecordCreate", (PyCFunction)mdnsRecordCreate, METH_VARARGS, pycotcp_docs},
    {"mdnsIsHostnameRecord", (PyCFunction)mdnsIsHostnameRecord, METH_VARARGS, pycotcp_docs},
    //TFTP
    {"tftpListen", (PyCFunction)tftpListen, METH_VARARGS, pycotcp_docs},
    {"tftpRejectRequest", (PyCFunction)tftpRejectRequest, METH_VARARGS, pycotcp_docs},
    {"tftpSessionSetup", (PyCFunction)tftpSessionSetup, METH_VARARGS, pycotcp_docs},
    {"tftpSetOption", (PyCFunction)tftpSetOption, METH_VARARGS, pycotcp_docs},
    {"tftpGetOption", (PyCFunction)tftpGetOption, METH_VARARGS, pycotcp_docs},
    {"tftpParseRequestArgs", (PyCFunction)tftpParseRequestArgs, METH_VARARGS, pycotcp_docs},
    {"tftpSend", (PyCFunction)tftpSend, METH_VARARGS, pycotcp_docs},
    {"tftpGetFileSize", (PyCFunction)tftpGetFileSize, METH_VARARGS, pycotcp_docs},
    {"tftpAbort", (PyCFunction)tftpAbort, METH_VARARGS, pycotcp_docs},
    {"tftpCloseServer", (PyCFunction)tftpCloseServer, METH_VARARGS, pycotcp_docs},
    {"tftpAppSetup", (PyCFunction)tftpAppSetup, METH_VARARGS, pycotcp_docs},
    {"tftpAppStartRx", (PyCFunction)tftpAppStartRx, METH_VARARGS, pycotcp_docs},
    {"tftpAppStartTx", (PyCFunction)tftpAppStartTx, METH_VARARGS, pycotcp_docs},
    {"tftpGet", (PyCFunction)tftpGet, METH_VARARGS, pycotcp_docs},
    {"tftpPut", (PyCFunction)tftpPut, METH_VARARGS, pycotcp_docs},
    {"tftpStartTx", (PyCFunction)tftpStartTx, METH_VARARGS, pycotcp_docs},
    {"tftpStartRx", (PyCFunction)tftpStartRx, METH_VARARGS, pycotcp_docs},
    {NULL}
};
void initpycoclib(void){
    PyObject *module;
    if (! PyEval_ThreadsInitialized()) {
        PyEval_InitThreads();
    }
    module = Py_InitModule3("pycoclib", pycotcp_funcs,
                   "pycoclib extension");
    PyModule_AddIntConstant(module, "TCP_NODELAY", PICO_TCP_NODELAY);
    PyModule_AddIntConstant(module, "IP_MULTICAST_IF", PICO_IP_MULTICAST_IF);
    PyModule_AddIntConstant(module, "IP_MULTICAST_TTL", PICO_IP_MULTICAST_IF);
    PyModule_AddIntConstant(module, "IP_MULTICAST_LOOP", PICO_IP_MULTICAST_LOOP);
    PyModule_AddIntConstant(module, "IP_ADD_MEMBERSHIP", PICO_IP_ADD_MEMBERSHIP);
    PyModule_AddIntConstant(module, "IP_DROP_MEMBERSHIP", PICO_IP_DROP_MEMBERSHIP);
    PyModule_AddIntConstant(module, "IP_ADD_SOURCE_MEMBERSHIP", PICO_IP_ADD_SOURCE_MEMBERSHIP);
    PyModule_AddIntConstant(module, "IP_DROP_SOURCE_MEMBERSHIP", PICO_IP_DROP_SOURCE_MEMBERSHIP);
    PyModule_AddIntConstant(module, "SOCKET_OPT_KEEPIDLE", PICO_SOCKET_OPT_KEEPIDLE);
    PyModule_AddIntConstant(module, "SOCKET_OPT_KEEPINTVL", PICO_SOCKET_OPT_KEEPINTVL);
    PyModule_AddIntConstant(module, "SOCKET_OPT_KEEPCNT",PICO_SOCKET_OPT_KEEPCNT );
    PyModule_AddIntConstant(module, "SOCKET_OPT_LINGER", PICO_SOCKET_OPT_LINGER);
    PyModule_AddIntConstant(module, "SOCKET_OPT_RCVBUF", PICO_SOCKET_OPT_RCVBUF);
    PyModule_AddIntConstant(module, "SOCKET_OPT_SNDBUF", PICO_SOCKET_OPT_SNDBUF);
    PyModule_AddIntConstant(module, "TFTP_NONE",PICO_TFTP_NONE);
    PyModule_AddIntConstant(module, "TFTP_RRQ",PICO_TFTP_RRQ);
    PyModule_AddIntConstant(module, "TFTP_WRQ",PICO_TFTP_WRQ);
    PyModule_AddIntConstant(module, "TFTP_DATA",PICO_TFTP_DATA);
    PyModule_AddIntConstant(module, "TFTP_ACK",PICO_TFTP_ACK);
    PyModule_AddIntConstant(module, "TFTP_ERROR",PICO_TFTP_ERROR);
    PyModule_AddIntConstant(module, "TFTP_OACK",PICO_TFTP_OACK);
    PyModule_AddIntConstant(module, "TFTP_EV_OK",PICO_TFTP_EV_OK);
    PyModule_AddIntConstant(module, "TFTP_EV_OPT",PICO_TFTP_EV_OPT);
    PyModule_AddIntConstant(module, "TFTP_EV_ERR_PEER",PICO_TFTP_EV_ERR_PEER);
    PyModule_AddIntConstant(module, "TFTP_EV_ERR_LOCAL",PICO_TFTP_EV_ERR_LOCAL);
    PyModule_AddIntConstant(module, "TFTP_PORT",short_be(PICO_TFTP_PORT));
}
