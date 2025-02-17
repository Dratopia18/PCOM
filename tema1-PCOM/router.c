#include "queue.h"
#include "lib.h"
#include "protocols.h"
#include <arpa/inet.h>
#include <string.h>

#define ETHERTYPE_IP 0x0800
#define ETHERTYPE_ARP 0x0806
#define ICMP_PROTOCOL 1
#define ARP_REQUEST 1
#define ARP_REPLY 2

struct route_table_entry *rtable;
int rtable_size;

struct arp_table_entry *arp_table;
int arp_table_size;

//facem compararea pentru qsort
int compare(const void *a, const void *b) {
    struct route_table_entry *route1 = (struct route_table_entry *)a;
    struct route_table_entry *route2 = (struct route_table_entry *)b;

    if ((route1->mask & route1->prefix) == (route2->mask & route2->prefix)) {
        if (ntohl(route1->mask) > ntohl(route2->mask)) {
            return 0;
        } else {
            return 1;
        }
    } else if (ntohl(route1->mask & route1->prefix) > ntohl(route2->mask & route2->prefix)) {
        return 0;
    } else {
        return 1;
    }
}

struct route_table_entry *get_best_route(uint32_t dest_ip) {
    struct route_table_entry *best_route = NULL;
    for (int i = 0; i < rtable_size; i++) {
        if ((dest_ip & rtable[i].mask) == rtable[i].prefix) {
            if (best_route == NULL || rtable[i].mask > best_route->mask) {
                best_route = &rtable[i];
            }
        }
    }
    return best_route;
}

struct route_table_entry *get_best_route_binary(uint32_t dest_ip) {
    int left = 0;
    int right = rtable_size - 1;
    struct route_table_entry *best_route = NULL;
    while (left <= right) {
        int mid = (left + right) / 2;
        if ((dest_ip & rtable[mid].mask) == (rtable[mid].prefix & rtable[mid].mask)) {
            if (best_route == NULL || rtable[mid].mask > best_route->mask) {
                best_route = &rtable[mid];
            }
            right = mid - 1;
        } else if (ntohl(dest_ip & rtable[mid].mask) > ntohl(rtable[mid].prefix)) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    return best_route;
}

struct arp_table_entry *get_arp_entry(uint32_t ip) {
    for (int i = 0; i < arp_table_size; i++) {
        if (arp_table[i].ip == ip) {
            return &arp_table[i];
        }
    }
    return NULL;
}

void make_arp_request(uint32_t ip, int interface) {
    char buf[98];
    memset(buf, 0, 98);
    struct ether_header *eth_hdr = (struct ether_header *)buf;
    eth_hdr->ether_type = htons(ETHERTYPE_ARP);

    uint8_t mac[6];
    get_interface_mac(interface, mac);

    memcpy(eth_hdr->ether_shost, mac, 6);
    memset(eth_hdr->ether_dhost, 0xff, 6);

    struct arp_header *arp_hdr = (struct arp_header *)(buf + sizeof(struct ether_header));
    arp_hdr->htype = htons(1);
    arp_hdr->ptype = htons(ETHERTYPE_IP);
    arp_hdr->hlen = 6;
    arp_hdr->plen = 4;
    arp_hdr->op = htons(ARP_REQUEST);

    get_interface_mac(interface, arp_hdr->sha);
    arp_hdr->spa = inet_addr(get_interface_ip(interface));
    memset(arp_hdr->tha, 0, 6);
    arp_hdr->tpa = ip;

    send_to_link(interface, buf, 98);
}

void arp_reply(char *buf, int interface) {
    struct ether_header *eth_hdr = (struct ether_header *)buf;
    struct arp_header *arp_hdr = (struct arp_header *)(buf + sizeof(struct ether_header));

    if (arp_hdr->op != htons(ARP_REQUEST)) {
        return;
    }

    char reply_buf[98];
    memset(reply_buf, 0, 98);

    struct ether_header *reply_eth_hdr = (struct ether_header *)reply_buf;
    reply_eth_hdr->ether_type = htons(ETHERTYPE_ARP);

    uint8_t mac[6];
    get_interface_mac(interface, mac);

    memcpy(reply_eth_hdr->ether_shost, mac, 6);
    memcpy(reply_eth_hdr->ether_dhost, eth_hdr->ether_shost, 6);

    struct arp_header *reply_arp_hdr = (struct arp_header *)(reply_buf + sizeof(struct ether_header));
    reply_arp_hdr->htype = htons(1);
    reply_arp_hdr->ptype = htons(ETHERTYPE_IP);
    reply_arp_hdr->hlen = 6;
    reply_arp_hdr->plen = 4;
    reply_arp_hdr->op = htons(ARP_REPLY);

    get_interface_mac(interface, reply_arp_hdr->sha);
    reply_arp_hdr->spa = inet_addr(get_interface_ip(interface));
    memcpy(reply_arp_hdr->tha, arp_hdr->sha, 6);
    reply_arp_hdr->tpa = arp_hdr->spa;

    send_to_link(interface, reply_buf, 98);
}

void icmp_error(int type, int code, char *buf, int interface) {
    struct ether_header *eth_hdr = (struct ether_header *)buf;
    struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header));
    char new_buf[98];
    memset(new_buf, 0, 98);
    struct ether_header *new_eth_hdr = (struct ether_header *)new_buf;
    new_eth_hdr->ether_type = htons(ETHERTYPE_IP);

    uint8_t mac[6];
    get_interface_mac(interface, mac);

    memcpy(new_eth_hdr->ether_shost, mac, 6);
    memcpy(new_eth_hdr->ether_dhost, eth_hdr->ether_shost, 6);

    struct iphdr *new_ip_hdr = (struct iphdr *)(new_buf + sizeof(struct ether_header));
    new_ip_hdr->version = 4;
    new_ip_hdr->ihl = 5;
    new_ip_hdr->tos = 0;
    new_ip_hdr->tot_len = htons(sizeof(struct iphdr) + 64);
    new_ip_hdr->id = htons(1);
    new_ip_hdr->frag_off = 0;
    new_ip_hdr->ttl = 64;
    new_ip_hdr->protocol = 1;

    char *interface_ip = get_interface_ip(interface);
    in_addr_t ip_interface_source;
    inet_aton(interface_ip, (struct in_addr *)&ip_interface_source);

    new_ip_hdr->daddr = ip_hdr->saddr;
    new_ip_hdr->saddr = ip_interface_source;
    new_ip_hdr->check = 0;

    struct icmphdr *icmp_hdr = (struct icmphdr *)(new_buf + sizeof(struct ether_header) + sizeof(struct iphdr));
    icmp_hdr->type = type;
    icmp_hdr->code = code;
    icmp_hdr->checksum = 0;

    send_to_link(interface, new_buf, 98);
}

