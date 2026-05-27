#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 8080

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
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char msg[512];
    // char msg_from_cli[512];
    char buffer[1024] = {0};

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    setnonblocking(client_fd);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        if (errno != EINPROGRESS)
        {
            perror("connect");
            return -1;
        }
    }

    while (1)
    {
        // scanf("%s", msg);
        fgets(msg, sizeof(msg), stdin);
        if ((strlen(msg) > 0) && (msg[strlen(msg) - 1] == '\n'))
        {
            msg[strlen(msg) - 1] = '\0';
        }

        send(client_fd, msg, strlen(msg), 0);

        while (1)
        {
            valread = read(client_fd, buffer, 1024 - 1);

            if (valread > 0)
            {
                buffer[valread] = '\0';
                printf("%s\n", buffer);
                break;
            }

            if (valread == -1 && errno == EAGAIN)
            {
                usleep(1000);
                // continue;
            }

            break;
        }
    }

    close(client_fd);

    return 0;
}