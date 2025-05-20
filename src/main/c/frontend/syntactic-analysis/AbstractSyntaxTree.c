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
            case AGGREGATION_COUNT:
            case AGGREGATION_SUM:   
            case AGGREGATION_AVG:
            case AGGREGATION_MIN:
            case AGGREGATION_MAX:
            releaseCondition(expression->aggregation);
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
    printf("[DEBUG][releaseStatement] Freeing DEFINE_STATEMENT...\n");
    free(statement->define.pattern_name);
    for (int i = 0; i < statement->define.condition_count; i++) {
        PatternCondition* pc = &statement->define.conditions[i];
        printf("[DEBUG][releaseStatement] Releasing PatternCondition[%d] (type=%d)\n", i, pc->type);
        switch (pc->type) {
            case PC_SAME:
                printf("[DEBUG][releaseStatement] → Releasing SAME field @ %p\n", (void*)pc->same.field);
                releaseField(pc->same.field);
                break;
            case PC_DIFFERENT:
                printf("[DEBUG][releaseStatement] → Releasing DIFFERENT field @ %p\n", (void*)pc->different.field);
                releaseField(pc->different.field);
                break;
            case PC_AGGREGATION:
                printf("[DEBUG][releaseStatement] → Releasing AGGREGATION field @ %p and expr @ %p\n",
                       (void*)pc->aggregation.field, (void*)pc->aggregation.value);
                releaseField(pc->aggregation.field);
                releaseExpression(pc->aggregation.value);
                break;
            case PC_TIMESPAN:
                printf("[DEBUG][releaseStatement] → Releasing TIMESPAN expr @ %p\n", (void*)pc->timespan.value);
                releaseExpression(pc->timespan.value);
                break;
        }
    }
    free(statement->define.conditions);
    break;
        case IMPORT_STATEMENT:
        case EXPORT_STATEMENT:
            free(statement->import_export.filename);
            if (statement->import_export.export_data != NULL) {
                free(statement->import_export.export_data);
            }
            break;
        case VARIABLE_DECLARATION_STATEMENT:
        releaseExpression(statement->variable_declaration.value);
        free(statement->variable_declaration.identifier);
        break;
    }
    free(statement);
}

void releaseCondition(Condition * condition) {
    if (condition == NULL) {
        return;
    }
    printf("[FREE] Condition @ %p (type=%d)\n", condition, condition->type);
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
        case PATTERN_CONDITION:
            switch (condition->pattern.type) {
                case PC_SAME:
                    releaseField(condition->pattern.same.field);
                    break;
                case PC_DIFFERENT:
                    releaseField(condition->pattern.different.field);
                    break;
                case PC_AGGREGATION:
                    releaseField(condition->pattern.aggregation.field);
                    releaseExpression(condition->pattern.aggregation.value);
                    break;
                case PC_TIMESPAN:
                    if (condition->pattern.timespan.value != NULL) {
                        releaseExpression(condition->pattern.timespan.value);
                    }
                    break;
                default:
                    logError(_logger, "Unknown pattern condition type: %d", condition->pattern.type);
                    break;
            }
            break;
        default:
            logError(_logger, "Unknown condition type: %d", condition->type);
            break;
    }
    free(condition);
}


void releaseField(Field * field) {
    if (field == NULL) return;
    printf("[FREE] Field @ %p (name='%s')\n", field, field->name);
    if (field->protocol != NULL) {
        free(field->protocol);
    }

    if (field->name != NULL) {
        free(field->name);
    }

    free(field);
}


void releaseFieldList(FieldList * fieldList) {
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
            case VARIABLE_REFERENCE:
            free(factor->variable_name);
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
            default:
                break;
        }
        free(constant);
    }
}