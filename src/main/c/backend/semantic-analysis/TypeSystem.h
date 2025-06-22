#ifndef VERM_TYPE_SYSTEM_H
#define VERM_TYPE_SYSTEM_H

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

/**
 * @brief Enumeration of Verm language types
 */
typedef enum {
    TYPE_PACKET,    // PCAP packet type
    TYPE_ADDRESS,   // IP address type
    TYPE_INTEGER,   // Integer type
    TYPE_BOOLEAN,   // Boolean type
    TYPE_STRING,    // String type
    TYPE_TIMESTAMP, // Timestamp type
    TYPE_ARRAY      // Array type
} VermType;

/**
 * @brief Enumeration of operator types
 */
typedef enum {
    OP_ADD,         // Addition
    OP_SUB,         // Subtraction
    OP_MUL,         // Multiplication
    OP_DIV,         // Division
    OP_EQ,          // Equal
    OP_NEQ,         // Not equal
    OP_GT,          // Greater than
    OP_LT,          // Less than
    OP_GTE,         // Greater than or equal
    OP_LTE          // Less than or equal
} OperatorType;

/**
 * @brief Checks if two types are compatible for a given operation
 * @param left Left operand type
 * @param right Right operand type
 * @param op Operator type
 * @return 0 if types are compatible, non-zero otherwise
 */
int check_type_compatibility(VermType left, VermType right, OperatorType op);

/**
 * @brief Infers the type of an expression
 * @param expr Expression node to analyze
 * @return Inferred type of the expression
 */
VermType infer_expression_type(Expression* expr);

/**
 * @brief Gets a string representation of a type
 * @param type The type to convert
 * @return String representation of the type
 */
const char* type_to_string(VermType type);

#endif // VERM_TYPE_SYSTEM_H 