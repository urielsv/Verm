#include "CsvOutput.h"
#include <string.h>
#include <stdlib.h>

static FILE* csv_file = NULL;
static int current_field = 0;
static int total_fields = 0;

int csv_init_output(const char* filename, const char** headers, int num_headers) {
    csv_file = fopen(filename, "w");
    if (!csv_file) {
        return -1;
    }
    
    total_fields = num_headers;
    
    // Write headers
    for (int i = 0; i < num_headers; i++) {
        if (i > 0) fprintf(csv_file, ",");
        fprintf(csv_file, "%s", headers[i]);
    }
    fprintf(csv_file, "\n");
    
    return 0;
}

int csv_write_row(const char** values, int num_values) {
    if (!csv_file) return -1;
    
    for (int i = 0; i < num_values; i++) {
        if (i > 0) fprintf(csv_file, ",");
        fprintf(csv_file, "%s", values[i] ? values[i] : "");
    }
    fprintf(csv_file, "\n");
    
    return 0;
}

void csv_close_output(void) {
    if (csv_file) {
        fclose(csv_file);
        csv_file = NULL;
    }
}

void csv_write_field(const char* value) {
    if (!csv_file) return;
    
    if (current_field > 0) fprintf(csv_file, ",");
    fprintf(csv_file, "%s", value ? value : "");
    current_field++;
}

void csv_end_row(void) {
    if (!csv_file) return;
    
    fprintf(csv_file, "\n");
    current_field = 0;
} 