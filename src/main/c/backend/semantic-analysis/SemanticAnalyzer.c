#include <string.h>
#include "SemanticAnalyzer.h"
#include "../../shared/Logger.h"

int validate_program(Program* root) {
    if (!root) {
        logError(NULL, "AST root is null");
        return -1;
    }
    
    logInformation(NULL, "Starting semantic analysis");
    
    // Basic validation - check if root has statements
    if (!root->statements) {
        logWarning(NULL, "AST has no statements");
        return 0;
    }
    
    // Validate each statement
    StatementList* current = root->statements;
    while (current) {
        if (check_variable_types(current->statement) != 0) {
            return -1;
        }
        current = current->next;
    }
    
    logInformation(NULL, "Semantic analysis completed successfully");
    return 0;
}

int check_variable_types(Statement* statement) {
    if (!statement) {
        return 0; // Null statements are valid
    }
    
    // Basic type checking - for now just log the statement type
    logDebugging(NULL, "Checking types for statement type: %d", statement->type);
    
    // TODO: Implement proper type checking based on statement type
    switch (statement->type) {
        case CAPTURE_STATEMENT:
            // Validate capture statement
            break;
        case EXTRACT_STATEMENT:
            // Validate extract statement
            break;
        case FILTER_STATEMENT:
            // Validate filter statement
            break;
        case ALERT_STATEMENT:
            // Validate alert statement
            break;
        case DEFINE_STATEMENT:
            // Validate define statement
            break;
        case IMPORT_STATEMENT:
            // Validate import statement
            break;
        case EXPORT_STATEMENT:
            // Validate export statement
            break;
        case VARIABLE_DECLARATION_STATEMENT:
            // Validate variable declaration
            break;
    }
    
    return 0;
}

int validate_protocol_fields(Statement* statement) {
    if (!statement) {
        return 0;
    }
    
    logDebugging(NULL, "Validating protocol fields for statement type: %d", statement->type);
    
    // TODO: Implement protocol field validation based on statement type
    switch (statement->type) {
        case EXTRACT_STATEMENT:
            // Validate fields in extract statement
            if (statement->extract.fields) {
                FieldList* current = statement->extract.fields;
                while (current) {
                    // Validate each field
                    if (current->field) {
                        logDebugging(NULL, "Validating field: %s.%s", 
                                current->field->protocol ? current->field->protocol : "unknown",
                                current->field->name ? current->field->name : "unknown");
                    }
                    current = current->next;
                }
            }
            break;
        default:
            // Other statement types don't have protocol fields
            break;
    }
    
    return 0;
}

int validate_bpf_filters(char* filter_expr) {
    if (!filter_expr) {
        logError(NULL, "BPF filter expression is null");
        return -1;
    }
    
    logDebugging(NULL, "Validating BPF filter: %s", filter_expr);
    
    // Basic validation - check if filter is not empty
    if (strlen(filter_expr) == 0) {
        logError(NULL, "BPF filter expression is empty");
        return -1;
    }
    
    // TODO: Add more sophisticated BPF validation
    logInformation(NULL, "BPF filter validation passed");
    return 0;
} 