void echo_reply(char* buf, int interface, size_t len) {
    struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header));
    
    char reply_buf[98];
    memset(reply_buf, 0, 98);

    memcpy(reply_buf, buf, sizeof(struct ether_header));

    memcpy(reply_buf + sizeof(struct ether_header), ip_hdr, sizeof(struct iphdr));

    struct iphdr *reply_ip_hdr = (struct iphdr *)(reply_buf + sizeof(struct ether_header));
    reply_ip_hdr->tot_len = htons(sizeof(struct iphdr) + 64);

    struct icmphdr *icmp_hdr = (struct icmphdr *)(reply_buf + sizeof(struct ether_header) + sizeof(struct iphdr));
    icmp_hdr->type = 0;
    icmp_hdr->code = 0;
    icmp_hdr->checksum = 0;
    icmp_hdr->checksum = htons(checksum((uint16_t *)icmp_hdr, sizeof(struct icmphdr)));

    send_to_link(interface, reply_buf, 98);
}


void handle_packet(char* buf, size_t len, int interface, struct queue *q) {
	struct ether_header *eth_hdr = (struct ether_header *)buf;

    if (len < sizeof(struct ether_header)) {
        return;
    }

    if (ntohs(eth_hdr->ether_type) ==  ETHERTYPE_IP) {
        struct iphdr *ip_hdr = (struct iphdr *)(buf + sizeof(struct ether_header));

        if (checksum((uint16_t *)ip_hdr, sizeof(struct iphdr)) != 0) {
            printf("Checksum error\n");
            return;
        }

        struct route_table_entry *best_route = get_best_route_binary(ip_hdr->daddr);
        if (best_route == NULL) {
            printf("No route found\n");
            icmp_error(3, 0, buf, interface);
            return;
        }

		uint8_t interface_mac[6];
    	get_interface_mac(best_route->interface, interface_mac);

        if (ip_hdr->ttl <= 1) {
            printf("TTL expired\n");
            icmp_error(11, 0, buf, interface);
            return;
        }

        if (inet_addr(get_interface_ip(interface)) == ip_hdr->daddr) {
            echo_reply(buf, interface, len);
            return;
        }

        ip_hdr->ttl--;
        ip_hdr->check = 0;
        ip_hdr->check = htons(checksum((uint16_t *)ip_hdr, sizeof(struct iphdr)));

        struct arp_table_entry *arp_entry = get_arp_entry(best_route->next_hop);
        if (arp_entry == NULL) {
            printf("No ARP entry found\n");
            char *new_buf = malloc(len);
            memcpy(new_buf, buf, len);
            queue_enq(q, new_buf);
            make_arp_request(best_route->next_hop, best_route->interface);
            return;
        }
		
        memcpy(eth_hdr->ether_shost, interface_mac, 6);
        memcpy(eth_hdr->ether_dhost, arp_entry->mac, 6);

        send_to_link(best_route->interface, buf, len);

    } else if (ntohs(eth_hdr->ether_type) == ETHERTYPE_ARP) {
        struct arp_header *arp_hdr = (struct arp_header *)(buf + sizeof(struct ether_header));

        if (arp_hdr->op == htons(ARP_REQUEST)) {
            arp_reply(buf, interface);
        } else if (arp_hdr->op == htons(ARP_REPLY)) {
            memcpy(arp_table[arp_table_size].mac, arp_hdr->sha, 6);
            arp_table[arp_table_size].ip = arp_hdr->spa;
            arp_table_size++;

            while (!queue_empty(q)) {
                buf = queue_deq(q);
                handle_packet(buf, len, interface, q);
            }
        }
    }
}
int main(int argc, char *argv[]) {
    char buf[MAX_PACKET_LEN];

    // Do not modify this line
    init(argc - 2, argv + 2);

    rtable = malloc(sizeof(struct route_table_entry) * 100000);
    DIE(rtable == NULL, "memory");

    rtable_size = read_rtable(argv[1], rtable);

    qsort(rtable, rtable_size, sizeof(struct route_table_entry), compare);

    arp_table = malloc(sizeof(struct route_table_entry) * 100000);
    DIE(arp_table == NULL, "memory");

    arp_table_size = 0;

    struct queue *q = queue_create();
    

    while (1) {
        int interface;
        size_t len;

        interface = recv_from_any_link(buf, &len);
        DIE(interface < 0, "recv_from_any_links");
        printf("We have received a packet\n");

		handle_packet(buf, len, interface, q);

	}
    free(rtable);
    free(arp_table);
    return 0;
}