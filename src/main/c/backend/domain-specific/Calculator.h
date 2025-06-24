#ifndef VERM_CALCULATOR_H
#define VERM_CALCULATOR_H

/**
 * We reuse the types from the AST for convenience, but you should separate
 * the layers of the backend and frontend using another group of
 * domain-specific models or DTOs (Data Transfer Objects).
 */
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../shared/Logger.h"
#include "../../shared/Type.h"
#include <limits.h>
#include <stdint.h>

/** Initialize module's internal state. */
void initializeCalculatorModule();

/** Shutdown module's internal state. */
void shutdownCalculatorModule();

/**
 * The result of a computation. It's considered valid only if "succeed" is
 * true.
 */
typedef struct {
	boolean succeed;
	int value;
} ComputationResult;

typedef ComputationResult (*BinaryOperator)(const int, const int);

/** Arithmetic operations. */

ComputationResult add(const int leftAddend, const int rightAddend);
ComputationResult divide(const int dividend, const int divisor);
ComputationResult multiply(const int multiplicand, const int multiplier);
ComputationResult subtract(const int minuend, const int subtract);

/**
 * Computes the final value of a mathematical constant.
 */
ComputationResult computeConstant(Constant * constant);

/**
 * Computes the final value of a mathematical expression.
 */
ComputationResult computeExpression(Expression * expression);

/**
 * Computes the final value of a mathematical factor.
 */
ComputationResult computeFactor(Factor * factor);

/**
* Computes the final value of a statement.
*/
ComputationResult computeStatement(Statement * statement);

/**
* Computes the final value of a list of statements.
*/
ComputationResult computeStatementList(StatementList * statementList);

// Pattern detection structures
typedef struct {
    char* ip_address;
    int packet_count;
    int port_scan_count;
    int failed_connections;
    double avg_packet_size;
    time_t first_seen;
    time_t last_seen;
} HostProfile;

typedef struct {
    char* src_ip;
    char* dst_ip;
    int src_port;
    int dst_port;
    int packet_count;
    double avg_latency;
    time_t first_seen;
    time_t last_seen;
} ConnectionProfile;

typedef struct {
    char* url;
    int request_count;
    int error_count;
    double avg_response_time;
    int max_content_length;
} HttpEndpointProfile;

typedef struct {
    char* pattern_name;
    int severity;
    char* description;
    char* recommendation;
} AnomalyPattern;

// Statistical analysis structures
typedef struct {
    double mean;
    double median;
    double std_dev;
    double min;
    double max;
    int count;
} StatisticalSummary;

/**
 * @brief Calculate statistical summary for a dataset
 * @param values Array of values
 * @param count Number of values
 * @return Statistical summary
 */
StatisticalSummary calculate_statistics(double* values, int count);

/**
 * @brief Detect port scanning behavior
 * @param src_ip Source IP address
 * @param dst_ports Array of destination ports
 * @param port_count Number of ports
 * @param time_window Time window in seconds
 * @return 1 if port scanning detected, 0 otherwise
 */
int detect_port_scanning(const char* src_ip, int* dst_ports, int port_count, int time_window);

/**
 * @brief Detect DDoS attack patterns
 * @param src_ips Array of source IPs
 * @param packet_counts Array of packet counts
 * @param ip_count Number of IPs
 * @param time_window Time window in seconds
 * @return 1 if DDoS detected, 0 otherwise
 */
int detect_ddos_attack(const char** src_ips, int* packet_counts, int ip_count, int time_window);

/**
 * @brief Detect slow HTTP attacks
 * @param request_times Array of request timestamps
 * @param request_count Number of requests
 * @param time_window Time window in seconds
 * @return 1 if slow HTTP attack detected, 0 otherwise
 */
int detect_slow_http_attack(time_t* request_times, int request_count, int time_window);

/**
 * @brief Calculate connection latency
 * @param request_time Request timestamp
 * @param response_time Response timestamp
 * @return Latency in milliseconds
 */
