#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif
#include "../../shared/Type.h"
#include "ProtocolExtractors.h"
#include "../../shared/Logger.h"
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>

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

// UDP header structure
struct udp_header {
    u_short source_port;
    u_short dest_port;
    u_short length;
    u_short checksum;
};

// Helper function to get IP header offset
static int get_ip_header_offset(const u_char* packet) {
    struct ethernet_header* eth = (struct ethernet_header*)packet;
    if (ntohs(eth->type) == 0x0800) { // IPv4
        return 14; // Ethernet header size
    }
    return -1; // Not IPv4
}

// Helper function to get TCP header offset
static int get_tcp_header_offset(const u_char* packet) {
    int ip_offset = get_ip_header_offset(packet);
    if (ip_offset < 0) return -1;
    
    struct ip_header* ip = (struct ip_header*)(packet + ip_offset);
    if (ip->protocol != 6) return -1; // Not TCP
    
    return ip_offset + ((ip->version_ihl & 0x0F) * 4);
}

// Helper function to get UDP header offset
static int get_udp_header_offset(const u_char* packet) {
    int ip_offset = get_ip_header_offset(packet);
    if (ip_offset < 0) return -1;
    
    struct ip_header* ip = (struct ip_header*)(packet + ip_offset);
    if (ip->protocol != 17) return -1; // Not UDP
    
    return ip_offset + ((ip->version_ihl & 0x0F) * 4);
}

// Helper function to parse HTTP headers
static char* parse_http_header(const char* payload, const char* header_name) {
    char* line_start = strstr(payload, header_name);
    if (!line_start) return NULL;
    
    char* value_start = strchr(line_start, ':');
    if (!value_start) return NULL;
    
    value_start++; // Skip ':'
    while (*value_start == ' ') value_start++; // Skip spaces
    
    char* line_end = strchr(value_start, '\r');
    if (!line_end) line_end = strchr(value_start, '\n');
    if (!line_end) return NULL;
    
    int len = line_end - value_start;
    char* result = malloc(len + 1);
    strncpy(result, value_start, len);
    result[len] = '\0';
    
    return result;
}

int extract_ip_src_addr(const u_char* packet, char* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int ip_offset = get_ip_header_offset(packet);
    if (ip_offset < 0) return -1;
    
    struct ip_header* ip = (struct ip_header*)(packet + ip_offset);
    inet_ntop(AF_INET, &(ip->source), result, INET_ADDRSTRLEN);
    return 0;
}

int extract_ip_dst_addr(const u_char* packet, char* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int ip_offset = get_ip_header_offset(packet);
    if (ip_offset < 0) return -1;
    
    struct ip_header* ip = (struct ip_header*)(packet + ip_offset);
    inet_ntop(AF_INET, &(ip->dest), result, INET_ADDRSTRLEN);
    return 0;
}

int extract_ip_protocol(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int ip_offset = get_ip_header_offset(packet);
    if (ip_offset < 0) return -1;
    
    struct ip_header* ip = (struct ip_header*)(packet + ip_offset);
    *result = ip->protocol;
    return 0;
}

int extract_ip_ttl(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int ip_offset = get_ip_header_offset(packet);
    if (ip_offset < 0) return -1;
    
    struct ip_header* ip = (struct ip_header*)(packet + ip_offset);
    *result = ip->ttl;
    return 0;
}

int extract_ip_length(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int ip_offset = get_ip_header_offset(packet);
    if (ip_offset < 0) return -1;
    
    struct ip_header* ip = (struct ip_header*)(packet + ip_offset);
    *result = ntohs(ip->total_length);
    return 0;
}

int extract_tcp_src_port(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int tcp_offset = get_tcp_header_offset(packet);
    if (tcp_offset < 0) return -1;
    
    struct tcp_header* tcp = (struct tcp_header*)(packet + tcp_offset);
    *result = ntohs(tcp->source_port);
    return 0;
}

int extract_tcp_dst_port(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int tcp_offset = get_tcp_header_offset(packet);
    if (tcp_offset < 0) return -1;
    
    struct tcp_header* tcp = (struct tcp_header*)(packet + tcp_offset);
    *result = ntohs(tcp->dest_port);
    return 0;
}

int extract_tcp_seq(const u_char* packet, uint32_t* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int tcp_offset = get_tcp_header_offset(packet);
    if (tcp_offset < 0) return -1;
    
    struct tcp_header* tcp = (struct tcp_header*)(packet + tcp_offset);
    *result = ntohl(tcp->seq_num);
    return 0;
}

int extract_tcp_ack(const u_char* packet, uint32_t* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int tcp_offset = get_tcp_header_offset(packet);
    if (tcp_offset < 0) return -1;
    
    struct tcp_header* tcp = (struct tcp_header*)(packet + tcp_offset);
    *result = ntohl(tcp->ack_num);
    return 0;
}

int extract_tcp_window(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int tcp_offset = get_tcp_header_offset(packet);
    if (tcp_offset < 0) return -1;
    
    struct tcp_header* tcp = (struct tcp_header*)(packet + tcp_offset);
    *result = ntohs(tcp->window);
    return 0;
}

