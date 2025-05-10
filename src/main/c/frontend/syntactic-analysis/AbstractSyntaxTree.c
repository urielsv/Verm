#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void initializeAbstractSyntaxTreeModule() {
    _logger = createLogger("AbstractSyntaxTree");
}

void shutdownAbstractSyntaxTreeModule() {
    if (_logger != NULL) {
        destroyLogger(_logger);
    }
}

/** PUBLIC FUNCTIONS */

void releaseExpression(Expression * expression) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (expression != NULL) {
        switch (expression->type) {
            case ADDITION:
            case DIVISION:
            case MULTIPLICATION:
            case SUBTRACTION:
                releaseExpression(expression->leftExpression);
                releaseExpression(expression->rightExpression);
                break;
            case FACTOR:
                releaseFactor(expression->factor);
                break;
            case EXPRESSION_COUNT:
            releaseCondition(expression->count);
                break;
        }
        free(expression);
    }
}

void releaseProgram(Program* program) {
    if (program == NULL) return;

    switch (program->type) {
        case PROGRAM_STATEMENTS:
            if (program->statements) {
                StatementList* current = program->statements;
                while (current) {
                    releaseStatement(current->statement);
                    StatementList* temp = current;
                    current = current->next;
                    free(temp);
                }
            }
            break;

        case PROGRAM_EXPRESSION:
            if (program->expression) {
                releaseExpression(program->expression);
            }
            break;
    }

    free(program);
}


void releaseStatement(Statement * statement) {
    if (statement == NULL) return;

    switch (statement->type) {
        case CAPTURE_STATEMENT:
            if (statement->capture.interface != NULL)
            free(statement->capture.interface);
            if (statement->capture.filter != NULL) {
                releaseCondition(statement->capture.filter);
            }
            break;

        case EXTRACT_STATEMENT:
            printf("[DEBUG] Freeing EXTRACT_STATEMENT\n");
            if (statement->extract.source != NULL) {
                free(statement->extract.source);
            }
            if (statement->extract.fields != NULL) {
                releaseFieldList(statement->extract.fields);
            }            
            if (statement->extract.filter != NULL) {
                releaseCondition(statement->extract.filter);
            }
        
            if (statement->group.group_fields != NULL) {
                releaseFieldList(statement->group.group_fields);
            }
        
            if (statement->group.having != NULL) {
                releaseCondition(statement->group.having);
            }
        
            break;
        
        case FILTER_STATEMENT:
            
            if (statement->filter.condition != NULL) {
                releaseCondition(statement->filter.condition);
            }
            break;

        case ALERT_STATEMENT:
            if (statement->alert.trigger != NULL) {
                releaseCondition(statement->alert.trigger);
            }
            free(statement->alert.message);
            break;

        case DEFINE_STATEMENT:
            free(statement->define.pattern_name);
            for (int i = 0; i < statement->define.condition_count; i++) {
                PatternCondition* pc = &statement->define.conditions[i];
                if (pc->type == PC_SAME || pc->type == PC_DIFFERENT || pc->type == PC_COUNT) {
                    free(pc->same.field); 
                }
            }
            free(statement->define.conditions);
            break;

        case IMPORT_EXPORT_STATEMENT:
            free(statement->import_export.filename);
            if (statement->import_export.export_data != NULL) {
                free(statement->import_export.export_data);
            }
            break;
    }
    printf("[DEBUG] Freeing statement = %p\n", (void*)statement);
    free(statement);
}

void releaseCondition(Condition * condition) {
    if (condition == NULL) {
        return;
    }


    switch (condition->type) {
        case COMPARISON:
            releaseExpression(condition->comparison.left);
            releaseExpression(condition->comparison.right);
            break;

        case LOGICAL_AND:
        case LOGICAL_OR:
            releaseCondition(condition->logical.left);
            releaseCondition(condition->logical.right);
            break;

        case LOGICAL_NOT:
            releaseCondition(condition->logical.left);
            break;

        default:
            if (condition->type >= PC_SAME && condition->type <= PC_TIMESPAN) {
                printf("[DEBUG] PATTERN: type = %d\n", condition->pattern.type);
                switch (condition->pattern.type) {
                    case PC_SAME:
                    case PC_DIFFERENT:
                        printf("[DEBUG] PATTERN SAME/DIFFERENT: field = %p\n",
                               (void*)condition->pattern.same.field);
                        if (condition->pattern.same.field) {
                            free(condition->pattern.same.field);
                        }
                        break;
                    case PC_COUNT:
                        printf("[DEBUG] PATTERN COUNT: field = %p\n", (void*)condition->pattern.count.field);
                            if (condition->pattern.count.field) {
                                free(condition->pattern.count.field);
                            }
                        break;
                    case PC_TIMESPAN:
                        printf("[DEBUG] PATTERN TIMESPAN: no fields to free\n");
                        break;
                }
            } else {
                logError(_logger, "Unknown condition type: %d", condition->type);
            }
            break;
    }

    printf("[DEBUG] Freeing condition = %p\n", (void*)condition);
    free(condition);
}

void releaseField(Field * field) {
    printf("[DEBUGPRINT]releaseField: %p\n", field);
    if (field == NULL) return;

    if (field->protocol != NULL) {
        free(field->protocol);
    }

    if (field->name != NULL) {
        free(field->name);
    }

    free(field);
}


void releaseFieldList(FieldList * fieldList) {
    printf("[DEBUGPRINT]releaseFieldList: %p", fieldList);
    while (fieldList != NULL) {
        FieldList * next = fieldList->next;
        if (fieldList->field != NULL) {
            releaseField(fieldList->field);
        }
        free(fieldList);
        fieldList = next;
    }
}

void releaseFactor(Factor * factor) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (factor != NULL) {
        switch (factor->type) {
            case CONSTANT:
                releaseConstant(factor->constant);
                break;
            case EXPRESSION:
                releaseExpression(factor->expression);
                break;
            case FIELD:
                if (factor->field != NULL) {
                    releaseField(factor->field);
                }
                break;
        }
        free(factor);
    }
}

void releaseConstant(Constant * constant) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (constant != NULL) {
        switch (constant->value.type) {
            case STRING_TYPE:
                free(constant->value.string);
                break;
            case ADDRESS_TYPE_VALUE:
                free(constant->value.address.value);
                break;
            case PACKET_TYPE_VALUE:
                free(constant->value.packet.raw_data);
                break;
            default:
                break;
        }
        free(constant);
    }
}