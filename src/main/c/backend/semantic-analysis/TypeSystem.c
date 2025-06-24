#include "TypeSystem.h"
#include "../../shared/Logger.h"
#include <string.h>

int check_type_compatibility(VermType left, VermType right, OperatorType op) {
    // Arithmetic operators
    if (op == OP_ADD || op == OP_SUB || op == OP_MUL || op == OP_DIV) {
        // Only integers can be used in arithmetic operations
        if (left == TYPE_INTEGER && right == TYPE_INTEGER) {
            return 0;
        }
        return -1;
    }
    
    // Comparison operators
    if (op == OP_EQ || op == OP_NEQ) {
        // All types can be compared for equality
        return 0;
    }
    
    if (op == OP_GT || op == OP_LT || op == OP_GTE || op == OP_LTE) {
        // Only integers, timestamps, and addresses can be compared
        if ((left == TYPE_INTEGER && right == TYPE_INTEGER) ||
            (left == TYPE_TIMESTAMP && right == TYPE_TIMESTAMP) ||
            (left == TYPE_ADDRESS && right == TYPE_ADDRESS)) {
            return 0;
        }
        return -1;
    }
    
    return -1;
}

VermType infer_expression_type(Expression* expr) {
    if (!expr) {
        return TYPE_INTEGER; // Default type
    }
    
    switch (expr->type) {
        case ADDITION:
        case SUBTRACTION:
        case MULTIPLICATION:
        case DIVISION:
            // Arithmetic operations always return integer
            return TYPE_INTEGER;
            
        case FACTOR:
            return infer_factor_type(expr->factor);
            
        case AGGREGATION_COUNT:
        case AGGREGATION_SUM:
        case AGGREGATION_AVG:
        case AGGREGATION_MIN:
        case AGGREGATION_MAX:
            // Aggregations return integer
            return TYPE_INTEGER;
            
        default:
            return TYPE_INTEGER;
    }
}

VermType infer_factor_type(Factor* factor) {
    if (!factor) {
        return TYPE_INTEGER;
    }
    
    switch (factor->type) {
        case CONSTANT:
            return factor->constant->value.type;
            
        case EXPRESSION:
            return infer_expression_type(factor->expression);
            
        case FIELD:
            return infer_field_type(factor->field);
            
        case VARIABLE_REFERENCE:
            // This would need to be resolved from symbol table
            return TYPE_INTEGER;
            
        default:
            return TYPE_INTEGER;
    }
}

VermType infer_field_type(Field* field) {
    if (!field || !field->protocol || !field->name) {
        return TYPE_INTEGER;
    }
    
    // IP fields
    if (strcmp(field->protocol, "ip") == 0) {
        if (strcmp(field->name, "src") == 0 || strcmp(field->name, "dst") == 0) {
            return TYPE_ADDRESS;
        }
        if (strcmp(field->name, "protocol") == 0 || 
            strcmp(field->name, "ttl") == 0 || 
            strcmp(field->name, "length") == 0) {
            return TYPE_INTEGER;
        }
    }
    
    // TCP fields
    if (strcmp(field->protocol, "tcp") == 0) {
        if (strcmp(field->name, "src_port") == 0 || 
            strcmp(field->name, "dst_port") == 0 ||
            strcmp(field->name, "window") == 0 ||
            strcmp(field->name, "flags") == 0) {
            return TYPE_INTEGER;
        }
        if (strcmp(field->name, "seq") == 0 || strcmp(field->name, "ack") == 0) {
            return TYPE_INTEGER;
        }
    }
    
    // UDP fields
    if (strcmp(field->protocol, "udp") == 0) {
        if (strcmp(field->name, "src_port") == 0 || 
            strcmp(field->name, "dst_port") == 0 ||
            strcmp(field->name, "length") == 0) {
            return TYPE_INTEGER;
        }
    }
    
    // HTTP fields
    if (strcmp(field->protocol, "http") == 0) {
        if (strcmp(field->name, "status_code") == 0 || 
            strcmp(field->name, "content_length") == 0) {
            return TYPE_INTEGER;
        }
        if (strcmp(field->name, "method") == 0 || 
            strcmp(field->name, "url") == 0 ||
            strcmp(field->name, "host") == 0 ||
            strcmp(field->name, "user_agent") == 0 ||
            strcmp(field->name, "cookies") == 0 ||
            strcmp(field->name, "content_type") == 0 ||
            strcmp(field->name, "server") == 0 ||
            strcmp(field->name, "date") == 0) {
            return TYPE_STRING;
        }
    }
    
    // Packet fields
    if (strcmp(field->protocol, "packet") == 0) {
        if (strcmp(field->name, "length") == 0) {
            return TYPE_INTEGER;
        }
        if (strcmp(field->name, "timestamp") == 0) {
            return TYPE_TIMESTAMP;
        }
    }
    
    return TYPE_INTEGER; // Default
}

const char* type_to_string(VermType type) {
    switch (type) {
        case TYPE_PACKET:
            return "packet";
        case TYPE_ADDRESS:
            return "address";
        case TYPE_INTEGER:
            return "integer";
        case TYPE_BOOLEAN:
            return "boolean";
        case TYPE_STRING:
            return "string";
        case TYPE_TIMESTAMP:
            return "timestamp";
        case TYPE_ARRAY:
            return "array";
        default:
            return "unknown";
    }
}

const char* operator_to_string(OperatorType op) {
    switch (op) {
        case OP_ADD:
            return "+";
        case OP_SUB:
            return "-";
        case OP_MUL:
            return "*";
        case OP_DIV:
            return "/";
        case OP_EQ:
            return "==";
        case OP_NEQ:
            return "!=";
        case OP_GT:
            return ">";
        case OP_LT:
            return "<";
        case OP_GTE:
            return ">=";
        case OP_LTE:
            return "<=";
        default:
            return "unknown";
    }
} 