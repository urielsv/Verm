#include "TypeSystem.h"
#include "../../shared/Logger.h"

int check_type_compatibility(VermType left, VermType right, OperatorType op) {
    logDebugging(NULL, "Checking type compatibility: %s %d %s", 
              type_to_string(left), op, type_to_string(right));
    
    // Basic type compatibility rules
    switch (op) {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
            // Arithmetic operations only work with numeric types
            if (left == TYPE_INTEGER && right == TYPE_INTEGER) {
                return 0;
            }
            logError(NULL, "Arithmetic operation not supported between %s and %s", 
                     type_to_string(left), type_to_string(right));
            return -1;
            
        case OP_EQ:
        case OP_NEQ:
            // Equality operations work with any types
            return 0;
            
        case OP_GT:
        case OP_LT:
        case OP_GTE:
        case OP_LTE:
            // Comparison operations work with numeric types
            if (left == TYPE_INTEGER && right == TYPE_INTEGER) {
                return 0;
            }
            logError(NULL, "Comparison operation not supported between %s and %s", 
                     type_to_string(left), type_to_string(right));
            return -1;
    }
    
    return -1;
}

VermType infer_expression_type(Expression* expr) {
    if (!expr) {
        return TYPE_INTEGER; // Default type
    }
    
    logDebugging(NULL, "Inferring type for expression");
    
    // Basic type inference - for now return INTEGER
    // TODO: Implement proper type inference based on expression content
    return TYPE_INTEGER;
}

const char* type_to_string(VermType type) {
    switch (type) {
        case TYPE_PACKET:    return "PACKET";
        case TYPE_ADDRESS:   return "ADDRESS";
        case TYPE_INTEGER:   return "INTEGER";
        case TYPE_BOOLEAN:   return "BOOLEAN";
        case TYPE_STRING:    return "STRING";
        case TYPE_TIMESTAMP: return "TIMESTAMP";
        case TYPE_ARRAY:     return "ARRAY";
        default:             return "UNKNOWN";
    }
} 