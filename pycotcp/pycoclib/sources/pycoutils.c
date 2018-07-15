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
//
#include <fcntl.h>
#include <ctype.h>
//
#include <sys/types.h>
#include <sys/stat.h>
//
////////////////////////////DEVICE LIST//////////////////////////////////////////////////////////////////////////////////////////////////////////////
dlist *head_dev = NULL;
dlist *tail_dev = NULL;
dlist *pos_dev = NULL;
int deviceAdd(struct pico_device *newdev){
    if (head_dev == NULL){
        head_dev = malloc(sizeof(dlist));
        if (head_dev == NULL) {
            return -1;
        }
        strcpy(head_dev->name,newdev->name);
        head_dev->device = newdev;
        head_dev->next = NULL;
        tail_dev = head_dev;
    }else{
        tail_dev->next = malloc(sizeof(dlist));
        tail_dev = tail_dev->next;
        if (tail_dev == NULL) {
            return -1;
        }
        strcpy(tail_dev->name,newdev->name);
        tail_dev->device = newdev;
        tail_dev->next = NULL;
    }
    return IS_OK;
}
int deviceFind(char *name){
    dlist *current = head_dev;
    pos_dev = NULL;
    while (current != NULL){
        if (strcmp(name,current->name) == IS_OK){
            pos_dev = current;
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
int deviceDelete(char *name){
    dlist *current = head_dev;

    if (strcmp(name,current->name) == IS_OK){
        head_dev = current->next;
        free(current);
        return IS_OK;
    }
    while (current->next != NULL){
        if (strcmp(name,current->next->name) == IS_OK){
            dlist *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
////////////////////////////SOCKET LIST////////////////////////////////////////////////////////////////////////////////////////////////////////
sbList *head_sock = NULL;
sbList *tail_sock = NULL;

int socketBoxAdd(struct socket_box *newbox, char *name, void (*callback) (uint16_t ev, struct pico_socket *s)){
    printf("Adding new socket %lu\n", newbox->s);
    if (head_sock == NULL){
        head_sock = malloc(sizeof(sbList));
        if (head_sock == NULL) {
            return -1;
        }
        strcpy(head_sock->name,name);
        head_sock->socbox = newbox;
        head_sock->socbox->callback = callback;
        head_sock->next = NULL;
        tail_sock = head_sock;
    }else{
        tail_sock->next = malloc(sizeof(sbList));
        tail_sock = tail_sock->next;
        if (tail_sock == NULL) {
            return -1;
        }
        strcpy(tail_sock->name,name);
        tail_sock->socbox = newbox;
        tail_sock->socbox->callback = callback;
        tail_sock->next = NULL;
    }
    checkSockets();
    return 0;
}

void checkSockets()
{
    printf("socket list has:\n");
    sbList *current = head_sock;
    while(current != NULL)
    {
        printf("\t%s\n", current->name);
        current = current->next;
    }
}

sbList* socketBoxFind(char *name){
    sbList *current = head_sock;
    while (current != NULL)
    {
        if (strcmp(name, current->name) == IS_OK)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int socketFindAddress(struct pico_socket *s, char *name){
    sbList *current = head_sock;
    while (current != NULL)
    {
        if (s == current->socbox->s)
        {
            strcpy(name, current->name);
            return IS_OK;
        }
        current = current->next;
    }
    return NOT_OK;
}

int socketBoxDelete(char *name){
    sbList *current = head_sock;

    if (strcmp(name, current->name) == 0)
    {
        head_sock = current->next;
        free(current);
        checkSockets();
        return IS_OK;
    }

    while (current->next != NULL)
    {
        if (strcmp(name,current->next->name) == 0)
        {
            if(tail_sock == current->next)
            {
                tail_sock = current;
            }

            sbList *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            checkSockets();
            return IS_OK;
        }
        current = current->next;
    }
    checkSockets();
    return -1;
}
///////////////////////////LINK IPV4 LIST///////////////////////////////////////////////////////////////////////////////////////////////////////////
l4List *head_link4 = NULL;
l4List *tail_link4 = NULL;
l4List *pos_link4 = NULL;
int link4Add(struct pico_ipv4_link *newlink, char *name){
    if (head_link4 == NULL){
        head_link4 = malloc(sizeof(l4List));
        if (head_link4 == NULL) {
            return -1;
        }
        strcpy(head_link4->name,name);
        head_link4->link = newlink;
        head_link4->next = NULL;
        tail_link4 = head_link4;
    }else{
        tail_link4->next = malloc(sizeof(l4List));
        tail_link4 = tail_link4->next;
        if (tail_link4 == NULL) {
            return -1;
        }
        strcpy(tail_link4->name,name);
        tail_link4->link = newlink;
        tail_link4->next = NULL;
    }
    return 0;
}
int link4Find(char *name){
    l4List *current = head_link4;
    pos_link4 = NULL;
    while (current != NULL){
        if (strcmp(name,current->name) == IS_OK){
            pos_link4 = current;
            return 1;
        }
        current = current->next;
    }
    return 0;
}
int link4Delete(char *name){
    l4List *current = head_link4;

    if (strcmp(name,current->name) == IS_OK){
        head_link4 = current->next;
        free(current);
        return IS_OK;
    }
    while (current->next != NULL){
        if (strcmp(name,current->next->name) == IS_OK){
            l4List *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
///////////////////////////LINK IPV6 LIST///////////////////////////////////////////////////////////////////////////////////////////////////////////
l6List *head_link6 = NULL;
l6List *tail_link6 = NULL;
l6List *pos_link6 = NULL;
int link6Add(struct pico_ipv6_link *newlink, char *name){
    if (head_link6 == NULL){
        head_link6 = malloc(sizeof(l6List));
        if (head_link6 == NULL) {
            return -1;
        }
        strcpy(head_link6->name,name);
        head_link6->link = newlink;
        head_link6->next = NULL;
        tail_link6 = head_link6;
    }else{
        tail_link6->next = malloc(sizeof(l6List));
        tail_link6 = tail_link6->next;
        if (tail_link6 == NULL) {
            return -1;
        }
        strcpy(tail_link6->name,name);
        tail_link6->link = newlink;
        tail_link6->next = NULL;
    }
    return 0;
}
int link6Find(char *name){
    l6List *current = head_link6;
    pos_link6 = NULL;
    while (current != NULL){
        if (strcmp(name,current->name) == IS_OK){
            pos_link6 = current;
            return 1;
        }
        current = current->next;
    }
    return 0;
}
int link6Delete(char *name){
        l6List *current = head_link6;

    if (strcmp(name,current->name) == IS_OK){
        head_link6 = current->next;
        free(current);
        return IS_OK;
    }
    while (current->next != NULL){
        if (strcmp(name,current->next->name) == IS_OK){
            l6List *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
///////////////////////////LINK MREQ LIST///////////////////////////////////////////////////////////////////////////////////////////////////////////
mList *head_mreq = NULL;
mList *tail_mreq = NULL;
mList *pos_mreq = NULL;
int mreqAdd(struct pico_ip_mreq *newmreq, char *name){
    if (head_mreq == NULL){
        head_mreq = malloc(sizeof(mList));
        if (head_mreq == NULL) {
            return -1;
        }
        strcpy(head_mreq->name,name);
        head_mreq->mreq = newmreq;
        head_mreq->next = NULL;
        tail_mreq = head_mreq;
    }else{
        tail_mreq->next = malloc(sizeof(mList));
        tail_mreq = tail_mreq->next;
        if (tail_mreq == NULL) {
            return -1;
        }
        strcpy(tail_mreq->name,name);
        tail_mreq->mreq = newmreq;
        tail_mreq->next = NULL;
    }
    return 0;
}
int mreqFind(char *name){
    mList *current = head_mreq;
    pos_mreq = NULL;
    while (current != NULL){
        if (strcmp(name,current->name) == IS_OK){
            pos_mreq = current;
            return 1;
        }
        current = current->next;
    }
    return 0;
}
int mreqDelete(char *name){
    mList *current = head_mreq;

    if (strcmp(name,current->name) == IS_OK){
        head_mreq = current->next;
        free(current);
        return IS_OK;
    }
    while (current->next != NULL){
        if (strcmp(name,current->next->name) == IS_OK){
            mList *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
///////////////////////////LINK MREQ LIST///////////////////////////////////////////////////////////////////////////////////////////////////////////
msList *head_mreqsrc = NULL;
msList *tail_mreqsrc = NULL;
msList *pos_mreqsrc = NULL;
int mreqsrcAdd(struct pico_ip_mreq_source *newmreqsrc, char *name){
    if (head_mreqsrc == NULL){
        head_mreqsrc = malloc(sizeof(msList));
        if (head_mreqsrc == NULL) {
            return -1;
        }
        strcpy(head_mreqsrc->name,name);
        head_mreqsrc->mreqsrc = newmreqsrc;
        head_mreqsrc->next = NULL;
        tail_mreq = head_mreqsrc;
    }else{
        tail_mreqsrc->next = malloc(sizeof(msList));
        tail_mreqsrc = tail_mreqsrc->next;
        if (tail_mreqsrc == NULL) {
            return -1;
        }
        strcpy(tail_mreqsrc->name,name);
        tail_mreqsrc->mreqsrc = newmreqsrc;
        tail_mreqsrc->next = NULL;
    }
    return 0;
}
int mreqsrcFind(char *name){
    msList *current = head_mreqsrc;
    pos_mreqsrc = NULL;
    while (current != NULL){
        if (strcmp(name,current->name) == IS_OK){
            pos_mreqsrc = current;
            return 1;
        }
        current = current->next;
    }
    return 0;
}
int mreqsrcDelete(char *name){
    msList *current = head_mreqsrc;

    if (strcmp(name,current->name) == IS_OK){
        head_mreqsrc = current->next;
        free(current);
        return IS_OK;
    }
    while (current->next != NULL){
        if (strcmp(name,current->next->name) == IS_OK){
            msList *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
///////////////////////////KV VECTOR LIST///////////////////////////////////////////////////////////////////////////////////////////////////////////
kvvList *head_kvv = NULL;
kvvList *tail_kvv = NULL;
kvvList *pos_kvv = NULL;
int kvvAdd(struct kv_vector *newvector, char *name){
    if (head_kvv == NULL){
        head_kvv = malloc(sizeof(kvvList));
        if (head_kvv == NULL) {
            return -1;
        }
        strcpy(head_kvv->name,name);
        head_kvv->vector = newvector;
        head_kvv->next = NULL;
        tail_kvv = head_kvv;
    }else{
        tail_kvv->next = malloc(sizeof(kvvList));
        tail_kvv = tail_kvv->next;
        if (tail_kvv == NULL) {
            return -1;
        }
        strcpy(tail_kvv->name,name);
        tail_kvv->vector = newvector;
        tail_kvv->next = NULL;
    }
    return 0;
}
int kvvFind(char *name){
    kvvList *current = head_kvv;
    pos_kvv = NULL;
    while (current != NULL){
        if (strcmp(name,current->name) == IS_OK){
            pos_kvv = current;
            return 1;
        }
        current = current->next;
    }
    return 0;
}
int kvvDelete(char *name){
    kvvList *current = head_kvv;

    if (strcmp(name,current->name) == IS_OK){
        head_kvv = current->next;
        free(current);
        return IS_OK;
    }
    while (current->next != NULL){
        if (strcmp(name,current->next->name) == IS_OK){
            kvvList *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
///////////////////////////TFTP SESSION LIST///////////////////////////////////////////////////////////////////////////////////////////////////////////
tftpList *head_tftps = NULL;
tftpList *tail_tftps = NULL;
tftpList *pos_tftps = NULL;
int tftpsAdd(struct pico_tftp_session *newsession, char *name){
    if (head_tftps == NULL){
        head_tftps = malloc(sizeof(tftpList));
        if (head_tftps == NULL) {
            return -1;
        }
        strcpy(head_tftps->name,name);
        head_tftps->session = newsession;
        head_tftps->next = NULL;
        tail_tftps = head_tftps;
    }else{
        tail_tftps->next = malloc(sizeof(tftpList));
        tail_tftps = tail_tftps->next;
        if (tail_tftps == NULL) {
            return -1;
        }
        strcpy(tail_tftps->name,name);
        tail_tftps->session = newsession;
        tail_tftps->next = NULL;
    }
    return 0;
}
int tftpsFind(char *name){
    tftpList *current = head_tftps;
    pos_tftps = NULL;
    int i = 1;
    while (current != NULL){
        if (strcmp(name,current->name) == IS_OK){
            pos_tftps = current;
            return 1;
        }
        current = current->next;
        i++;
    }
    return 0;
}
int tftpsFindAddress(struct pico_tftp_session *s, char *name){
    tftpList *current = head_tftps;
    while (current != NULL){
        if ( s == current->session){
            strcpy(name,current->name);
            return IS_OK;
        }
        current = current->next;
    }
    return NOT_OK;
}
int tftpsDelete(char *name){
    tftpList *current = head_tftps;

    if (strcmp(name,current->name) == IS_OK){
        head_kvv = current->next;
        free(current);
        return IS_OK;
    }
    while (current->next != NULL){
        if (strcmp(name,current->next->name) == IS_OK){
            tftpList *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
///////////////////////////MDNS RECORD LIST///////////////////////////////////////////////////////////////////////////////////////////////////////////
mrList *head_mdnsr = NULL;
mrList *tail_mdnsr = NULL;
mrList *pos_mdnsr = NULL;
int mdnsrAdd(struct pico_mdns_record *newrecord, char *name){
    if (head_mdnsr == NULL){
        head_mdnsr = malloc(sizeof(mrList));
        if (head_mdnsr == NULL) {
            return -1;
        }
        strcpy(head_mdnsr->name,name);
        head_mdnsr->record = newrecord;
        head_mdnsr->next = NULL;
        tail_mdnsr = head_mdnsr;
    }else{
        tail_mdnsr->next = malloc(sizeof(mrList));
        tail_mdnsr = tail_kvv->next;
        if (tail_kvv == NULL) {
            return -1;
        }
        strcpy(tail_mdnsr->name,name);
        tail_mdnsr->record = newrecord;
        tail_mdnsr->next = NULL;
    }
    return 0;
}
int mdnsrFind(char *name){
    mrList *current = head_mdnsr;
    pos_mdnsr = NULL;
    while (current != NULL){
        if (strcmp(name,current->name) == IS_OK){
            pos_mdnsr = current;
            return 1;
        }
        current = current->next;
    }
    return 0;
}
int mdnsrDelete(char *name){
    mrList *current = head_mdnsr;

    if (strcmp(name,current->name) == IS_OK){
        head_mdnsr = current->next;
        free(current);
        return IS_OK;
    }
    while (current->next != NULL){
        if (strcmp(name,current->next->name) == IS_OK){
            mrList *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
///////////////////////////RTREE LIST///////////////////////////////////////////////////////////////////////////////////////////////////////////
rtList *head_rtree = NULL;
rtList *tail_rtree = NULL;
rtList *pos_rtree = NULL;
int rtreeAdd(struct pico_mdns_rtree *newrtree, char *name){
    if (head_rtree == NULL){
        head_rtree = malloc(sizeof(rtList));
        if (head_rtree == NULL) {
            return -1;
        }
        strcpy(head_rtree->name,name);
        head_rtree->rtree = newrtree;
        head_rtree->next = NULL;
        tail_rtree = head_rtree;
    }else{
        tail_rtree->next = malloc(sizeof(rtList));
        tail_rtree = tail_rtree->next;
        if (tail_rtree == NULL) {
            return -1;
        }
        strcpy(tail_rtree->name,name);
        tail_rtree->rtree = newrtree;
        tail_rtree->next = NULL;
    }
    return 0;
}
int rtreeFind(char *name){
    rtList *current = head_rtree;
    pos_rtree = NULL;
    while (current != NULL){
        if (strcmp(name,current->name) == IS_OK){
            pos_rtree = current;
            return 1;
        }
        current = current->next;
    }
    return 0;
}
int rtreeDelete(char *name){
    rtList *current = head_rtree;

    if (strcmp(name,current->name) == IS_OK){
        head_rtree = current->next;
        free(current);
        return IS_OK;
    }
    while (current->next != NULL){
        if (strcmp(name,current->next->name) == IS_OK){
            rtList *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
int rtreeFindAddress(struct pico_mdns_rtree *tree, char *name){
    rtList *current = head_rtree;
    while (current != NULL){
        if ( tree == current->rtree){
            strcpy(name,current->name);
            return IS_OK;
        }
        current = current->next;
    }
    return NOT_OK;
}
///////////////////////////NOTE LIST///////////////////////////////////////////////////////////////////////////////////////////////////////////
nList *head_note = NULL;
nList *tail_note = NULL;
nList *pos_note = NULL;
int noteAdd(struct note_t *newnote, char *name){
    if (head_note == NULL){
        head_note = malloc(sizeof(nList));
        if (head_note == NULL) {
            return -1;
        }
        strcpy(head_note->name,name);
        head_note->note = newnote;
        head_note->next = NULL;
        tail_note = head_note;
    }else{
        tail_note->next = malloc(sizeof(nList));
        tail_note = tail_note->next;
        if (tail_note == NULL) {
            return -1;
        }
        strcpy(tail_note->name,name);
        tail_note->note = newnote;
        tail_note->next = NULL;
    }
    return 0;
}
int noteFind(char *name){
    nList *current = head_note;
    pos_note = NULL;
    while (current != NULL){
        if (strcmp(name,current->name) == IS_OK){
            pos_note = current;
            return 1;
        }
        current = current->next;
    }
    return 0;
}
int noteDelete(char *name){
    nList *current = head_note;

    if (strcmp(name,current->name) == IS_OK){
        head_note = current->next;
        free(current);
        return IS_OK;
    }
    while (current->next != NULL){
        if (strcmp(name,current->next->name) == IS_OK){
            nList *tmp = current->next;
            current->next = tmp->next;
            free(tmp);
            return IS_OK;
        }
        current = current->next;
    }
    return -1;
}
int noteFindAddress(struct note_t *n, char *name){
    nList *current = head_note;
    while (current != NULL){
        if ( n == current->note){
            strcpy(name,current->name);
            return IS_OK;
        }
        current = current->next;
    }
    return NOT_OK;
}
////////////////////////////////////////////////////////////////////////
struct note_t *add_note(const char *filename, int fd, char direction)
{
    struct note_t *note = PICO_ZALLOC(sizeof(struct note_t));
    note->filename = strdup(filename);
    note->fd = fd;
    note->filesize = 0;
    return note;
}
struct pico_tftp_session *make_session_or_die(union pico_address *addr, uint16_t family)
{
    char peer[50];
    struct pico_tftp_session *session;
    pico_ipv4_to_string(&peer,addr->ip4.addr);
    session = pico_tftp_session_setup(addr, family);
    if (!session) {
        fprintf(stderr, "TFTP: Error in session setup\n");
        exit(3);
    }

    return session;
}
struct note_t *setup_transfer(char operation, const char *filename)
{
    int fd;
    fd = open(filename, (toupper(operation) == 'T') ? O_RDONLY : O_WRONLY | O_EXCL | O_CREAT, 0666);
    if (fd < 0) {
        perror("open");
        fprintf(stderr, "Unable to handle file %s\n", filename);
        return NULL;
    }
    return add_note(filename, fd, operation);
}
struct note_t *transfer_prepare(struct pico_tftp_session **psession, char operation, const char *filename, union pico_address *addr, uint16_t family)
{
    struct note_t *note;

    note = setup_transfer(operation, filename);
    *psession = make_session_or_die(addr, family);
    return note;
}
