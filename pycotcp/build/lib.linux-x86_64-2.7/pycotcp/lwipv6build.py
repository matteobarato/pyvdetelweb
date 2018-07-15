#!/usr/bin/env python

from cffi import FFI
ffi = FFI()

ffi.set_source("_pycolwipv6",
        """
            #include <stdio.h>
            //#include <stdlib.h>
            //#include <unistd.h>
            //#include <dlfcn.h>

            //#include <sys/stat.h>

            //#include <sys/un.h>
            #include <sys/types.h>
            #include <sys/socket.h>

            #include <sys/select.h>
            #include <sys/poll.h>

            //#include <poll.h>
            #include <arpa/inet.h>
            #include <netinet/in.h>
            #include <linux/in6.h>

            #include <lwipv6.h>

            static unsigned int ip4_addrx(struct ip_addr* addr, unsigned int a, unsigned int b, unsigned int c, unsigned int d)
            {
                return IP4_ADDRX(addr, a, b, c, d);
            }

            static unsigned int ip64_prefix()
            {
                return IP64_PREFIX;
            }

            static struct ip_addr ip64_addr(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
            {
                struct ip_addr addr;
                IP64_ADDR(&addr, a, b, c, d);
                return addr;
            }

            static struct ip_addr ip64_maskaddr(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
            {
                struct ip_addr mask;
                IP64_MASKADDR(&mask, a, b, c, d);
                return mask;
            }

            static void check_sockaddr(struct sockaddr_in* sockaddr)
            {
                printf("Checking sockaddr %d ", sockaddr);
                printf("l %d l", sockaddr->sin_family);
                printf("l %d l", sockaddr->sin_port);
                char string[INET_ADDRSTRLEN];
                inet_ntop(sockaddr->sin_family, &sockaddr->sin_addr, string, INET_ADDRSTRLEN),
                printf("l %s l", string);
                fflush(stdout);
            }

            static int fetch_port(struct sockaddr *from, int family)
            {
                if(family == AF_INET6)
                {
                    return ((struct sockaddr_in6*) from)->sin6_port;
                }
                else
                {
                    return ((struct sockaddr_in*) from)->sin_port;
                }
            }

            static struct sockaddr_in* create_empty_sockaddr()
            {
                struct sockaddr_in* sockaddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
                return sockaddr;
            }

            static struct sockaddr* create_server_sockaddr(int family, int port)
            {
                struct sockaddr* sockaddr = NULL;

                if(family == AF_INET6)
                {
                    sockaddr = (struct sockaddr*) malloc(sizeof(struct sockaddr_in6));
                    ((struct sockaddr_in6*) sockaddr)->sin6_family = family;
                    ((struct sockaddr_in6*) sockaddr)->sin6_port = htons(port);
                    ((struct sockaddr_in6*) sockaddr)->sin6_addr = in6addr_any;
                }
                else
                {
                    sockaddr = (struct sockaddr*) malloc(sizeof(struct sockaddr_in));
                    ((struct sockaddr_in*) sockaddr)->sin_family = family;
                    ((struct sockaddr_in*) sockaddr)->sin_port = htons(port);
                    ((struct sockaddr_in*) sockaddr)->sin_addr.s_addr = INADDR_ANY;
                }

                return sockaddr;
            }

            static struct sockaddr* create_sockaddr(int family, int port, char* addr)
            {
                struct sockaddr* sockaddr = NULL;
                if(family == AF_INET6)
                {
                    sockaddr = (struct sockaddr*) malloc(sizeof(struct sockaddr_in6));
                    ((struct sockaddr_in6*) sockaddr)->sin6_family = family;
                    ((struct sockaddr_in6*) sockaddr)->sin6_port = htons(port);
                    inet_pton(family, addr, &(((struct sockaddr_in6*) sockaddr)->sin6_addr));
                }
                else
                {
                    sockaddr = (struct sockaddr*) malloc(sizeof(struct sockaddr_in));
                    ((struct sockaddr_in*) sockaddr)->sin_family = family;
                    ((struct sockaddr_in*) sockaddr)->sin_port = htons(port);
                    inet_pton(family, addr, &(((struct sockaddr_in*) sockaddr)->sin_addr));
                }

                return sockaddr;
            }

            int sockaddr_size()
            {
                return sizeof(struct sockaddr_in);
            }

            static void lwip_add_addr6(struct netif* netif, uint32_t ipaddr[4], unsigned int ipmask[4])
            {
		struct ip_addr* addr = (struct ip_addr*) malloc(sizeof(struct ip_addr));
		struct ip_addr* mask = (struct ip_addr*) malloc(sizeof(struct ip_addr));

		/* set the local IP address of the interface */
                IP6_ADDR(addr, ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3], ipaddr[4], ipaddr[5], ipaddr[6], ipaddr[7]);
                IP6_ADDR(mask, ipmask[0], ipmask[1], ipmask[2], ipmask[3], ipmask[4], ipmask[5], ipmask[6], ipmask[7]);

                int result = lwip_add_addr(netif, addr, mask);
		return result;
            }


            static void lwip_add_addr4(struct netif* netif, uint32_t ipaddr[4], unsigned int ipmask[4])
            {
		struct ip_addr* addr = (struct ip_addr*) malloc(sizeof(struct ip_addr));
		struct ip_addr* mask = (struct ip_addr*) malloc(sizeof(struct ip_addr));

		/* set the local IP address of the interface */
                IP64_ADDR(addr, ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
                IP64_MASKADDR(mask, ipmask[0], ipmask[1], ipmask[2], ipmask[3]);

                int result = lwip_add_addr(netif, addr, mask);
		return result;
            }

            static int lwip_connectw(int socket_fd, char* string_address, unsigned int port_number, int family)
            {
                int result = -1;
                if(family == AF_INET6)
                {
                    struct sockaddr_in6 serv_addr;
                    memset((char *) &serv_addr, 0, sizeof(serv_addr));

                    serv_addr.sin6_family      = family;
                    serv_addr.sin6_port        = htons(port_number);
                    inet_pton(family, string_address, &(serv_addr.sin6_addr));

                    result = lwip_connect(socket_fd, (struct sockaddr *)(&serv_addr), sizeof(serv_addr));
                }
                else
                {
                    struct sockaddr_in serv_addr;
                    memset((char *) &serv_addr, 0, sizeof(serv_addr));

                    serv_addr.sin_family      = family;
                    serv_addr.sin_port        = htons(port_number);
                    //serv_addr.sin_addr.s_addr = inet_addr(string_address);
                    inet_pton(family, string_address, &(serv_addr.sin_addr));

                    result = lwip_connect(socket_fd, (struct sockaddr *)(&serv_addr), sizeof(serv_addr));
                }

		if (result < 0)
                {
                    perror("Socket connecting error");
		}
                return result;
            }

            static char* lwip_recvw(int socket_fd, int len, unsigned int flags)
            {
                void* buffer = malloc(sizeof(char) * len);
                lwip_recv(socket_fd, buffer, len, flags);
                return (char*) buffer;
            }

            static char* lwip_recvfromw(int socket_fd, int len, unsigned int flags, struct sockaddr* from)
            {
                void* buffer = malloc(sizeof(char) * len);
                lwip_recvfrom(socket_fd, buffer, len, flags, from, sizeof(from));

                return (char*) buffer;
            }

            static char* lwip_readw(int socket_fd, int len)
            {
                void* buffer = malloc(sizeof(char) * len);
                lwip_read(socket_fd, buffer, len);
                return (char*) buffer;
            }

            static int lwip_shutdownw(int socket_fd, int mode)
            {
                int sock_mode = -1;
                if(mode == 0)
                {
                    //"r"
                    sock_mode = SHUT_RD;
                }
                else if(mode == 1)
                {
                    //"w"
                    sock_mode = SHUT_WR;
                }
                else if(mode == 2)
                {
                    //"rw"
                    sock_mode = SHUT_RDWR;
                }

                return lwip_shutdown(socket_fd, sock_mode);
            }

            static int lwip_event_subscribe_read(void (*cb)(void *), char *arg, int fd)
            {
                printf("Subscribing to event");
                return lwip_event_subscribe(cb, arg, fd, POLLIN);
            }

            static int lwip_event_subscribe_write(void (*cb)(void *), char *arg, int fd)
            {
                printf("Subscribing to event");
                return lwip_event_subscribe(cb, arg, fd, POLLOUT);
            }
        """,
        libraries=['vdeplug', 'lwipv6'],
        #library_dirs=['/home/federico/Documents/fd_picotcp/nuovosicuro/nuovopico/src'],
        extra_objects=['/usr/lib/liblwipv6.so']
        )

