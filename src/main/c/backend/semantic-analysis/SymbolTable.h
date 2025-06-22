#ifndef VERM_SYMBOL_TABLE_H
#define VERM_SYMBOL_TABLE_H

#include "TypeSystem.h"

/**
 * @brief Initialize the symbol table
 * @return 0 if successful, non-zero otherwise
 */
int symbol_table_init(void);

/**
 * @brief Add a symbol to the current scope
 * @param name Symbol name
 * @param type Symbol type
 * @return 0 if successful, non-zero otherwise
 */
int symbol_table_add(const char* name, VermType type);

/**
 * @brief Look up a symbol in the symbol table
 * @param name Symbol name to look up
 * @return Type of the symbol, or TYPE_INTEGER if not found
 */
VermType symbol_table_lookup(const char* name);

/**
 * @brief Enter a new scope
 */
void symbol_table_enter_scope(void);

/**
 * @brief Exit the current scope
 */
void symbol_table_exit_scope(void);

/**
 * @brief Clean up the symbol table
 */
void symbol_table_cleanup(void);

#endif // VERM_SYMBOL_TABLE_H 