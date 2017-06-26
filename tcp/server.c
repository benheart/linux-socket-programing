/* TCP Server */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{

    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sfd != -1) {
        printf("Create a socket with fd: %d\n", sfd);
    } else {
        fprintf(stderr, "Fail to create a socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(12345);
    srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(srv_addr.sin_zero, '\0', sizeof srv_addr.sin_zero);

    if (bind(sfd, (const struct sockaddr *) &srv_addr, sizeof srv_addr) ==
        0) {
        printf("Binding socket is OK\n");
    } else {
        fprintf(stderr, "Fail to bind the socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (listen(sfd, 10) == 0) {
        printf("Start listening\n");
    } else {
        fprintf(stderr, "Fail to start listening: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char buf[512];
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    while (1) {
        int c_sock =
            accept(sfd, (struct sockaddr *) &client_addr, &client_addrlen);

        if (c_sock != -1) {
            printf("Accept a new client from %s:%d on socket %d\n",
                   (char *) inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port), c_sock);
        } else {
            fprintf(stderr, "Fail to accept a connection: %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
        }

        memset(buf, 0, sizeof buf);
        ssize_t recv_size = recv(c_sock, buf, 512, 0);
        if (recv_size != -1) {
            printf("Receive a %d-byte message \"%s\" from %s:%d \n",
                   recv_size, buf,
                   (char *) inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));
        } else {
            fprintf(stderr, "Fail to receive: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (shutdown(c_sock, SHUT_RDWR) == 0) {
            printf("Shutdown the socket with fd: %d\n", c_sock);
        } else {
            fprintf(stderr, "Fail to shutdown the socket: %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (close(c_sock) == 0) {
            printf("Close the socket with fd: %d\n", c_sock);
        } else {
            fprintf(stderr, "Fail to close the socket: %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
        }

    }

    if (close(sfd) == 0) {
        printf("Close the socket with fd: %d\n", sfd);
    } else {
        fprintf(stderr, "Fail to close the socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
