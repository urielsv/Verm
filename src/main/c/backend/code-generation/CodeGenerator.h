#ifndef VERM_CODE_GENERATOR_H
#define VERM_CODE_GENERATOR_H

#include <stdio.h>
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

/**
 * @brief Generates C code for PCAP processing
 * @param root Root node of the validated AST
 * @param output File to write the generated code to
 * @return 0 if generation successful, non-zero otherwise
 */
int generate_pcap_processor(Program* root, FILE* output);

/**
 * @brief Generates C code for BPF filter
 * @param filter Filter condition to generate code for
 * @param output File to write the generated code to
 * @return 0 if generation successful, non-zero otherwise
 */
int generate_filter_code(Condition* filter, FILE* output);

/**
 * @brief Generates C code for field extraction
 * @param extract Extraction statement to generate code for
 * @param output File to write the generated code to
 * @return 0 if generation successful, non-zero otherwise
 */
int generate_extraction_code(ExtractStatement* extract, FILE* output);

/**
 * @brief Generates C code for aggregation operations
 * @param agg Aggregation expression to generate code for
 * @param output File to write the generated code to
 * @return 0 if generation successful, non-zero otherwise
 */
int generate_aggregation_code(Expression* agg, FILE* output);

/**
 * @brief Generates the main function for the C program
 * @param root Root node of the AST
 * @param output File to write the generated code to
 * @return 0 if generation successful, non-zero otherwise
 */
int generate_main_function(Program* root, FILE* output);

#endif // VERM_CODE_GENERATOR_H 