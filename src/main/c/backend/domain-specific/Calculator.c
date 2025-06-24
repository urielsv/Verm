#include "Calculator.h"
#include "../../shared/Logger.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void initializeCalculatorModule() {
	_logger = createLogger("Calculator");
}

void shutdownCalculatorModule() {
	if (_logger != NULL) {
		destroyLogger(_logger);
	}
}

/** PRIVATE FUNCTIONS */

static BinaryOperator _expressionTypeToBinaryOperator(const ExpressionType type);
static ComputationResult _invalidBinaryOperator(const int x, const int y);
static ComputationResult _invalidComputation();

/**
 * Converts and expression type to the proper binary operator. If that's not
 * possible, returns a binary operator that always returns an invalid
 * computation result.
 */
static BinaryOperator _expressionTypeToBinaryOperator(const ExpressionType type) {
	switch (type) {
		case ADDITION: return add;
		case DIVISION: return divide;
		case MULTIPLICATION: return multiply;
		case SUBTRACTION: return subtract;
		default:
			logError(_logger, "The specified expression type cannot be converted into character: %d", type);
			return _invalidBinaryOperator;
	}
}

/**
 * A binary operator that always returns an invalid computation result.
 */
static ComputationResult _invalidBinaryOperator(const int x, const int y) {
	return _invalidComputation();
}

/**
 * A computation that always returns an invalid result.
 */
static ComputationResult _invalidComputation() {
	ComputationResult computationResult = {
		.succeed = false,
		.value = 0
	};
	return computationResult;
}

/** PUBLIC FUNCTIONS */

ComputationResult add(const int leftAddend, const int rightAddend) {
	ComputationResult computationResult = {
		.succeed = true,
		.value = leftAddend + rightAddend
	};
	return computationResult;
}

ComputationResult divide(const int dividend, const int divisor) {
	const int sign = dividend < 0 ? -1 : +1;
	const boolean divisionByZero = divisor == 0 ? true : false;
	if (divisionByZero) {
		logError(_logger, "The divisor cannot be zero (the computation was %d/%d).", dividend, divisor);
	}
	ComputationResult computationResult = {
		.succeed = divisionByZero ? false : true,
		.value = divisionByZero ? (sign * INT_MAX) : (dividend / divisor)
	};
	return computationResult;
}

ComputationResult multiply(const int multiplicand, const int multiplier) {
	ComputationResult computationResult = {
		.succeed = true,
		.value = multiplicand * multiplier
	};
	return computationResult;
}

ComputationResult subtract(const int minuend, const int subtract) {
	ComputationResult computationResult = {
		.succeed = true,
		.value = minuend - subtract
	};
	return computationResult;
}

ComputationResult computeConstant(Constant * constant) {
	ComputationResult computationResult = {
		.succeed = true,
		.value = constant->value.integer
	};
	return computationResult;
}

ComputationResult computeStatementList(StatementList * statementList){
	//todo stage 3
	ComputationResult computationResult = {
		.succeed = true,
		.value = 0
	};
	return computationResult;
}


ComputationResult computeStatement(Statement * statement) {
	//todo_ stage 3 
	ComputationResult computationResult = {
		.succeed = true,
		.value = 0
	};
	return computationResult;
}

ComputationResult computeExpression(Expression * expression) {
	switch (expression->type) {
		case ADDITION:
		case DIVISION:
		case MULTIPLICATION:
		case SUBTRACTION: {
			ComputationResult leftResult = computeExpression(expression->leftExpression);
			ComputationResult rightResult = computeExpression(expression->rightExpression);
			if (leftResult.succeed && rightResult.succeed) {
				BinaryOperator binaryOperator = _expressionTypeToBinaryOperator(expression->type);
				return binaryOperator(leftResult.value, rightResult.value);
			}
			else {
				return _invalidComputation();
			}
		}
		case FACTOR:
			return computeFactor(expression->factor);
		default:
			return _invalidComputation();
	}
}

ComputationResult computeFactor(Factor * factor) {
	switch (factor->type) {
		case CONSTANT:
			return computeConstant(factor->constant);
		case EXPRESSION:
			return computeExpression(factor->expression);
		default:
			return _invalidComputation();
	}
}

// Comparison function for qsort
static int compare_doubles(const void* a, const void* b) {
	double da = *(const double*)a;
	double db = *(const double*)b;
	return (da > db) - (da < db);
}

// Comparison function for integers
static int compare_ints(const void* a, const void* b) {
	return (*(const int*)a - *(const int*)b);
}

