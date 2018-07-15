#!/usr/bin/env python

from cffi import FFI
ffi = FFI()

#ffi.dlopen("libfdpicotcp.so")

ffi.set_source("_fdpicotcp",
        """
            #include <sys/stat.h>

            #include <sys/un.h>
            #include <sys/types.h>
            //#include <sys/socket.h>

            #include <poll.h>
            #include <netinet/in.h>
            #include <arpa/inet.h>

            #include <fd_picotcp.h>
            #include <fede_socket.h>

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

            static struct sockaddr* create_empty_sockaddr()
            {
                struct sockaddr_in* sockaddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
                return (struct sockaddr*) sockaddr;
            }

            static struct sockaddr* create_server_sockaddr(int family, int port)
            {
                struct sockaddr_in* sockaddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
                sockaddr->sin_family = family;
                sockaddr->sin_port = htons(port);
                sockaddr->sin_addr.s_addr = INADDR_ANY;

                return (struct sockaddr*) sockaddr;
            }

            static struct sockaddr* create_sockaddr(int family, int port, char* addr)
            {
                struct sockaddr_in* sockaddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
                sockaddr->sin_family = family;
                sockaddr->sin_port = htons(port);
                sockaddr->sin_addr.s_addr = inet_addr(addr);

                return (struct sockaddr*) sockaddr;
            }

            int sockaddr_size()
            {
                return sizeof(struct sockaddr_in);
            }

            static int pico_connectw(int socket_fd, char* string_address, unsigned int port_number)
            {
                struct sockaddr_in serv_addr;

		memset((char *) &serv_addr,0,sizeof(serv_addr));
		serv_addr.sin_family      = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(string_address);
		serv_addr.sin_port        = htons(port_number);

                int result = pico_connect(socket_fd, (struct sockaddr *)(&serv_addr), sizeof(serv_addr));
		if (result < 0)
                {
                    perror("Socket connecting error");
		}
                return result;
            }

            static char* pico_readw(int socket_fd, int len)
            {
                void* buffer = malloc(sizeof(char) * len);
                pico_read(socket_fd, buffer, len);
                return (char*) buffer;
            }

        """,
        libraries=['vdeplug', 'fdpicotcp'],
        #library_dirs=['/home/federico/Documents/fd_picotcp/nuovosicuro/nuovopico/src', '/home/federico/Documents/fd_picotcp/nuovosicuro/nuovopico/src/picotcp/include'],
        extra_objects=['/usr/local/lib/libfdpicotcp.so']
        )

ffi.cdef("""
        typedef int ssize_t;
        typedef unsigned int socklen_t;

        void pico_stack_init();
        void pico_stack_tick();

        ssize_t pico_read(int fd, void* buf, size_t len);
        ssize_t pico_write(int fd, const void* buf, size_t count);
        int pico_listen(int fd, int backlog);
        int pico_close(int fd);
        int pico_socket(int domain, int type, int protocol);
        int pico_bind(int fd, const struct sockaddr* addr, socklen_t addrlen);
        int pico_connect(int fd, const struct sockaddr* addr, socklen_t addrlen);
        int pico_accept(int fd, const struct sockaddr* addr, socklen_t addrlen);

        static void check_sockaddr(struct sockaddr_in* sockaddr);
        static struct sockaddr* create_empty_sockaddr();
        static struct sockaddr* create_server_sockaddr(int family, int port);
        static struct sockaddr* create_sockaddr(int family, int port, char* addr);
        int sockaddr_size();
        static char* pico_readw(int socket_fd, int len);
        static int pico_connectw(int socket_fd, char* string_address, unsigned int port_number);
        """)

if __name__ == "__main__":
    ffi.compile()
