#include "SymbolTable.h"
#include "../../shared/Logger.h"
#include <string.h>
#include <stdlib.h>

#define MAX_SYMBOLS 1000

typedef struct Symbol {
    char* name;
    VermType type;
    int scope;
    void* value;
} Symbol;

static Symbol symbol_table[MAX_SYMBOLS];
static int symbol_count = 0;
static int current_scope = 0;

int symbol_table_init(void) {
    symbol_count = 0;
    current_scope = 0;
    memset(symbol_table, 0, sizeof(symbol_table));
    logInformation(NULL, "Symbol table initialized");
    return 0;
}

int symbol_table_add(const char* name, VermType type) {
    if (symbol_count >= MAX_SYMBOLS) {
        logError(NULL, "Symbol table is full");
        return -1;
    }
    
    // Check if symbol already exists in current scope
    for (int i = 0; i < symbol_count; i++) {
        if (symbol_table[i].scope == current_scope && 
            strcmp(symbol_table[i].name, name) == 0) {
            logError(NULL, "Symbol '%s' already exists in current scope", name);
            return -1;
        }
    }
    
    // Add new symbol
    symbol_table[symbol_count].name = strdup(name);
    symbol_table[symbol_count].type = type;
    symbol_table[symbol_count].scope = current_scope;
    symbol_table[symbol_count].value = NULL;
    
    logDebugging(NULL, "Added symbol: %s (type: %s, scope: %d)", 
              name, type_to_string(type), current_scope);
    
    symbol_count++;
    return 0;
}

VermType symbol_table_lookup(const char* name) {
    // Search from current scope backwards (for variable shadowing)
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            logDebugging(NULL, "Found symbol: %s (type: %s, scope: %d)", 
                      name, type_to_string(symbol_table[i].type), symbol_table[i].scope);
            return symbol_table[i].type;
        }
    }
    
    logWarning(NULL, "Symbol '%s' not found", name);
    return TYPE_INTEGER; // Default type for unknown symbols
}

void symbol_table_enter_scope(void) {
    current_scope++;
    logDebugging(NULL, "Entered scope: %d", current_scope);
}

void symbol_table_exit_scope(void) {
    if (current_scope > 0) {
        logDebugging(NULL, "Exited scope: %d", current_scope);
        current_scope--;
        
        // Remove symbols from exited scope
        for (int i = symbol_count - 1; i >= 0; i--) {
            if (symbol_table[i].scope > current_scope) {
                free(symbol_table[i].name);
                // Move last symbol to this position
                if (i < symbol_count - 1) {
                    symbol_table[i] = symbol_table[symbol_count - 1];
                }
                symbol_count--;
            }
        }
    }
}

void symbol_table_cleanup(void) {
    for (int i = 0; i < symbol_count; i++) {
        free(symbol_table[i].name);
    }
    symbol_count = 0;
    current_scope = 0;
    logInformation(NULL, "Symbol table cleaned up");
} 