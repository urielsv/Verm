#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif
#include "../../shared/Type.h"

#ifndef VERM_PROTOCOL_EXTRACTORS_H
#define VERM_PROTOCOL_EXTRACTORS_H

#include <pcap.h>
#include <stdint.h>
#include <stdbool.h>

// HTTP response structure
typedef struct {
    int status_code;
    char* content_type;
    int content_length;
    char* server;
    char* date;
} HttpResponse;

// HTTP request structure
typedef struct {
    char* method;
    char* url;
    char* host;
    char* user_agent;
    char* cookies;
} HttpRequest;

/**
 * @brief Extract source IP address from packet
 * @param packet Raw packet data
 * @param result Buffer to store IP address string
 * @return 0 if successful, non-zero otherwise
 */
int extract_ip_src_addr(const u_char* packet, char* result);

/**
 * @brief Extract destination IP address from packet
 * @param packet Raw packet data
 * @param result Buffer to store IP address string
 * @return 0 if successful, non-zero otherwise
 */
int extract_ip_dst_addr(const u_char* packet, char* result);

/**
 * @brief Extract IP protocol number
 * @param packet Raw packet data
 * @param result Pointer to store protocol number
 * @return 0 if successful, non-zero otherwise
 */
int extract_ip_protocol(const u_char* packet, int* result);

/**
 * @brief Extract IP TTL value
 * @param packet Raw packet data
 * @param result Pointer to store TTL value
 * @return 0 if successful, non-zero otherwise
 */
int extract_ip_ttl(const u_char* packet, int* result);

/**
 * @brief Extract IP total length
 * @param packet Raw packet data
 * @param result Pointer to store total length
 * @return 0 if successful, non-zero otherwise
 */
int extract_ip_length(const u_char* packet, int* result);

/**
 * @brief Extract TCP source port from packet
 * @param packet Raw packet data
 * @param result Pointer to store port number
 * @return 0 if successful, non-zero otherwise
 */
int extract_tcp_src_port(const u_char* packet, int* result);

/**
 * @brief Extract TCP destination port from packet
 * @param packet Raw packet data
 * @param result Pointer to store port number
 * @return 0 if successful, non-zero otherwise
 */
int extract_tcp_dst_port(const u_char* packet, int* result);

/**
 * @brief Extract TCP sequence number
 * @param packet Raw packet data
 * @param result Pointer to store sequence number
 * @return 0 if successful, non-zero otherwise
 */
int extract_tcp_seq(const u_char* packet, uint32_t* result);

/**
 * @brief Extract TCP acknowledgment number
 * @param packet Raw packet data
 * @param result Pointer to store ack number
 * @return 0 if successful, non-zero otherwise
 */
int extract_tcp_ack(const u_char* packet, uint32_t* result);

/**
 * @brief Extract TCP window size
 * @param packet Raw packet data
 * @param result Pointer to store window size
 * @return 0 if successful, non-zero otherwise
 */
int extract_tcp_window(const u_char* packet, int* result);

/**
 * @brief Extract TCP flags
 * @param packet Raw packet data
 * @param result Pointer to store flags
 * @return 0 if successful, non-zero otherwise
 */
int extract_tcp_flags(const u_char* packet, int* result);

/**
 * @brief Extract UDP source port from packet
 * @param packet Raw packet data
 * @param result Pointer to store port number
 * @return 0 if successful, non-zero otherwise
 */
int extract_udp_src_port(const u_char* packet, int* result);

/**
 * @brief Extract UDP destination port from packet
 * @param packet Raw packet data
 * @param result Pointer to store port number
 * @return 0 if successful, non-zero otherwise
 */
int extract_udp_dst_port(const u_char* packet, int* result);

/**
 * @brief Extract UDP length
 * @param packet Raw packet data
 * @param result Pointer to store UDP length
 * @return 0 if successful, non-zero otherwise
 */
int extract_udp_length(const u_char* packet, int* result);

/**
 * @brief Extract HTTP request information
 * @param packet Raw packet data
 * @param result Pointer to store HTTP request structure
 * @return 0 if successful, non-zero otherwise
 */
int extract_http_request(const u_char* packet, HttpRequest* result);

/**
 * @brief Extract HTTP response information
 * @param packet Raw packet data
 * @param result Pointer to store HTTP response structure
 * @return 0 if successful, non-zero otherwise
 */
int extract_http_response(const u_char* packet, HttpResponse* result);

/**
 * @brief Extract HTTP status code
 * @param packet Raw packet data
 * @param result Pointer to store status code
 * @return 0 if successful, non-zero otherwise
 */
int extract_http_status_code(const u_char* packet, int* result);

/**
 * @brief Extract HTTP method
 * @param packet Raw packet data
 * @param result Buffer to store HTTP method
 * @return 0 if successful, non-zero otherwise
 */
int extract_http_method(const u_char* packet, char* result);

/**
 * @brief Extract HTTP URL
 * @param packet Raw packet data
 * @param result Buffer to store HTTP URL
 * @return 0 if successful, non-zero otherwise
 */
int extract_http_url(const u_char* packet, char* result);

/**
 * @brief Extract HTTP content length
 * @param packet Raw packet data
 * @param result Pointer to store content length
 * @return 0 if successful, non-zero otherwise
 */
int extract_http_content_length(const u_char* packet, int* result);

/**
 * @brief Extract HTTP cookies
 * @param packet Raw packet data
 * @param result Buffer to store cookies
 * @return 0 if successful, non-zero otherwise
 */
int extract_http_cookies(const u_char* packet, char* result);

/**
 * @brief Extract timestamp from packet header
 * @param header Packet header
 * @param result Buffer to store timestamp string
 * @return 0 if successful, non-zero otherwise
 */
int extract_timestamp(const struct pcap_pkthdr* header, char* result);

/**
 * @brief Extract packet length
 * @param header Packet header
 * @param result Pointer to store packet length
 * @return 0 if successful, non-zero otherwise
 */
int extract_packet_length(const struct pcap_pkthdr* header, int* result);

/**
 * @brief Check if packet is HTTP
 * @param packet Raw packet data
 * @return true if packet is HTTP, false otherwise
 */
bool is_http_packet(const u_char* packet);

/**
 * @brief Check if packet is TCP
 * @param packet Raw packet data
 * @return true if packet is TCP, false otherwise
 */
bool is_tcp_packet(const u_char* packet);

/**
 * @brief Check if packet is UDP
 * @param packet Raw packet data
 * @return true if packet is UDP, false otherwise
 */
bool is_udp_packet(const u_char* packet);

/**
 * @brief Check if packet is IPv4
 * @param packet Raw packet data
 * @return true if packet is IPv4, false otherwise
 */
bool is_ipv4_packet(const u_char* packet);

/**
 * @brief Get payload data from packet
 * @param packet Raw packet data
 * @param payload Pointer to store payload data
 * @param payload_len Pointer to store payload length
 * @return 0 if successful, non-zero otherwise
 */
int get_packet_payload(const u_char* packet, u_char** payload, int* payload_len);

#endif // VERM_PROTOCOL_EXTRACTORS_H 