double calculate_latency(time_t request_time, time_t response_time);

/**
 * @brief Analyze HTTP response patterns
 * @param status_codes Array of HTTP status codes
 * @param response_times Array of response times
 * @param count Number of responses
 * @return Anomaly pattern if detected, NULL otherwise
 */
AnomalyPattern* analyze_http_patterns(int* status_codes, double* response_times, int count);

/**
 * @brief Detect data exfiltration
 * @param packet_sizes Array of packet sizes
 * @param packet_count Number of packets
 * @param time_window Time window in seconds
 * @return 1 if data exfiltration detected, 0 otherwise
 */
int detect_data_exfiltration(int* packet_sizes, int packet_count, int time_window);

/**
 * @brief Analyze protocol distribution
 * @param protocols Array of protocol identifiers
 * @param counts Array of packet counts
 * @param protocol_count Number of protocols
 * @return Statistical summary of protocol distribution
 */
StatisticalSummary analyze_protocol_distribution(const char** protocols, int* counts, int protocol_count);

/**
 * @brief Detect periodic communication patterns
 * @param timestamps Array of packet timestamps
 * @param packet_count Number of packets
 * @param min_period Minimum period in seconds
 * @param max_period Maximum period in seconds
 * @return 1 if periodic pattern detected, 0 otherwise
 */
int detect_periodic_patterns(time_t* timestamps, int packet_count, int min_period, int max_period);

/**
 * @brief Calculate bandwidth usage
 * @param packet_sizes Array of packet sizes
 * @param timestamps Array of packet timestamps
 * @param packet_count Number of packets
 * @param time_window Time window in seconds
 * @return Bandwidth in bytes per second
 */
double calculate_bandwidth(int* packet_sizes, time_t* timestamps, int packet_count, int time_window);

/**
 * @brief Analyze TCP connection patterns
 * @param connections Array of connection profiles
 * @param connection_count Number of connections
 * @return Anomaly pattern if detected, NULL otherwise
 */
AnomalyPattern* analyze_tcp_patterns(ConnectionProfile* connections, int connection_count);

/**
 * @brief Detect SYN flood attack
 * @param syn_packets Array of SYN packet timestamps
 * @param packet_count Number of SYN packets
 * @param time_window Time window in seconds
 * @return 1 if SYN flood detected, 0 otherwise
 */
int detect_syn_flood(time_t* syn_packets, int packet_count, int time_window);

/**
 * @brief Analyze host behavior patterns
 * @param hosts Array of host profiles
 * @param host_count Number of hosts
 * @return Array of anomaly patterns
 */
AnomalyPattern* analyze_host_behavior(HostProfile* hosts, int host_count, int* anomaly_count);

/**
 * @brief Calculate packet size distribution
 * @param packet_sizes Array of packet sizes
 * @param packet_count Number of packets
 * @return Statistical summary of packet sizes
 */
StatisticalSummary analyze_packet_sizes(int* packet_sizes, int packet_count);

/**
 * @brief Detect unusual traffic patterns
 * @param timestamps Array of packet timestamps
 * @param packet_sizes Array of packet sizes
 * @param packet_count Number of packets
 * @param baseline_mean Baseline mean packet size
 * @param baseline_std Baseline standard deviation
 * @return 1 if unusual pattern detected, 0 otherwise
 */
int detect_unusual_traffic(time_t* timestamps, int* packet_sizes, int packet_count, 
                          double baseline_mean, double baseline_std);

/**
 * @brief Free anomaly pattern memory
 * @param pattern Pattern to free
 */
void free_anomaly_pattern(AnomalyPattern* pattern);

/**
 * @brief Free array of anomaly patterns
 * @param patterns Array of patterns
 * @param count Number of patterns
 */
void free_anomaly_patterns(AnomalyPattern* patterns, int count);

#endif
