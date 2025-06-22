#include "ProtocolExtractors.h"
#include "../../shared/Logger.h"
#include <string.h>
#include <arpa/inet.h>

// Ethernet header structure
struct ethernet_header {
    u_char dest[6];
    u_char source[6];
    u_short type;
};

// IP header structure
struct ip_header {
    u_char version_ihl;
    u_char tos;
    u_short total_length;
    u_short id;
    u_short frag_offset;
    u_char ttl;
    u_char protocol;
    u_short checksum;
    struct in_addr source;
    struct in_addr dest;
};

// TCP header structure
struct tcp_header {
    u_short source_port;
    u_short dest_port;
    u_int seq_num;
    u_int ack_num;
    u_char data_offset;
    u_char flags;
    u_short window;
    u_short checksum;
    u_short urgent_ptr;
};

int extract_ip_src_addr(const u_char* packet, char* result) {
    if (!packet || !result) {
        return -1;
    }
    
    struct ethernet_header* eth = (struct ethernet_header*)packet;
    struct ip_header* ip = (struct ip_header*)(packet + 14); // Skip Ethernet header
    
    if (ntohs(eth->type) != 0x0800) { // Not IP
        return -1;
    }
    
    inet_ntop(AF_INET, &(ip->source), result, INET_ADDRSTRLEN);
    return 0;
}

int extract_ip_dst_addr(const u_char* packet, char* result) {
    if (!packet || !result) {
        return -1;
    }
    
    struct ethernet_header* eth = (struct ethernet_header*)packet;
    struct ip_header* ip = (struct ip_header*)(packet + 14);
    
    if (ntohs(eth->type) != 0x0800) {
        return -1;
    }
    
    inet_ntop(AF_INET, &(ip->dest), result, INET_ADDRSTRLEN);
    return 0;
}

int extract_tcp_src_port(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    struct ethernet_header* eth = (struct ethernet_header*)packet;
    struct ip_header* ip = (struct ip_header*)(packet + 14);
    
    if (ntohs(eth->type) != 0x0800 || ip->protocol != 6) { // Not TCP
        return -1;
    }
    
    struct tcp_header* tcp = (struct tcp_header*)(packet + 14 + ((ip->version_ihl & 0x0F) * 4));
    *result = ntohs(tcp->source_port);
    return 0;
}

int extract_tcp_dst_port(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    struct ethernet_header* eth = (struct ethernet_header*)packet;
    struct ip_header* ip = (struct ip_header*)(packet + 14);
    
    if (ntohs(eth->type) != 0x0800 || ip->protocol != 6) {
        return -1;
    }
    
    struct tcp_header* tcp = (struct tcp_header*)(packet + 14 + ((ip->version_ihl & 0x0F) * 4));
    *result = ntohs(tcp->dest_port);
    return 0;
}

int extract_timestamp(const struct pcap_pkthdr* header, char* result) {
    if (!header || !result) {
        return -1;
    }
    
    time_t timestamp = header->ts.tv_sec;
    struct tm* tm_info = localtime(&timestamp);
    
    strftime(result, 64, "%Y-%m-%d %H:%M:%S", tm_info);
    return 0;
} 