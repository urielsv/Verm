#ifndef VERM_SEMANTIC_ANALYZER_H
#define VERM_SEMANTIC_ANALYZER_H

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

/**
 * @brief Validates the entire program AST
 * @param root The root node of the AST
 * @return 0 if validation successful, non-zero otherwise
 */
int validate_program(Program* root);

/**
 * @brief Checks variable types in the AST
 * @param statement The statement to check
 * @return 0 if types are valid, non-zero otherwise
 */
int check_variable_types(Statement* statement);

/**
 * @brief Validates protocol fields in the AST
 * @param statement The statement containing protocol information
 * @return 0 if protocol fields are valid, non-zero otherwise
 */
int validate_protocol_fields(Statement* statement);

/**
 * @brief Validates BPF filter expressions
 * @param filter_expr The filter expression to validate
 * @return 0 if filter is valid, non-zero otherwise
 */
int validate_bpf_filters(char* filter_expr);

#endif // VERM_SEMANTIC_ANALYZER_H 