int extract_tcp_flags(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int tcp_offset = get_tcp_header_offset(packet);
    if (tcp_offset < 0) return -1;
    
    struct tcp_header* tcp = (struct tcp_header*)(packet + tcp_offset);
    *result = tcp->flags;
    return 0;
}

int extract_udp_src_port(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int udp_offset = get_udp_header_offset(packet);
    if (udp_offset < 0) return -1;
    
    struct udp_header* udp = (struct udp_header*)(packet + udp_offset);
    *result = ntohs(udp->source_port);
    return 0;
}

int extract_udp_dst_port(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int udp_offset = get_udp_header_offset(packet);
    if (udp_offset < 0) return -1;
    
    struct udp_header* udp = (struct udp_header*)(packet + udp_offset);
    *result = ntohs(udp->dest_port);
    return 0;
}

int extract_udp_length(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    int udp_offset = get_udp_header_offset(packet);
    if (udp_offset < 0) return -1;
    
    struct udp_header* udp = (struct udp_header*)(packet + udp_offset);
    *result = ntohs(udp->length);
    return 0;
}

int extract_http_request(const u_char* packet, HttpRequest* result) {
    if (!packet || !result) {
        return -1;
    }
    
    u_char* payload;
    int payload_len;
    if (get_packet_payload(packet, &payload, &payload_len) != 0) {
        return -1;
    }
    
    // Check if it's HTTP request
    if (strncmp((char*)payload, "GET ", 4) != 0 && 
        strncmp((char*)payload, "POST ", 5) != 0 &&
        strncmp((char*)payload, "PUT ", 4) != 0 &&
        strncmp((char*)payload, "DELETE ", 7) != 0 &&
        strncmp((char*)payload, "HEAD ", 5) != 0) {
        return -1;
    }
    
    // Parse method
    char* space = strchr((char*)payload, ' ');
    if (!space) return -1;
    
    int method_len = space - (char*)payload;
    result->method = malloc(method_len + 1);
    strncpy(result->method, (char*)payload, method_len);
    result->method[method_len] = '\0';
    
    // Parse URL
    char* url_start = space + 1;
    char* url_end = strchr(url_start, ' ');
    if (!url_end) return -1;
    
    int url_len = url_end - url_start;
    result->url = malloc(url_len + 1);
    strncpy(result->url, url_start, url_len);
    result->url[url_len] = '\0';
    
    // Parse headers
    result->host = parse_http_header((char*)payload, "Host");
    result->user_agent = parse_http_header((char*)payload, "User-Agent");
    result->cookies = parse_http_header((char*)payload, "Cookie");
    
    return 0;
}

int extract_http_response(const u_char* packet, HttpResponse* result) {
    if (!packet || !result) {
        return -1;
    }
    
    u_char* payload;
    int payload_len;
    if (get_packet_payload(packet, &payload, &payload_len) != 0) {
        return -1;
    }
    
    // Check if it's HTTP response
    if (strncmp((char*)payload, "HTTP/", 5) != 0) {
        return -1;
    }
    
    // Parse status code
    char* space = strchr((char*)payload, ' ');
    if (!space) return -1;
    
    char* status_start = space + 1;
    char* status_end = strchr(status_start, ' ');
    if (!status_end) return -1;
    
    int status_len = status_end - status_start;
    char status_str[4];
    strncpy(status_str, status_start, status_len);
    status_str[status_len] = '\0';
    result->status_code = atoi(status_str);
    
    // Parse headers
    result->content_type = parse_http_header((char*)payload, "Content-Type");
    result->server = parse_http_header((char*)payload, "Server");
    result->date = parse_http_header((char*)payload, "Date");
    
    // Parse content length
    char* content_length_str = parse_http_header((char*)payload, "Content-Length");
    if (content_length_str) {
        result->content_length = atoi(content_length_str);
        free(content_length_str);
    } else {
        result->content_length = -1;
    }
    
    return 0;
}

int extract_http_status_code(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    u_char* payload;
    int payload_len;
    if (get_packet_payload(packet, &payload, &payload_len) != 0) {
        return -1;
    }
    
    // Check if it's HTTP response
    if (strncmp((char*)payload, "HTTP/", 5) != 0) {
        return -1;
    }
    
    // Parse status code
    char* space = strchr((char*)payload, ' ');
    if (!space) return -1;
    
    char* status_start = space + 1;
    char* status_end = strchr(status_start, ' ');
    if (!status_end) return -1;
    
    int status_len = status_end - status_start;
    char status_str[4];
    strncpy(status_str, status_start, status_len);
    status_str[status_len] = '\0';
    *result = atoi(status_str);
    
    return 0;
}

