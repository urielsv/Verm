#ifndef VERM_HTML_OUTPUT_H
#define VERM_HTML_OUTPUT_H

#include <stdio.h>

/**
 * @brief Initialize HTML dashboard
 * @param filename Output HTML file
 * @param title Dashboard title
 * @return 0 if successful, non-zero otherwise
 */
int html_init_dashboard(const char* filename, const char* title);

/**
 * @brief Add a metric to the dashboard
 * @param name Metric name
 * @param value Metric value
 * @param description Metric description
 * @param type Metric type (info, success, warning, danger)
 * @return 0 if successful, non-zero otherwise
 */
int html_add_metric(const char* name, const char* value, const char* description, const char* type);

/**
 * @brief Update a metric value
 * @param name Metric name
 * @param value New metric value
 * @return 0 if successful, non-zero otherwise
 */
int html_update_metric(const char* name, const char* value);

/**
 * @brief Add a table to the dashboard
 * @param title Table title
 * @param headers Array of column headers
 * @param header_count Number of headers
 * @return 0 if successful, non-zero otherwise
 */
int html_add_table(const char* title, const char** headers, int header_count);

/**
 * @brief Add a row to the current table
 * @param values Array of cell values
 * @param value_count Number of values
 * @return 0 if successful, non-zero otherwise
 */
int html_add_table_row(const char** values, int value_count);

/**
 * @brief Add a chart to the dashboard
 * @param title Chart title
 * @param chart_type Chart type (line, bar, pie)
 * @param data JSON data for the chart
 * @return 0 if successful, non-zero otherwise
 */
int html_add_chart(const char* title, const char* chart_type, const char* data);

/**
 * @brief Add a section to the dashboard
 * @param title Section title
 * @param content Section content
 * @return 0 if successful, non-zero otherwise
 */
int html_add_section(const char* title, const char* content);

/**
 * @brief Add a filter summary
 * @param filter_string BPF filter string
 * @param packet_count Number of packets matching filter
 * @return 0 if successful, non-zero otherwise
 */
int html_add_filter_summary(const char* filter_string, int packet_count);

/**
 * @brief Add protocol statistics
 * @param protocol Protocol name
 * @param count Packet count
 * @param percentage Percentage of total packets
 * @return 0 if successful, non-zero otherwise
 */
int html_add_protocol_stats(const char* protocol, int count, double percentage);

/**
 * @brief Add top IP addresses
 * @param ips Array of IP addresses
 * @param counts Array of packet counts
 * @param count Number of entries
 * @return 0 if successful, non-zero otherwise
 */
int html_add_top_ips(const char** ips, int* counts, int count);

/**
 * @brief Add top ports
 * @param ports Array of port numbers
 * @param counts Array of packet counts
 * @param count Number of entries
 * @return 0 if successful, non-zero otherwise
 */
int html_add_top_ports(const int* ports, int* counts, int count);

/**
 * @brief Add HTTP statistics
 * @param status_codes Array of status codes
 * @param counts Array of counts
 * @param count Number of entries
 * @return 0 if successful, non-zero otherwise
 */
int html_add_http_stats(const int* status_codes, int* counts, int count);

/**
 * @brief Add alert section
 * @param title Alert title
 * @param message Alert message
 * @param severity Alert severity (info, warning, error, critical)
 * @return 0 if successful, non-zero otherwise
 */
int html_add_alert(const char* title, const char* message, const char* severity);

/**
 * @brief Close the HTML dashboard
 * @return 0 if successful, non-zero otherwise
 */
int html_close_dashboard();

/**
 * @brief Add a query section
 * @param dsl_query DSL query
 * @return 0 if successful, non-zero otherwise
 */
int html_add_query_section(const char* dsl_query);

/**
 * @brief Add a simple table
 * @param headers Array of column headers
 * @param header_count Number of headers
 * @param rows Array of row arrays
 * @param row_count Number of rows
 * @return 0 if successful, non-zero otherwise
 */
int html_add_simple_table(const char** headers, int header_count, const char*** rows, int row_count);

#endif // VERM_HTML_OUTPUT_H 