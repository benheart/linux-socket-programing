#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>

#pragma pack(1)
struct EchoPacket {
    u_int8_t type;
    u_int8_t code;
    u_int16_t checksum;
    u_int16_t identifier;
    u_int16_t sequence;
    timeval timestamp;
    char data[40];   //sizeof(EchoPacket) == 64
};
#pragma pack()

int sent_num=0;
int recv_num=0; 
unsigned short CheckSum(unsigned short *ptr, int nbytes) ;
inline double CountTime(timeval before, timeval after);
void ping(in_addr_t source, in_addr_t destination) ;

int main(int argc,char *argv[]) 
{
    int i;
    in_addr_t source = inet_addr("192.168.235.136");
    in_addr_t destination;
    // in_addr_t destination = inet_addr("61.135.169.125");
    unsigned long inaddr=0l;
    struct addrinfo *result;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if(argc<2) 
    {       printf("usage:%s hostname/IP address\n",argv[0]); 
            exit(1); 
    } 
    //判断是主机名还是ip地址 
    inaddr=inet_addr(argv[1]);
    
    if( inaddr==INADDR_NONE)//是主机名
    {
        int s = getaddrinfo(argv[1], NULL, &hints, &result); 
        if(s != 0) //根据主机名找不到ip
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(EXIT_FAILURE);
        } 
        destination = inet_addr((char *) inet_ntoa(((struct sockaddr_in *) result->ai_addr)->sin_addr));
    } 
    else    //是ip地址 ,直接赋值
        destination = inaddr;
    
    for(i = 0 ; i < 7; i++)
    {
        ping(source, destination);
        sleep(1);
    }

    printf("-----------------------------------\n");
    printf("%d sent, %d received, %2d%% paket loss\n",sent_num,recv_num,(int)(1-(float)recv_num/sent_num)*100);
    return 0;
}
//计算校验和
unsigned short CheckSum(unsigned short *ptr, int nbytes) 
{
    register long sum;
    unsigned short oddbyte;
    register short answer;
    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char*) &oddbyte) = *(u_char *) ptr;
        sum += oddbyte;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short) ~sum;
    return answer;
}
inline double CountTime(timeval before, timeval after)
{
    return (after.tv_sec - before.tv_sec)*1000 + (after.tv_usec - before.tv_usec)/1000.0;
}
void ping(in_addr_t source, in_addr_t destination) 
{
    static int sequence = 1;
    static int pid = getpid();
    static int ipId = 0;

    char sendBuf[sizeof(iphdr) + sizeof(EchoPacket)] = { 0 };

    struct iphdr * ipHeader = (iphdr *)sendBuf;
    ipHeader->version = 4;
    ipHeader->ihl = 5;

    ipHeader->tos = 0;
    ipHeader->tot_len = htons(sizeof(sendBuf));

    ipHeader->id = htons(ipId++);
    ipHeader->frag_off = htons(0x4000);  //set Flags: don't fragment

    ipHeader->ttl = 64;
    ipHeader->protocol = IPPROTO_ICMP;
    ipHeader->check = 0;
    ipHeader->saddr = source;
    ipHeader->daddr = destination;

    ipHeader->check = CheckSum((unsigned short *)ipHeader, ipHeader->ihl * 2);

    EchoPacket *echoRequest = (EchoPacket *)(sendBuf + sizeof(iphdr));
    echoRequest->type = 8;
    echoRequest->code = 0;
    echoRequest->checksum = 0;
    echoRequest->identifier = htons(pid);
    echoRequest->sequence = htons(sequence++);
    gettimeofday(&(echoRequest->timestamp), NULL);
    u_int16_t ccsum = CheckSum((unsigned short *)echoRequest, sizeof(sendBuf) - sizeof(iphdr));

    echoRequest->checksum = ccsum;

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(0);
    sin.sin_addr.s_addr = destination;

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (s == -1) {
        perror("socket");
        return;
    }

    //IP_HDRINCL to tell the kernel that headers are included in the packet
    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, "1",sizeof("1")) < 0) {
        perror("Error setting IP_HDRINCL");
        exit(0);
    }
    int send_size;
    send_size = sendto(s, sendBuf, sizeof(sendBuf), 0, (struct sockaddr *) &sin, sizeof(sin));
    if(send_size==-1){
        printf("Failed to send: %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
    sent_num++;

    char responseBuf[sizeof(iphdr) + sizeof(EchoPacket)] = {0};

    struct sockaddr_in receiveAddress;
    socklen_t len = sizeof(receiveAddress);
    int reveiveSize = recvfrom(s, (void *)responseBuf, sizeof(responseBuf), 0, (struct sockaddr *) &receiveAddress, &len);

    if(reveiveSize == sizeof(responseBuf)){
        EchoPacket *echoResponse = (EchoPacket *) (responseBuf + sizeof(iphdr));
        //TODO check identifier == pid ?
        if(echoResponse->type == 0)
        {
            recv_num++;
            struct timeval tv;
            gettimeofday(&tv, NULL);

            in_addr tempAddr;
            tempAddr.s_addr = destination;
            printf("%d bytes from %s : icmp_seq=%d ttl=%d time=%.2f ms\n",
                    sizeof(EchoPacket),
                    inet_ntoa(tempAddr),
                    ntohs(echoResponse->sequence),
                    ((iphdr *)responseBuf)->ttl,
                    CountTime(echoResponse->timestamp, tv));
        }else{
            printf("response error, type:%d\n", echoResponse->type);
        }
    }else{
        printf("error, response size != request size.\n");
    }

    close(s);
}