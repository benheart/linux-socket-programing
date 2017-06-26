/* TCP Client */

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
    int s = socket(AF_INET, SOCK_STREAM, 0);

    if (s != -1) {
        printf("Create a socket with fd: %d\n", s);
    } else {
        fprintf(stderr, "Fail to create a socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in srv_addr;

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(12345);
    srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset(srv_addr.sin_zero, '\0', sizeof srv_addr.sin_zero);

    if (connect(s, (const struct sockaddr *) &srv_addr, sizeof srv_addr) ==
        0) {
        printf("Connect to the server\n");
    } else {
        fprintf(stderr, "Fail to connect to the server: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    char buf[512];
    printf("Please type your message: ");
    fgets(buf, 512, stdin);
    buf[strlen(buf) - 1] = 0;

    ssize_t send_size = send(s, buf, strlen(buf), 0);

    if (send_size != -1) {
        printf("Send a %d-byte message \"%s\" to the server\n", send_size,
               buf);
    } else {
        fprintf(stderr, "Fail to send: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (shutdown(s, SHUT_RDWR) == 0) {
        printf("Shutdown the socket with fd: %d\n", s);
    } else {
        fprintf(stderr, "Fail to shutdown the socket: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(s) == 0) {
        printf("Close the socket with fd: %d\n", s);
    } else {
        fprintf(stderr, "Fail to close the socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
