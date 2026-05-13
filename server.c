#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, conn_socket;
    ssize_t valread;
    struct sockaddr_in servaddress, cliaddress;
    int opt = 1;
    int keepalive_delay = 30;
    int count = 1; // keep alive probes count
    socklen_t addrlen = sizeof(servaddress);
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

    servaddress.sin_family = AF_INET;
    servaddress.sin_addr.s_addr = INADDR_ANY;
    servaddress.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&servaddress, sizeof(servaddress)))
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3))
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((conn_socket = accept(server_fd, (struct sockaddr *)&cliaddress, &addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    valread = read(conn_socket, buffer, 1024 - 1);
    printf("%s\n", buffer);
    send(conn_socket, msg, strlen(msg), 0);
    printf("Hello message sent\n");

    close(conn_socket);
    close(server_fd);

    return 0;
}