StatisticalSummary calculate_statistics(double* values, int count) {
	StatisticalSummary summary = {0};
	
	if (!values || count <= 0) {
		return summary;
	}
	
	// Calculate mean
	double sum = 0;
	for (int i = 0; i < count; i++) {
		sum += values[i];
	}
	summary.mean = sum / count;
	
	// Calculate min, max
	summary.min = values[0];
	summary.max = values[0];
	for (int i = 1; i < count; i++) {
		if (values[i] < summary.min) summary.min = values[i];
		if (values[i] > summary.max) summary.max = values[i];
	}
	
	// Calculate standard deviation
	double variance = 0;
	for (int i = 0; i < count; i++) {
		double diff = values[i] - summary.mean;
		variance += diff * diff;
	}
	summary.std_dev = sqrt(variance / count);
	
	// Calculate median
	double* sorted_values = malloc(count * sizeof(double));
	memcpy(sorted_values, values, count * sizeof(double));
	qsort(sorted_values, count, sizeof(double), compare_doubles);
	
	if (count % 2 == 0) {
		summary.median = (sorted_values[count/2 - 1] + sorted_values[count/2]) / 2;
	} else {
		summary.median = sorted_values[count/2];
	}
	
	summary.count = count;
	free(sorted_values);
	
	return summary;
}

int detect_port_scanning(const char* src_ip, int* dst_ports, int port_count, int time_window) {
	if (!src_ip || !dst_ports || port_count <= 0) {
		return 0;
	}
	
	// Sort ports to find ranges
	int* sorted_ports = malloc(port_count * sizeof(int));
	memcpy(sorted_ports, dst_ports, port_count * sizeof(int));
	qsort(sorted_ports, port_count, sizeof(int), compare_ints);
	
	// Check for sequential port scanning
	int sequential_count = 1;
	int max_sequential = 1;
	
	for (int i = 1; i < port_count; i++) {
		if (sorted_ports[i] == sorted_ports[i-1] + 1) {
			sequential_count++;
			if (sequential_count > max_sequential) {
				max_sequential = sequential_count;
			}
		} else {
			sequential_count = 1;
		}
	}
	
	// Check for common ports scanning
	int common_ports[] = {21, 22, 23, 25, 53, 80, 110, 143, 443, 993, 995};
	int common_port_count = sizeof(common_ports) / sizeof(common_ports[0]);
	int common_ports_hit = 0;
	
	for (int i = 0; i < port_count; i++) {
		for (int j = 0; j < common_port_count; j++) {
			if (sorted_ports[i] == common_ports[j]) {
				common_ports_hit++;
				break;
			}
		}
	}
	
	free(sorted_ports);
	
	// Detection criteria
	if (max_sequential >= 10 || common_ports_hit >= 5 || port_count >= 20) {
		logInformation(NULL, "Port scanning detected from %s: %d ports, %d sequential, %d common ports", 
					  src_ip, port_count, max_sequential, common_ports_hit);
		return 1;
	}
	
	return 0;
}

int detect_ddos_attack(const char** src_ips, int* packet_counts, int ip_count, int time_window) {
	if (!src_ips || !packet_counts || ip_count <= 0) {
		return 0;
	}
	
	// Calculate total packets and average
	int total_packets = 0;
	for (int i = 0; i < ip_count; i++) {
		total_packets += packet_counts[i];
	}
	
	double avg_packets_per_ip = (double)total_packets / ip_count;
	
	// Check for high packet rate
	double packets_per_second = (double)total_packets / time_window;
	
	// Check for many unique sources
	int unique_sources = 0;
	for (int i = 0; i < ip_count; i++) {
		if (packet_counts[i] > 0) {
			unique_sources++;
		}
	}
	
	// Detection criteria
	if (packets_per_second > 1000 || unique_sources > 100 || avg_packets_per_ip > 100) {
		logInformation(NULL, "DDoS attack detected: %.2f packets/sec, %d unique sources, %.2f avg packets/IP", 
					  packets_per_second, unique_sources, avg_packets_per_ip);
		return 1;
	}
	
	return 0;
}

int detect_slow_http_attack(time_t* request_times, int request_count, int time_window) {
	if (!request_times || request_count <= 0) {
		return 0;
	}
	
	// Calculate request intervals
	double* intervals = malloc((request_count - 1) * sizeof(double));
	for (int i = 1; i < request_count; i++) {
		intervals[i-1] = difftime(request_times[i], request_times[i-1]);
	}
	
	// Check for slow request patterns
	int slow_requests = 0;
	for (int i = 0; i < request_count - 1; i++) {
		if (intervals[i] > 10.0) { // More than 10 seconds between requests
			slow_requests++;
		}
	}
	
	free(intervals);
	
	if (slow_requests > request_count * 0.3) { // More than 30% are slow
		logInformation(NULL, "Slow HTTP attack detected: %d/%d slow requests", slow_requests, request_count);
		return 1;
	}
	
	return 0;
}

