#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#define PORT 8080
#define BUF_SIZE 512
#define MAX_CONN 16
#define MAX_EVENTS 32
#define ADDR_BUF_SIZE 16

void set_sockaddr(struct sockaddr_in *addr)
{
    bzero((char *)addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr->sin_port = htons(PORT);
}

void epoll_ctl_add(int epfd, int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev))
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
}

int setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) == -1)
    {
        return -1;
    }

    return 0;
}

int main(int argc, char const *argv[])
{
    int n;
    int epfd;
    int nfds; // events count
    struct epoll_event eventsfd[MAX_EVENTS];
    int server_fd, conn_socket;
    ssize_t valread;
    struct sockaddr_in srv_addr, cliaddress;
    int opt = 1;
    int keepalive_delay = 30;
    int count = 1; // keep alive probes count
    socklen_t addrlen = sizeof(srv_addr);
    char addr_buf[ADDR_BUF_SIZE] = {0};
    char buffer[1024] = {0};
    char *msg = "Hello from Server";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)))
    {
        perror("SO_KEEPALIVE");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_delay, sizeof(keepalive_delay)))
    {
        perror("TCP_KEEPIDLE");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_delay, sizeof(keepalive_delay)))
    {
        perror("TCP_KEEPINTVL");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count))) // max num of keepalives probes TCP should send before dropping connection
    {
        perror("TCP_KEEPCNT");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt))) // disable Nagle algorithm
    {
        perror("TCP_NODELAY");
        exit(EXIT_FAILURE);
    }

    set_sockaddr(&srv_addr);

    if (bind(server_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)))
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    setnonblocking(server_fd);

    if (listen(server_fd, MAX_CONN))
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    epfd = epoll_create1(0);
    epoll_ctl_add(epfd, server_fd, EPOLLIN);
    int currsockfd;
    for (;;)
    {
        nfds = epoll_wait(epfd, eventsfd, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++)
        {
            if (eventsfd[i].data.fd == server_fd)
            {
                for (;;)
                {
                    conn_socket = accept(server_fd, (struct sockaddr *)&cliaddress, &addrlen);
                    if (conn_socket == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            break;
                        }
                        perror("accept");
                        break;
                    }
                    inet_ntop(AF_INET, (char *)&(cliaddress.sin_addr), addr_buf, sizeof(addr_buf));
                    printf("[+] connecte with %s:%d\n", addr_buf, ntohs(cliaddress.sin_port));
                    setnonblocking(conn_socket);
                    epoll_ctl_add(epfd, conn_socket, EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR);
                }
            }
            else
            {
                if (eventsfd[i].events & (EPOLLERR | EPOLLRDHUP | EPOLLHUP))
                {
                    printf("[+] connection closed\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, eventsfd[i].data.fd, NULL);
                    close(eventsfd[i].data.fd);
                    continue;
                }

                if (eventsfd[i].events & EPOLLIN)
                {
                    for (;;)
                    {
                        bzero(buffer, sizeof(buffer));
                        n = read(eventsfd[i].data.fd, buffer, sizeof(buffer) - 1);
                        if (n > 0 /* || errno == EAGAIN*/)
                        {
                            buffer[n] = '\0';
                            printf("[+] data: %s\n", buffer);
                        }
                        else if (n == -1 && errno == EAGAIN)
                        {
                            break;
                        }
                        else
                        {
                            perror("read");
                            close(eventsfd[i].data.fd);
                            break;
                        }
                    }
                }
            }
        }
    }

    // valread = read(conn_socket, buffer, 1024 - 1);
    // printf("%s\n", buffer);
    // send(conn_socket, msg, strlen(msg), 0);
    // printf("Hello message sent\n");

    // close(conn_socket);
    // close(server_fd);

    return 0;
}