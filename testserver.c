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

#define SERVER_INTERFACE "eth0"

int main()
{
    int fd = socket(AF_PACKET, SOCK_RAW, htons(0x86DC));
    if (fd == -1) {
        printf("open socket fail, errno: %d, %s\n", errno, strerror(errno));
        return 0;
    }

    struct ifreq device;
    memset(&device, 0, sizeof(device));
    memcpy(device.ifr_name, SERVER_INTERFACE, IFNAMSIZ);

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
    strncpy(if_mac.ifr_name, SERVER_INTERFACE, IFNAMSIZ-1);
    if (ioctl(fd, SIOCGIFHWADDR, &if_mac) == -1) {
        printf("SIOCGIFHWADDR fail, errno: %d, %s\n", errno, strerror(errno));
        close(fd);
        return 0;
    }

    printf("%s mac:\n", SERVER_INTERFACE);
    for (int i = 0; i < 6; ++i) {
        printf("%02x ", ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[i]);
    }
    printf("\n");

    struct sockaddr_ll ifsock_addr;
    memset(&ifsock_addr, 0, sizeof(ifsock_addr));
    ifsock_addr.sll_family = AF_PACKET;
    ifsock_addr.sll_ifindex = ifindex;
    ifsock_addr.sll_protocol = htons(0x86DC);
    ifsock_addr.sll_pkttype = PACKET_HOST | PACKET_MULTICAST;
    ifsock_addr.sll_addr[0] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[0];
    ifsock_addr.sll_addr[1] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[1];
    ifsock_addr.sll_addr[2] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[2];
    ifsock_addr.sll_addr[3] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[3];
    ifsock_addr.sll_addr[4] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[4];
    ifsock_addr.sll_addr[5] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[5];
    ret = bind(fd, (struct sockaddr*)&ifsock_addr, sizeof(ifsock_addr));
    if (ret == -1) {
        printf("bind fail, errno: %d, %s\n", errno, strerror(errno));
        close(fd);
        return 0;
    }

    printf("bind successful!\n");

    while (1) {
        socklen_t size = 0;
        unsigned char frame[1024] = {0};
        ret = recvfrom(fd, frame, sizeof(frame), 0, (struct sockaddr*)&ifsock_addr, &size);
        if (ret > 0) {
            printf("recv from %02x-%02x-%02x-%02x-%02x-%02x\n", ifsock_addr.sll_addr[0], ifsock_addr.sll_addr[1], ifsock_addr.sll_addr[2], ifsock_addr.sll_addr[3], ifsock_addr.sll_addr[4], ifsock_addr.sll_addr[5]);
            printf("size: %d %d\n", size, ret);
            for (int i = 0; i < ret; ++i) {
                printf("%02x ", frame[i]);
            }
            printf("\n---------------------------\n");
        }
        else {
            printf("recvfrom packet socket failed, errno: %d, %s\n", errno, strerror(errno));
        }
    }
    close(fd);
    return 0;
}
