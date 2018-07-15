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
#include "pico_stack.h"
#include "pico_config.h"
#include "pico_ipv4.h"
#include "pico_icmp4.h"
#include "pico_ipv6.h"
#include "pico_icmp6.h"
#include "pico_device.h"
#include "pico_dev_vde.h"
#include "pico_socket.h"
#include "pico_nat.h"
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

#define IS_OK 0
#define NOT_OK -1
#define NOT_SUPPORTED -4
#define ERROR -3
#define PARSE_FAIL -2
#define IPV4 "ipv4"
#define IPV6 "ipv6"
#define IPDIM 50
#define NXT_MAC(x) ++x[5]
//////////////STRUCT/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct device_list{
    char name[50];
    struct pico_device *device;
    struct device_list *next;
};
struct socket_box {
    struct pico_socket *s;
    uint16_t sport;
    union pico_address dst;
    void (*callback) (uint16_t ev, struct pico_socket *s);
};
struct socketbox_list {
    char name[50];
    struct socket_box *socbox;
    struct socketbox_list *next;
};
struct link4_list {
    char name[50];
    struct pico_ipv4_link *link;
    struct link4_list *next;
};
struct link6_list {
    char name[50];
    struct pico_ipv4_link *link;
    struct link6_list *next;
};
struct mreq_list{
    char name[50];
    struct pico_ip_mreq *mreq;
    struct mreq_list *next;
};
struct mreqsource_list{
    char name[50];
    struct pico_ip_mreq_source *mreqsrc;
    struct mreqsource_list *next;
} ;
struct kv_vector_list{
    char name[50];
    struct kv_vector *vector;
    struct kv_vector_list *next;
} ;
struct tftpsession_list{
    char name[50];
    struct pico_tftp_session *session;
    struct tftpsession_list *next;
} ;
struct mdnsr_list{
    char name[50];
    struct pico_mdns_record *record;
    struct mdnsr_list *next;
} ;
struct rtree_list{
    char name[50];
    struct pico_mdns_rtree *rtree;
    struct rtree_list *next;
} ;

struct note_t {
    char *name;
    char *filename;
    int fd;
    int options;
    uint8_t opttimeoutopt;
    int32_t optfilesize;
    int32_t filesize;
};
struct note_list{
    char name[50];
    struct note_t *note;
    struct rtree_list *next;
} ;
typedef struct device_list dlist;
typedef struct socketbox_list sbList;
typedef struct link4_list l4List;
typedef struct link6_list l6List;
typedef struct mreq_list mList;
typedef struct mreqsource_list msList;
typedef struct kv_vector_list kvvList;
typedef struct tftpsession_list tftpList;
typedef struct mdnsr_list mrList;
typedef struct rtree_list rtList;
typedef struct note_list nList;

int deviceAdd(struct pico_device *newdev);
int deviceFind(char *name);
int socketBoxAdd(struct socket_box *newbox, char *name, void (*callback) (uint16_t ev, struct pico_socket *s));
sbList* socketBoxFind(char *name);
int socketFindAddress(struct pico_socket *s, char *name);
int link4Add(struct pico_ipv4_link *newlink, char *name);
int link4Find(char *name);
int link6Add(struct pico_ipv6_link *newlink, char *name);
int link6Find(char *name);
int mreqAdd(struct pico_ip_mreq *newmreq, char *name);
int mreqFind(char *name);
int link6Delete(char *name);
