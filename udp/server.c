/* UDP Server */

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

    int s = socket(AF_INET, SOCK_DGRAM, 0);

    if (s != -1) {
        printf("Create a socket with fd: %d\n", s);
    } else {
        fprintf(stderr, "Fail to create a socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(12345);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(srv_addr.sin_zero, '\0', sizeof srv_addr.sin_zero);

    if (bind(s, (const struct sockaddr *) &srv_addr, sizeof srv_addr) == 0) {
        printf("Bind socket to 127.0.0.1:12345\n");
    } else {
        fprintf(stderr, "Fail to bind the socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char buf[512];
    struct sockaddr_in client_addr;
    socklen_t client_addrlen;

    while (1) {
        memset(buf, 0, sizeof buf);
        ssize_t recv_size =
            recvfrom(s, buf, 512, 0, (struct sockaddr *) &client_addr,
                     &client_addrlen);
        if (recv_size != -1) {
            printf("Receive a %d-byte message \"%s\" from %s:%d \n",
                   recv_size, buf,
                   (char *) inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));
        } else {
            fprintf(stderr, "Fail to receive: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    close(s);

    exit(EXIT_SUCCESS);
}
