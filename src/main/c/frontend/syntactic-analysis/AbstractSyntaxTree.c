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
        }
        free(expression);
    }
}

void releaseProgram(Program * program) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (program != NULL) {
        if (program->statements != NULL) {
            StatementList* current = program->statements;
            while (current != NULL) {
                StatementList* next = current->next;
                releaseStatement(current->statement);
                free(current);
                current = next;
            }
        }
        free(program);
    }
}

void releaseStatement(Statement * statement) {
    if (statement == NULL) return;

    switch (statement->type) {
        case CAPTURE_STATEMENT:
            free(statement->capture.interface);
            if (statement->capture.filter != NULL) {
                releaseCondition(statement->capture.filter);
            }
            break;

        case EXTRACT_STATEMENT:
            free(statement->extract.source);
            if (statement->extract.filter != NULL) {
                releaseCondition(statement->extract.filter);
            }
            releaseFieldList(statement->extract.fields);
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

        case GROUP_STATEMENT:
            releaseFieldList(statement->group.group_fields);
            if (statement->group.having != NULL) {
                releaseCondition(statement->group.having);
            }
            break;

        case DEFINE_STATEMENT:
            free(statement->define.pattern_name);
            for (int i = 0; i < statement->define.condition_count; i++) {
                PatternCondition* pc = &statement->define.conditions[i];
                if (pc->type == PC_SAME || pc->type == PC_DIFFERENT || pc->type == PC_COUNT) {
                    free(pc->same.field); // Todos comparten el mismo campo en la union
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
    free(statement);
}


void releaseCondition(Condition * condition) {
    if (condition == NULL) return;

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
                switch (condition->pattern.type) {
                    case PC_SAME:
                    case PC_DIFFERENT:
                        if (condition->pattern.same.field) {
                            free(condition->pattern.same.field);
                        }
                        break;
                    case PC_COUNT:
                        if (condition->pattern.count.field) {
                            free(condition->pattern.count.field);
                        }
                        break;
                    case PC_TIMESPAN:
                        break;
                }
            } else {
                logError(_logger, "Unknown condition type: %d", condition->type);
            }
            break;
    }
    free(condition);
}
void releaseFieldList(FieldList * fieldList) {
    while (fieldList != NULL) {
        FieldList * next = fieldList->next;
        if (fieldList->field != NULL) {
            if (fieldList->field->protocol != NULL) {
                free(fieldList->field->protocol);
            }
            free(fieldList->field->name);
            if (fieldList->field->alias != NULL) {
                free(fieldList->field->alias);
            }
            free(fieldList->field);
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
                    if (factor->field->protocol != NULL) {
                        free(factor->field->protocol);
                    }
                    free(factor->field->name);
                    if (factor->field->alias != NULL) {
                        free(factor->field->alias);
                    }
                    free(factor->field);
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