ffi.cdef("""

    extern "Python" void event_callback(void* arg);

    struct __sigset_t {
        ...;
    };
    typedef struct __sigset_t sigset_t;

    typedef unsigned int socklen_t;
    typedef unsigned int nfds_t;

    /* //TODO NON HA NOME IN select.h E' SOLO UN TYPEDEF STRUCT (COME LA METTO?)
    typedef struct {
        ...;
    } fd_set;
    */

    struct stack *lwip_stack_new(void);
    void lwip_stack_free(struct stack *stack);

    struct stack *lwip_stack_get(void);
    void lwip_stack_set(struct stack *stack);

    struct netif *lwip_vdeif_add(struct stack *stack, void *arg);
    struct netif *lwip_tapif_add(struct stack *stack, void *arg);
    struct netif *lwip_tunif_add(struct stack *stack, void *arg);

    int lwip_add_addr(struct netif *netif,struct ip_addr *ipaddr, struct ip_addr *netmask);
    int lwip_del_addr(struct netif *netif,struct ip_addr *ipaddr, struct ip_addr *netmask);

    int lwip_add_route(struct stack *stack, struct ip_addr *addr, struct ip_addr *netmask, struct ip_addr *nexthop, struct netif *netif, int flags);
    int lwip_del_route(struct stack *stack, struct ip_addr *addr, struct ip_addr *netmask, struct ip_addr *nexthop, struct netif *netif, int flags);

    int lwip_ifup(struct netif *netif);
    int lwip_ifdown(struct netif *netif);

    int lwip_msocket(struct stack *stack, int domain, int type, int protocol);
    int lwip_socket(int domain, int type, int protocol);
    int lwip_bind(int s, struct sockaddr *name, socklen_t namelen);
    int lwip_connect(int s, struct sockaddr *name, socklen_t namelen);
    int lwip_listen(int s, int backlog);
    int lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
    int lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);
    int lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);
    int lwip_send(int s, void *dataptr, int size, unsigned int flags);
    int lwip_recv(int s, void *mem, int len, unsigned int flags);
    int lwip_sendto(int s, void *dataptr, int size, unsigned int flags,
       struct sockaddr *to, socklen_t tolen);
    int lwip_recvfrom(int s, void *mem, int len, unsigned int flags,
         struct sockaddr *from, socklen_t *fromlen);
    int lwip_shutdown(int s, int how);
    int lwip_setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);
    int lwip_getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
    int lwip_sendmsg(int fd, const struct msghdr *msg, int flags);
    int lwip_recvmsg(int fd, struct msghdr *msg, int flags);
    int lwip_write(int s, void *dataptr, int size);
    int lwip_read(int s, void *mem, int len);
    int lwip_writev(int s, struct iovec *vector, int count);
    int lwip_readv(int s, struct iovec *vector, int count);
    int lwip_ioctl(int s, long cmd, void *argp);
    int lwip_close(int s);
    //int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
    //int lwip_pselect(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, const struct timespec *timeout, const sigset_t *sigmask);
    int lwip_poll(struct pollfd *fds, nfds_t nfds, int timeout);
    int lwip_ppoll(struct pollfd *fds, nfds_t nfds,
    const struct timespec *timeout, const sigset_t *sigmask);

    typedef void (*lwipvoidfun)();
    int lwip_event_subscribe(void (*cb)(void *), void *arg, int fd, int how);
    int lwip_event_subscribe_read(void (*cb)(void *), void *arg, int fd);
    int lwip_event_subscribe_write(void (*cb)(void *), void *arg, int fd);


    /* PYTHON HELPER FUNCTIONS */
    static int lwip_shutdownw(int socket_fd, int mode);
    static int fetch_port(struct sockaddr *from, int family);
    static struct sockaddr_in* create_empty_sockaddr();
    static struct sockaddr_in* create_server_sockaddr(int family, int port);
    static struct sockaddr_in* create_sockaddr(int family, int port, char* addr);
    static void check_sockaddr(struct sockaddr* sockaddr);
    int sockaddr_size();
    static void lwip_add_addr4(struct netif* netif, uint32_t ipaddr[4], unsigned int ipmask[4]);
    static void lwip_add_addr6(struct netif* netif, uint32_t ipaddr[8], unsigned int ipmask[4]);
    static lwip_connectw(int socket_fd, char* string_address, unsigned int port_number, int family);
    static char* lwip_recvw(int socket_fd, int len, unsigned int flags);
    static char* lwip_recvfromw(int socket_fd, int len, unsigned int flags, struct sockaddr* from);
    static char* lwip_readw(int socket_fd, int len);

        """)

if __name__ == "__main__":
    ffi.compile()