double calculate_latency(time_t request_time, time_t response_time) {
	return difftime(response_time, request_time) * 1000.0; // Convert to milliseconds
}

AnomalyPattern* analyze_http_patterns(int* status_codes, double* response_times, int count) {
	if (!status_codes || !response_times || count <= 0) {
		return NULL;
	}
	
	// Calculate error rate
	int error_count = 0;
	for (int i = 0; i < count; i++) {
		if (status_codes[i] >= 400) {
			error_count++;
		}
	}
	
	double error_rate = (double)error_count / count;
	
	// Calculate response time statistics
	StatisticalSummary response_stats = calculate_statistics(response_times, count);
	
	AnomalyPattern* pattern = NULL;
	
	// Check for high error rate
	if (error_rate > 0.1) { // More than 10% errors
		pattern = malloc(sizeof(AnomalyPattern));
		pattern->pattern_name = "High HTTP Error Rate";
		pattern->severity = 2;
		pattern->description = "High rate of HTTP error responses detected";
		pattern->recommendation = "Investigate server issues or potential attacks";
		return pattern;
	}
	
	// Check for slow response times
	if (response_stats.mean > 5000.0) { // Average response time > 5 seconds
		pattern = malloc(sizeof(AnomalyPattern));
		pattern->pattern_name = "Slow HTTP Responses";
		pattern->severity = 1;
		pattern->description = "Unusually slow HTTP response times detected";
		pattern->recommendation = "Check server performance and network connectivity";
		return pattern;
	}
	
	return NULL;
}

int detect_data_exfiltration(int* packet_sizes, int packet_count, int time_window) {
	if (!packet_sizes || packet_count <= 0) {
		return 0;
	}
	
	// Calculate total data transferred
	int total_data = 0;
	for (int i = 0; i < packet_count; i++) {
		total_data += packet_sizes[i];
	}
	
	// Calculate data transfer rate
	double data_rate = (double)total_data / time_window; // bytes per second
	
	// Check for large data transfers
	if (data_rate > 1000000) { // More than 1MB/s
		logInformation(NULL, "Potential data exfiltration detected: %.2f bytes/sec", data_rate);
		return 1;
	}
	
	// Check for large individual packets
	int large_packets = 0;
	for (int i = 0; i < packet_count; i++) {
		if (packet_sizes[i] > 1400) { // Large packets
			large_packets++;
		}
	}
	
	if (large_packets > packet_count * 0.5) { // More than 50% are large
		logInformation(NULL, "Large packet exfiltration detected: %d/%d large packets", large_packets, packet_count);
		return 1;
	}
	
	return 0;
}

StatisticalSummary analyze_protocol_distribution(const char** protocols, int* counts, int protocol_count) {
	if (!protocols || !counts || protocol_count <= 0) {
		StatisticalSummary empty = {0};
		return empty;
	}
	
	// Convert counts to doubles for statistical analysis
	double* values = malloc(protocol_count * sizeof(double));
	for (int i = 0; i < protocol_count; i++) {
		values[i] = (double)counts[i];
	}
	
	StatisticalSummary summary = calculate_statistics(values, protocol_count);
	free(values);
	
	return summary;
}

int detect_periodic_patterns(time_t* timestamps, int packet_count, int min_period, int max_period) {
	if (!timestamps || packet_count < 3) {
		return 0;
	}
	
	// Calculate intervals between packets
	double* intervals = malloc((packet_count - 1) * sizeof(double));
	for (int i = 1; i < packet_count; i++) {
		intervals[i-1] = difftime(timestamps[i], timestamps[i-1]);
	}
	
	// Check for consistent intervals
	StatisticalSummary interval_stats = calculate_statistics(intervals, packet_count - 1);
	
	free(intervals);
	
	// Check if intervals are within the expected range and consistent
	if (interval_stats.mean >= min_period && interval_stats.mean <= max_period && 
		interval_stats.std_dev < interval_stats.mean * 0.1) { // Low variance
		logInformation(NULL, "Periodic pattern detected: mean interval %.2f seconds, std dev %.2f", 
					  interval_stats.mean, interval_stats.std_dev);
		return 1;
	}
	
	return 0;
}

