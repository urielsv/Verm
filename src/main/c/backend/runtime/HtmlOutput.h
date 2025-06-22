#ifndef VERM_HTML_OUTPUT_H
#define VERM_HTML_OUTPUT_H

#include <stdio.h>

/**
 * @brief Initialize HTML dashboard output
 * @param filename Output HTML file name
 * @param title Dashboard title
 * @return 0 if successful, non-zero otherwise
 */
int html_init_dashboard(const char* filename, const char* title);

/**
 * @brief Add a metric card to the dashboard
 * @param title Metric title
 * @param value Metric value
 * @param description Metric description
 * @param color CSS color class (e.g., "success", "warning", "danger", "info")
 */
void html_add_metric(const char* title, const char* value, const char* description, const char* color);

/**
 * @brief Add a data table to the dashboard
 * @param title Table title
 * @param headers Array of column headers
 * @param num_headers Number of headers
 */
void html_start_table(const char* title, const char** headers, int num_headers);

/**
 * @brief Add a row to the current table
 * @param values Array of cell values
 * @param num_values Number of values
 */
void html_add_table_row(const char** values, int num_values);

/**
 * @brief End the current table
 */
void html_end_table(void);

/**
 * @brief Close HTML dashboard
 */
void html_close_dashboard(void);

#endif // VERM_HTML_OUTPUT_H 