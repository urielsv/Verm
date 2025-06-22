#ifndef VERM_PROTOCOL_EXTRACTORS_H
#define VERM_PROTOCOL_EXTRACTORS_H

#include <pcap.h>

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
 * @brief Extract timestamp from packet header
 * @param header Packet header
 * @param result Buffer to store timestamp string
 * @return 0 if successful, non-zero otherwise
 */
int extract_timestamp(const struct pcap_pkthdr* header, char* result);

#endif // VERM_PROTOCOL_EXTRACTORS_H 