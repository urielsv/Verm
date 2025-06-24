#ifndef VERM_CODE_GENERATOR_H
#define VERM_CODE_GENERATOR_H

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include <stdio.h>

/**
 * @brief Generate PCAP processor code
 * @param root Program AST root
 * @param output Output file
 * @return 0 if successful, non-zero otherwise
 */
int generate_pcap_processor(Program* root, FILE* output);

/**
 * @brief Generate filter code
 * @param filter Filter condition
 * @param output Output file
 * @return 0 if successful, non-zero otherwise
 */
int generate_filter_code(Condition* filter, FILE* output);

/**
 * @brief Generate extraction code
 * @param extract Extract statement
 * @param output Output file
 * @return 0 if successful, non-zero otherwise
 */
int generate_extraction_code(ExtractStatement* extract, FILE* output);

/**
 * @brief Generate aggregation code
 * @param agg Aggregation expression
 * @param output Output file
 * @return 0 if successful, non-zero otherwise
 */
int generate_aggregation_code(Expression* agg, FILE* output);

/**
 * @brief Generate main function
 * @param root Program AST root
 * @param output Output file
 * @return 0 if successful, non-zero otherwise
 */
int generate_main_function(Program* root, FILE* output);

/**
 * @brief Generate helper functions
 * @param output Output file
 */
void generate_helper_functions(FILE* output);

/**
 * @brief Generate expression code
 * @param expr Expression to generate code for
 * @param output Output file
 */
void generate_expression_code(Expression* expr, FILE* output);

/**
 * @brief Generate factor code
 * @param factor Factor to generate code for
 * @param output Output file
 */
void generate_factor_code(Factor* factor, FILE* output);

/**
 * @brief Generate field code
 * @param field Field to generate code for
 * @param output Output file
 */
void generate_field_code(Field* field, FILE* output);

/**
 * @brief Generate condition code
 * @param condition Condition to generate code for
 * @param output Output file
 */
void generate_condition_code(Condition* condition, FILE* output);

/**
 * @brief Generate field extraction code
 * @param field Field to extract
 * @param output Output file
 */
void generate_field_extraction_code(Field* field, FILE* output);

/**
 * @brief Get comparison operator string
 * @param op Comparison operator
 * @return String representation of operator
 */
const char* get_comparison_operator(ComparisonOperator op);

#endif // VERM_CODE_GENERATOR_H 