#ifndef VERM_CSV_OUTPUT_H
#define VERM_CSV_OUTPUT_H

#include <stdio.h>

/**
 * @brief Initialize CSV output with headers
 * @param filename Output CSV file name
 * @param headers Array of header strings
 * @param num_headers Number of headers
 * @return 0 if successful, non-zero otherwise
 */
int csv_init_output(const char* filename, const char** headers, int num_headers);

/**
 * @brief Write a row of data to CSV
 * @param values Array of string values
 * @param num_values Number of values
 * @return 0 if successful, non-zero otherwise
 */
int csv_write_row(const char** values, int num_values);

/**
 * @brief Close CSV output file
 */
void csv_close_output(void);

/**
 * @brief Write a single field to CSV (for building rows)
 * @param value String value to write
 */
void csv_write_field(const char* value);

/**
 * @brief End current row in CSV
 */
void csv_end_row(void);

#endif // VERM_CSV_OUTPUT_H 