int extract_http_method(const u_char* packet, char* result) {
    if (!packet || !result) {
        return -1;
    }
    
    u_char* payload;
    int payload_len;
    if (get_packet_payload(packet, &payload, &payload_len) != 0) {
        return -1;
    }
    
    // Check if it's HTTP request
    if (strncmp((char*)payload, "GET ", 4) != 0 && 
        strncmp((char*)payload, "POST ", 5) != 0 &&
        strncmp((char*)payload, "PUT ", 4) != 0 &&
        strncmp((char*)payload, "DELETE ", 7) != 0 &&
        strncmp((char*)payload, "HEAD ", 5) != 0) {
        return -1;
    }
    
    // Parse method
    char* space = strchr((char*)payload, ' ');
    if (!space) return -1;
    
    int method_len = space - (char*)payload;
    strncpy(result, (char*)payload, method_len);
    result[method_len] = '\0';
    
    return 0;
}

int extract_http_url(const u_char* packet, char* result) {
    if (!packet || !result) {
        return -1;
    }
    
    u_char* payload;
    int payload_len;
    if (get_packet_payload(packet, &payload, &payload_len) != 0) {
        return -1;
    }
    
    // Check if it's HTTP request
    if (strncmp((char*)payload, "GET ", 4) != 0 && 
        strncmp((char*)payload, "POST ", 5) != 0 &&
        strncmp((char*)payload, "PUT ", 4) != 0 &&
        strncmp((char*)payload, "DELETE ", 7) != 0 &&
        strncmp((char*)payload, "HEAD ", 5) != 0) {
        return -1;
    }
    
    // Parse URL
    char* space = strchr((char*)payload, ' ');
    if (!space) return -1;
    
    char* url_start = space + 1;
    char* url_end = strchr(url_start, ' ');
    if (!url_end) return -1;
    
    int url_len = url_end - url_start;
    strncpy(result, url_start, url_len);
    result[url_len] = '\0';
    
    return 0;
}

int extract_http_content_length(const u_char* packet, int* result) {
    if (!packet || !result) {
        return -1;
    }
    
    u_char* payload;
    int payload_len;
    if (get_packet_payload(packet, &payload, &payload_len) != 0) {
        return -1;
    }
    
    char* content_length_str = parse_http_header((char*)payload, "Content-Length");
    if (content_length_str) {
        *result = atoi(content_length_str);
        free(content_length_str);
        return 0;
    }
    
    return -1;
}

int extract_http_cookies(const u_char* packet, char* result) {
    if (!packet || !result) {
        return -1;
    }
    
    u_char* payload;
    int payload_len;
    if (get_packet_payload(packet, &payload, &payload_len) != 0) {
        return -1;
    }
    
    char* cookies = parse_http_header((char*)payload, "Cookie");
    if (cookies) {
        strcpy(result, cookies);
        free(cookies);
        return 0;
    }
    
    return -1;
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

int extract_packet_length(const struct pcap_pkthdr* header, int* result) {
    if (!header || !result) {
        return -1;
    }
    
    *result = header->len;
    return 0;
}

bool is_http_packet(const u_char* packet) {
    u_char* payload;
    int payload_len;
    if (get_packet_payload(packet, &payload, &payload_len) != 0) {
        return false;
    }
    
    return (strncmp((char*)payload, "GET ", 4) == 0 ||
            strncmp((char*)payload, "POST ", 5) == 0 ||
            strncmp((char*)payload, "PUT ", 4) == 0 ||
            strncmp((char*)payload, "DELETE ", 7) == 0 ||
            strncmp((char*)payload, "HEAD ", 5) == 0 ||
            strncmp((char*)payload, "HTTP/", 5) == 0);
}

bool is_tcp_packet(const u_char* packet) {
    int ip_offset = get_ip_header_offset(packet);
    if (ip_offset < 0) return false;
    
    struct ip_header* ip = (struct ip_header*)(packet + ip_offset);
    return (ip->protocol == 6);
}

bool is_udp_packet(const u_char* packet) {
    int ip_offset = get_ip_header_offset(packet);
    if (ip_offset < 0) return false;
    
    struct ip_header* ip = (struct ip_header*)(packet + ip_offset);
    return (ip->protocol == 17);
}

bool is_ipv4_packet(const u_char* packet) {
    return (get_ip_header_offset(packet) >= 0);
}

int get_packet_payload(const u_char* packet, u_char** payload, int* payload_len) {
    if (!packet || !payload || !payload_len) {
        return -1;
    }
    
    int ip_offset = get_ip_header_offset(packet);
    if (ip_offset < 0) return -1;
    
    struct ip_header* ip = (struct ip_header*)(packet + ip_offset);
    int ip_header_len = (ip->version_ihl & 0x0F) * 4;
    
    if (ip->protocol == 6) { // TCP
        struct tcp_header* tcp = (struct tcp_header*)(packet + ip_offset + ip_header_len);
        int tcp_header_len = ((tcp->data_offset >> 4) & 0x0F) * 4;
        *payload = (u_char*)(packet + ip_offset + ip_header_len + tcp_header_len);
        *payload_len = ntohs(ip->total_length) - ip_header_len - tcp_header_len;
    } else if (ip->protocol == 17) { // UDP
        *payload = (u_char*)(packet + ip_offset + ip_header_len + 8); // UDP header is 8 bytes
        *payload_len = ntohs(ip->total_length) - ip_header_len - 8;
    } else {
        return -1;
    }
    
    return 0;
} 