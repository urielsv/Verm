#ifndef VERM_PCAP_RUNTIME_H
#define VERM_PCAP_RUNTIME_H

#include <pcap.h>
#include <stdint.h>

/**
 * @brief Callback function type for packet processing
 */
typedef void (*packet_handler_t)(u_char* user, const struct pcap_pkthdr* header,
                               const u_char* packet);

/**
 * @brief Context structure for PCAP processing
 */
typedef struct {
    char* filename;           // PCAP file to process
    pcap_t* handle;          // PCAP handle
    struct bpf_program filter; // Compiled BPF filter
    char* filter_str;        // Original filter string
} PcapContext;

/**
 * @brief Initialize PCAP context
 * @param ctx Context to initialize
 * @param filename PCAP file to process
 * @param filter BPF filter string
 * @return 0 if successful, non-zero otherwise
 */
int init_pcap_context(PcapContext* ctx, char* filename, char* filter);

/**
 * @brief Process packets from PCAP file
 * @param ctx PCAP context
 * @param handler Packet processing callback
 * @return 0 if successful, non-zero otherwise
 */
int process_packets(PcapContext* ctx, packet_handler_t handler);

/**
 * @brief Extract field from packet
 * @param packet Raw packet data
 * @param protocol Protocol name
 * @param field Field name
 * @param result Pointer to store result
 * @return 0 if successful, non-zero otherwise
 */
int extract_field(u_char* packet, char* protocol, char* field, void* result);

/**
 * @brief Clean up PCAP context
 * @param ctx Context to clean up
 */
void cleanup_pcap_context(PcapContext* ctx);

#endif // VERM_PCAP_RUNTIME_H 