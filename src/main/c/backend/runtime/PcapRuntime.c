#include "PcapRuntime.h"
#include "../../shared/Logger.h"
#include <string.h>
#include <stdlib.h>

int init_pcap_context(PcapContext* ctx, char* filename, char* filter) {
    logDebugging(NULL, "init_pcap_context: called with ctx=%p, filename=%s, filter=%s", ctx, filename, filter ? filter : "(null)");
    if (!ctx || !filename) {
        logError(NULL, "Invalid parameters for PCAP context initialization");
        return -1;
    }
    
    logInformation(NULL, "Initializing PCAP context for file: %s", filename);
    
    // Initialize context
    ctx->filename = strdup(filename);
    ctx->handle = NULL;
    ctx->filter_str = filter ? strdup(filter) : NULL;
    memset(&ctx->filter, 0, sizeof(ctx->filter));
    logDebugging(NULL, "init_pcap_context: context initialized, about to open file");
    
    // Open PCAP file
    char errbuf[PCAP_ERRBUF_SIZE];
    ctx->handle = pcap_open_offline(filename, errbuf);
    logDebugging(NULL, "init_pcap_context: after pcap_open_offline, handle=%p", ctx->handle);
    if (!ctx->handle) {
        logError(NULL, "Failed to open PCAP file: %s", errbuf);
        return -1;
    }
    
    // Compile filter if provided
    if (filter && strlen(filter) > 0) {
        logDebugging(NULL, "init_pcap_context: compiling filter: %s", filter);
        if (pcap_compile(ctx->handle, &ctx->filter, filter, 0, PCAP_NETMASK_UNKNOWN) != 0) {
            logError(NULL, "Failed to compile BPF filter: %s", pcap_geterr(ctx->handle));
            return -1;
        }
        
        if (pcap_setfilter(ctx->handle, &ctx->filter) != 0) {
            logError(NULL, "Failed to set BPF filter: %s", pcap_geterr(ctx->handle));
            return -1;
        }
    }
    
    logInformation(NULL, "PCAP context initialized successfully");
    return 0;
}

int process_packets(PcapContext* ctx, packet_handler_t handler) {
    if (!ctx || !ctx->handle || !handler) {
        logError(NULL, "Invalid parameters for packet processing");
        return -1;
    }
    
    logInformation(NULL, "Starting packet processing");
    
    int packet_count = 0;
    struct pcap_pkthdr header;
    const u_char* packet;
    
    while ((packet = pcap_next(ctx->handle, &header)) != NULL) {
        handler(NULL, &header, packet);
        packet_count++;
    }
    
    logInformation(NULL, "Processed %d packets", packet_count);
    return 0;
}

int extract_field(u_char* packet, char* protocol, char* field, void* result) {
    if (!packet || !protocol || !field || !result) {
        logError(NULL, "Invalid parameters for field extraction");
        return -1;
    }
    
    logDebugging(NULL, "Extracting field %s from protocol %s", field, protocol);
    
    // Basic field extraction - for now just return success
    // TODO: Implement actual protocol parsing
    *(int*)result = 0;
    
    return 0;
}

void cleanup_pcap_context(PcapContext* ctx) {
    if (!ctx) {
        return;
    }
    
    if (ctx->handle) {
        pcap_close(ctx->handle);
        ctx->handle = NULL;
    }
    
    if (ctx->filename) {
        free(ctx->filename);
        ctx->filename = NULL;
    }
    
    if (ctx->filter_str) {
        free(ctx->filter_str);
        ctx->filter_str = NULL;
    }
    
    logInformation(NULL, "PCAP context cleaned up");
} 