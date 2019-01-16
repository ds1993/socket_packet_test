#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define CLIENT_INTERFACE "enp1s0"

const char dhost[] = {0xdc, 0x4a, 0x3e, 0x6d, 0xd9, 0x20};

int main()
{
    int fd = socket(AF_PACKET, SOCK_RAW, htons(0x86DC));
    if (fd == -1) {
        printf("open socket fail, errno: %d, %s\n", errno, strerror(errno));
        return 0;
    }

    struct ifreq device;
    memset(&device, 0, sizeof(device));
    memcpy(device.ifr_name, CLIENT_INTERFACE, IFNAMSIZ);

    int ret = ioctl(fd, SIOCGIFINDEX, &device);
    if (ret == -1) {
        printf("ioctl fail, errno: %d, %s\n", errno, strerror(errno));
        close(fd);
        return 0;
    }

    int ifindex = device.ifr_ifindex;
    printf("device interface index is %d.\n", ifindex);

    struct ifreq if_mac;
    memset(&if_mac, 0, sizeof(if_mac));
    strncpy(if_mac.ifr_name, CLIENT_INTERFACE, IFNAMSIZ-1);
    if (ioctl(fd, SIOCGIFHWADDR, &if_mac) == -1) {
        printf("SIOCGIFHWADDR fail, errno: %d, %s\n", errno, strerror(errno));
        close(fd);
        return 0;
    }

    char sendbuf[1024] = {0};
    struct ether_header* eh = (struct ether_header*)sendbuf;
    eh->ether_shost[0] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[0];
    eh->ether_shost[1] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[1];
    eh->ether_shost[2] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[2];
    eh->ether_shost[3] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[3];
    eh->ether_shost[4] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[4];
    eh->ether_shost[5] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[5];
    eh->ether_dhost[0] = dhost[0];
    eh->ether_dhost[1] = dhost[1];
    eh->ether_dhost[2] = dhost[2];
    eh->ether_dhost[3] = dhost[3];
    eh->ether_dhost[4] = dhost[4];
    eh->ether_dhost[5] = dhost[5];
    eh->ether_type = htons(0x86DC);
    int tx_len = sizeof(struct ether_header);
    sendbuf[tx_len++] = 'f';
    sendbuf[tx_len++] = 'u';
    sendbuf[tx_len++] = 'c';
    sendbuf[tx_len++] = 'k';
    sendbuf[tx_len++] = '!';
    sendbuf[tx_len++] = '\n';
    sendbuf[tx_len++] = 0;

    struct sockaddr_ll ifsock_addr;
    memset(&ifsock_addr, 0, sizeof(ifsock_addr));
    ifsock_addr.sll_family = AF_PACKET;
    ifsock_addr.sll_ifindex = ifindex;
    ifsock_addr.sll_addr[0] = dhost[0];
    ifsock_addr.sll_addr[1] = dhost[1];
    ifsock_addr.sll_addr[2] = dhost[2];
    ifsock_addr.sll_addr[3] = dhost[3];
    ifsock_addr.sll_addr[4] = dhost[4];
    ifsock_addr.sll_addr[5] = dhost[5];
    ifsock_addr.sll_halen = ETH_ALEN;

    ret = sendto(fd, sendbuf, tx_len, 0, (struct sockaddr*)&ifsock_addr, sizeof(ifsock_addr));
    if (ret < 0) {
        printf("sentto failed, error: %d, %s\n", errno, strerror(errno));
        close(fd);
        return 0;
    }

    printf("send to with result %d.\n", ret);
    close(fd);

    return 0;

}
