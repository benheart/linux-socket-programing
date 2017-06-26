/* raw socket */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
//#include <netpacket/packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#define IP_MAX_SIZE 65535
#define ETH_HLEN        14      /* Total octets in header.       */
#define ETH_ZLEN	60      /* Min. octets in frame sans FCS */

int main(int argc, char *argv[])
{
    int s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));

    if (s != -1) {
        printf("Create a raw socket with fd: %d\n", s);
    } else {
        fprintf(stderr, "Fail to create a raw socket: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (1) {
        int size;
        char packet[IP_MAX_SIZE];

        size = recv(s, packet, IP_MAX_SIZE, 0);
        if (size == -1) {
            if (errno == EINTR) {
                continue;       /* 'cause sd is readable */
            } else {
                fprintf(stderr, "Fail to recv: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        if (size < ETH_HLEN + sizeof(struct iphdr)) {
            fprintf(stderr, "Too small packet of %d bytes\n", size);
            continue;
        }

        /* skip the ethernet header */
        struct iphdr *header = (struct iphdr *) (packet + ETH_HLEN);
        size -= ETH_HLEN;
        int ip_size = ntohs(header->tot_len);
        if (ip_size > size) {
            fprintf(stderr,
                    "IP packet size is greater than the frame payload size\n");
            continue;
        }
        printf("\nA %d-byte IP packet from %s", ip_size,
               inet_ntoa(*(struct in_addr *) &(header->saddr)));
        printf(" to %s\n",
               inet_ntoa(*(struct in_addr *) &(header->daddr)));
        int i = 0;
        for (i = 0; i < ip_size; i++) {
            printf("%3d (%2x)  ", ((unsigned char *) header)[i],
                   ((unsigned char *) header)[i]);
            if (i % 4 == 3) {
                printf("\n");
            }
        }
    }

    if (close(s) == 0) {
        printf("Close the socket with fd: %d\n", s);
    } else {
        fprintf(stderr, "Fail to close the socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
