#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <unistd.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#define IP "127.0.0.1"
struct eth_header {
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
    unsigned short ethertype;
};

struct ip_header {
    unsigned char  ihl:4, version:4;
    unsigned char  tos;
    uint16_t length;
    uint16_t id;
    uint16_t frag_off;
    unsigned char  ttl;
    unsigned char  protocol;
    uint16_t checksum;
    uint32_t source_ip;
    uint32_t dest_ip;
};

struct udp_header {
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
};

int main() {
    int sockfd;
    struct sockaddr_ll sa;
    struct ifreq ifr;
    char packet[4096];

    memset(packet, 0, 4096);

    struct eth_header *eth = (struct eth_header *)packet;
    struct ip_header *iph = (struct ip_header *)(packet + sizeof(struct eth_header));
    struct udp_header *udph = (struct udp_header *)(packet + sizeof(struct eth_header) + sizeof(struct ip_header));
    char *data = packet + sizeof(struct eth_header) + sizeof(struct ip_header) + sizeof(struct udp_header);

    const char *message = "Hello UDP Server!";
    int data_size = strlen(message);

    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (sockfd < 0) {
        perror("Socket");
        exit(EXIT_FAILURE);
    }

    memset(eth->dest_mac, 0x0, 6); 
    memset(eth->src_mac, 0x0, 6);

    eth->ethertype = htons(ETH_P_IP);

    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->length = htons(sizeof(struct ip_header) + sizeof(struct udp_header) + data_size);
    iph->id = htons(8566);
    iph->frag_off = 0;//htons(0x4000);
    iph->ttl = 64;
    iph->protocol = IPPROTO_UDP;
    iph->checksum = 0;
    iph->source_ip = inet_addr(IP); 
    iph->dest_ip = inet_addr(IP); 

    udph->source_port = htons(47266);
    udph->dest_port = htons(8080);
    udph->length = htons(sizeof(struct udp_header) + data_size);
    udph->checksum = 0;

    memcpy(data, message, data_size);

    iph->checksum = 0;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "lo", IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("IOCTL");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sll_ifindex = ifr.ifr_ifindex;
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_IP);

    if (sendto(sockfd, packet, sizeof(struct eth_header) + sizeof(struct ip_header) + sizeof(struct udp_header) + data_size, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Message Sent.\n");

    close(sockfd);

    return 0;
}