double calculate_bandwidth(int* packet_sizes, time_t* timestamps, int packet_count, int time_window) {
	if (!packet_sizes || !timestamps || packet_count <= 0) {
		return 0.0;
	}
	
	// Calculate total data transferred
	int total_data = 0;
	for (int i = 0; i < packet_count; i++) {
		total_data += packet_sizes[i];
	}
	
	// Calculate bandwidth in bytes per second
	return (double)total_data / time_window;
}

AnomalyPattern* analyze_tcp_patterns(ConnectionProfile* connections, int connection_count) {
	if (!connections || connection_count <= 0) {
		return NULL;
	}
	
	// Check for connection flooding
	int high_connection_count = 0;
	for (int i = 0; i < connection_count; i++) {
		if (connections[i].packet_count > 1000) {
			high_connection_count++;
		}
	}
	
	if (high_connection_count > connection_count * 0.2) { // More than 20% have high packet counts
		AnomalyPattern* pattern = malloc(sizeof(AnomalyPattern));
		pattern->pattern_name = "TCP Connection Flooding";
		pattern->severity = 2;
		pattern->description = "High number of packets per connection detected";
		pattern->recommendation = "Investigate for potential DoS attacks";
		return pattern;
	}
	
	return NULL;
}

int detect_syn_flood(time_t* syn_packets, int packet_count, int time_window) {
	if (!syn_packets || packet_count <= 0) {
		return 0;
	}
	
	// Calculate SYN packet rate
	double syn_rate = (double)packet_count / time_window;
	
	// Check for high SYN rate without corresponding ACKs
	if (syn_rate > 100) { // More than 100 SYN packets per second
		logInformation(NULL, "SYN flood detected: %.2f SYN packets/sec", syn_rate);
		return 1;
	}
	
	return 0;
}

AnomalyPattern* analyze_host_behavior(HostProfile* hosts, int host_count, int* anomaly_count) {
	if (!hosts || host_count <= 0 || !anomaly_count) {
		return NULL;
	}
	
	AnomalyPattern* patterns = malloc(host_count * sizeof(AnomalyPattern));
	*anomaly_count = 0;
	
	for (int i = 0; i < host_count; i++) {
		// Check for suspicious host behavior
		if (hosts[i].port_scan_count > 10) {
			patterns[*anomaly_count].pattern_name = "Port Scanning Host";
			patterns[*anomaly_count].severity = 2;
			patterns[*anomaly_count].description = "Host performing port scanning";
			patterns[*anomaly_count].recommendation = "Investigate host for potential reconnaissance";
			(*anomaly_count)++;
		}
		
		if (hosts[i].failed_connections > hosts[i].packet_count * 0.5) {
			patterns[*anomaly_count].pattern_name = "High Connection Failure Rate";
			patterns[*anomaly_count].severity = 1;
			patterns[*anomaly_count].description = "Host with high connection failure rate";
			patterns[*anomaly_count].recommendation = "Check host connectivity and target availability";
			(*anomaly_count)++;
		}
	}
	
	return patterns;
}

StatisticalSummary analyze_packet_sizes(int* packet_sizes, int packet_count) {
	if (!packet_sizes || packet_count <= 0) {
		StatisticalSummary empty = {0};
		return empty;
	}
	
	// Convert to doubles for statistical analysis
	double* values = malloc(packet_count * sizeof(double));
	for (int i = 0; i < packet_count; i++) {
		values[i] = (double)packet_sizes[i];
	}
	
	StatisticalSummary summary = calculate_statistics(values, packet_count);
	free(values);
	
	return summary;
}

int detect_unusual_traffic(time_t* timestamps, int* packet_sizes, int packet_count, 
						  double baseline_mean, double baseline_std) {
	if (!timestamps || !packet_sizes || packet_count <= 0) {
		return 0;
	}
	
	// Calculate current packet size statistics
	StatisticalSummary current_stats = analyze_packet_sizes(packet_sizes, packet_count);
	
	// Check if current mean is significantly different from baseline
	double z_score = fabs(current_stats.mean - baseline_mean) / baseline_std;
	
	if (z_score > 2.0) { // More than 2 standard deviations from baseline
		logInformation(NULL, "Unusual traffic pattern detected: z-score %.2f", z_score);
		return 1;
	}
	
	return 0;
}

void free_anomaly_pattern(AnomalyPattern* pattern) {
	if (pattern) {
		free(pattern);
	}
}

void free_anomaly_patterns(AnomalyPattern* patterns, int count) {
	if (patterns) {
		for (int i = 0; i < count; i++) {
			// Free any dynamically allocated strings if needed
		}
		free(patterns);
	}
}
