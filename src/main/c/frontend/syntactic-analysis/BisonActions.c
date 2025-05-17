#include "BisonActions.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void initializeBisonActionsModule() {
	_logger = createLogger("BisonActions");
}

void shutdownBisonActionsModule() {
	if (_logger != NULL) {
		destroyLogger(_logger);
	}
}

/** IMPORTED FUNCTIONS */

extern unsigned int flexCurrentContext(void);

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PUBLIC FUNCTIONS */


int countConditions(Condition* cond) {
    if (!cond) return 0;
    if (cond->type == LOGICAL_AND) {
        return countConditions(cond->logical.left) + countConditions(cond->logical.right);
    } else if (cond->type == PATTERN_CONDITION) {
        return 1;
    } else {
        fprintf(stderr, "[countConditions] Unexpected condition type %d\n", cond->type);
        exit(1);
    }
}

static void flattenConditions(Condition* cond, PatternCondition* out, int* index) {
    if (!cond) return;
    if (cond->type == LOGICAL_AND) {
        flattenConditions(cond->logical.left, out, index);
        flattenConditions(cond->logical.right, out, index);
        printf("[DEBUG][convert] Freeing LOGICAL_AND node @ %p\n", (void*)cond);
        free(cond);
    } else if (cond->type == PATTERN_CONDITION) {
        printf("[DEBUG][convert] Flattening PATTERN_CONDITION @ %p\n", (void*)cond);
        PatternCondition* pc = &out[(*index)++];
        pc->type = cond->pattern.type;
        switch (cond->pattern.type) {
            case PC_SAME:
                pc->same.field = cond->pattern.same.field;
                break;
            case PC_DIFFERENT:
                pc->different.field = cond->pattern.different.field;
                break;
            case PC_AGGREGATION:
                pc->aggregation.func = cond->pattern.aggregation.func;
                pc->aggregation.field = cond->pattern.aggregation.field;
                pc->aggregation.op = cond->pattern.aggregation.op;
                pc->aggregation.value = cond->pattern.aggregation.value;
                break;
            case PC_TIMESPAN:
                pc->timespan.op = cond->pattern.timespan.op;
                pc->timespan.value = cond->pattern.timespan.value;
                break;
        }
        printf("[DEBUG][convert] Flattened pattern type %d from Condition @ %p\n", pc->type, (void*)cond);
        free(cond);
    } else {
        fprintf(stderr, "[flattenConditions] Unexpected condition type %d\n", cond->type);
        exit(1);
    }
}

PatternCondition* convertConditionsToPatterns(Condition* cond) {
    int count = countConditions(cond);
    printf("[DEBUG][convert] countConditions = %d\n", count);
    PatternCondition* patterns = calloc(count, sizeof(PatternCondition));
    int index = 0;
    flattenConditions(cond, patterns, &index);
    return patterns;
}





/* ==================== FUNCIONES DE CONDICIONES ==================== */

Condition* SamePatternConditionSemanticAction(Field* field) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* cond = calloc(1, sizeof(Condition));
    cond->type = PATTERN_CONDITION;
    cond->pattern.type = PC_SAME;
    cond->pattern.same.field = field;
    return cond;
}

Condition* DifferentPatternConditionSemanticAction(Field* field) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* cond = calloc(1, sizeof(Condition));
    cond->type = PATTERN_CONDITION;
    cond->pattern.type = PC_DIFFERENT;
    cond->pattern.different.field = field;
    return cond;
}

Condition* CountPatternConditionSemanticAction(Field* field, ComparisonOperator op, int value) {
    Constant* constant = IntegerConstantSemanticAction(value);
    Factor* factor = ConstantFactorSemanticAction(constant);
    Expression* expr = FactorExpressionSemanticAction(factor);
    return CountPatternConditionExpressionSemanticAction(field, op, expr);
}

Condition* MultiplePatternConditionsSemanticAction(Condition* conditions, Condition* newCondition) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* combined = calloc(1, sizeof(Condition));
    combined->type = LOGICAL_AND;
    combined->logical.left = conditions;
    combined->logical.right = newCondition;
    combined->logical.op = AND_OP;
    return combined;    
}

Condition* ParenthesizedConditionSemanticAction(Condition* condition) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return condition;
}

Condition* IdentifierConditionSemanticAction(char* identifier) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return FieldConditionSemanticAction(identifier, NULL);
}

Condition* FieldConditionSemanticAction(char* field1, char* field2) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* cond = calloc(1, sizeof(Condition));
    cond->type = PC_SAME;
    cond->pattern.type = PC_SAME;
    
   if (field2) {
    char* fullName = malloc(strlen(field1) + strlen(field2) + 2);
    sprintf(fullName, "%s.%s", field1, field2);
    cond->pattern.same.field = createSimpleField(fullName, NULL);
} else {
    cond->pattern.same.field = createSimpleField(field1, NULL);
}
    
    return cond;
}


Condition* TimespanPatternConditionSemanticAction(ComparisonOperator op, Expression* value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* cond = calloc(1, sizeof(Condition));
    cond->type = PATTERN_CONDITION;
    cond->pattern.type = PC_TIMESPAN;
    cond->pattern.timespan.op = op;
    cond->pattern.timespan.value = value;
    return cond;
}

Condition* ComparisonSemanticAction(Expression* left, Expression* right, ComparisonOperator op) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* cond = calloc(1, sizeof(Condition));
    cond->type = COMPARISON;
    cond->comparison.left = left;
    cond->comparison.right = right;
    cond->comparison.op = op;
    return cond;
}


/* ==================== FUNCIONES DE FACTORES ==================== */
Constant* IntegerConstantSemanticAction(const int value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Constant* constant = malloc(sizeof(Constant));
    constant->value.integer= value;
    return constant;
}

Constant* StringConstantSemanticAction(char* value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Constant* constant = malloc(sizeof(Constant));
    constant->value.type = STRING_TYPE;
    constant->value.string = value;
    return constant;
}

Constant* BooleanConstantSemanticAction(bool value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Constant* constant = malloc(sizeof(Constant));
    constant->value.type = BOOLEAN_TYPE;
    constant->value.boolean = value;
    return constant;
}

Constant* TimestampConstantSemanticAction(time_t value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Constant* constant = malloc(sizeof(Constant));
    constant->value.type = TIMESTAMP_TYPE;
    constant->value.timestamp = value;
    return constant;
}



/* ==================== EXPRESIONES Y FACTORES ==================== */

Expression* ArithmeticExpressionSemanticAction(Expression* left, Expression* right, ExpressionType type) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Expression* expr = calloc(1, sizeof(Expression));
    expr->type = type;
    expr->leftExpression = left;
    expr->rightExpression = right;
    return expr;
}

Expression* FactorExpressionSemanticAction(Factor* factor) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Expression* expr = calloc(1, sizeof(Expression));
    expr->type = FACTOR;
    expr->factor = factor;
    return expr;
}

Factor* ConstantFactorSemanticAction(Constant* constant) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Factor* factor = calloc(1, sizeof(Factor));
    factor->type = CONSTANT;
    factor->constant = constant;
    return factor;
}

Factor* ExpressionFactorSemanticAction(Expression* expression) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Factor* factor = calloc(1, sizeof(Factor));
    factor->type = EXPRESSION;
    factor->expression = expression;
    return factor;
}

Factor* FieldFactorSemanticAction(Field* field) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Factor* factor = calloc(1, sizeof(Factor));
    factor->type = FIELD;
    factor->field = field;
    return factor;
}

/* ==================== PROGRAMAS ==================== */

Program* StatementProgramSemanticAction(CompilerState* state, Statement* statement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Program* program = calloc(1, sizeof(Program));
    program->type = PROGRAM_STATEMENTS;
    StatementList* list = calloc(1, sizeof(StatementList));
    list->statement = statement;
    list->next = NULL;
    program->statements = list;
    state->abstractSyntaxtTree = program;
    state->succeed = true;
    return program;
}

Program* ExpressionProgramSemanticAction(CompilerState* state, Expression* expression) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Program* program = calloc(1, sizeof(Program));
    program->type = PROGRAM_EXPRESSION;
    program->expression = expression;
    state->abstractSyntaxtTree = program;
    state->succeed = (flexCurrentContext() == 0);
    return program;
}


Program* MultiStatementProgramSemanticAction(CompilerState* state, Program* program, Statement* statement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    StatementList* new_node = calloc(1, sizeof(StatementList));
    new_node->statement = statement;
    new_node->next = program->statements;
    program->statements = new_node;
    state->abstractSyntaxtTree = program;
    state->succeed = true;
    return program;
}

/* ==================== CAMPOS ==================== */

Field* FieldSemanticAction(char* name, char* protocol) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Field* field = calloc(1, sizeof(Field));
    printf("[CREATE] Field @ %p (protocol='%s', name='%s')\n", field, protocol, name);
    field->name = name;
    field->protocol = protocol;
    return field;
}


FieldList* createFieldList(Field* field) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    FieldList* list = calloc(1, sizeof(FieldList));
    list->field = field;
    list->next = NULL;
    return list;
}

FieldList* appendToFieldList(FieldList* list, Field* newField) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    FieldList* new_node = calloc(1, sizeof(FieldList));
    new_node->field = newField;
    new_node->next = NULL;
    
    if (!list) return new_node;
    
    FieldList* current = list;
    while (current->next) {
        current = current->next;
    }
    current->next = new_node;
    return list;
}

/* ==================== CONDICIONES ==================== */

Condition* ComparisonConditionSemanticAction(Expression* left, Expression* right, ComparisonOperator op) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* cond = calloc(1, sizeof(Condition));
    cond->type = COMPARISON;
    cond->comparison.left = left;
    cond->comparison.right = right;
    cond->comparison.op = op;
    return cond;
}

Condition* LogicalConditionSemanticAction(Condition* left, Condition* right, LogicalOperator op) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* cond = calloc(1, sizeof(Condition));
    cond->type = (op == AND_OP) ? LOGICAL_AND : LOGICAL_OR;
    cond->logical.left = left;
    cond->logical.right = right;
    cond->logical.op = op;
    return cond;
}

Condition* SameConditionSemanticAction(Field* field) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* cond = calloc(1, sizeof(Condition));
    cond->type = PC_SAME;
    cond->pattern.type = PC_SAME;
    cond->pattern.same.field = field;
    return cond;
}

Condition* DifferentConditionSemanticAction(Field* field) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* cond = calloc(1, sizeof(Condition));
    cond->type = PC_DIFFERENT;
    cond->pattern.type = PC_DIFFERENT;
    cond->pattern.different.field = field;
    return cond;
}



Condition* EmptyHavingClauseSemanticAction() {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return NULL;
}

Condition* HavingClauseSemanticAction(Condition* condition) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return condition;
}

/* ==================== STATEMENTS ==================== */

Statement* CaptureStatementSemanticAction(char* source, Condition* condition) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Statement* stmt = calloc(1, sizeof(Statement));
    stmt->type = CAPTURE_STATEMENT;
    stmt->capture.interface = source;
    stmt->capture.filter = condition;
    return stmt;
}

Statement* ExtractStatementSemanticAction(FieldList* fields, Condition* filter, FieldList* group_fields, Condition* having) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Statement* stmt = calloc(1, sizeof(Statement));
    stmt->type = EXTRACT_STATEMENT;
    stmt->extract.fields = fields;
    stmt->extract.filter = filter;
    stmt->group.group_fields = group_fields;
    stmt->group.having = having;              
    return stmt;
}

Expression* CountExpressionSemanticAction(Condition* inner) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return AggregationExpressionSemanticAction(inner, AGGREGATION_COUNT);
}

Condition* ExpressionConditionSemanticAction(Expression* expression) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Condition* cond = calloc(1, sizeof(Condition));
    cond->type = COMPARISON;
    cond->comparison.left = expression;
    cond->comparison.right = NULL; // Podés usar esto para distinguir que es una expr directa
    return cond;
}


Statement* FilterStatementSemanticAction(Condition* condition) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Statement* stmt = calloc(1, sizeof(Statement));
    stmt->type = FILTER_STATEMENT;
    stmt->filter.condition = condition;
    return stmt;
}

Statement* AlertStatementSemanticAction(Condition* trigger, char* message) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Statement* stmt = calloc(1, sizeof(Statement));
    stmt->type = ALERT_STATEMENT;
    stmt->alert.trigger = trigger;
    stmt->alert.message = message;
    return stmt;
}



Statement* DefineStatementSemanticAction(char* pattern_name, PatternCondition* conditions, int count) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Statement* stmt = calloc(1, sizeof(Statement));
    stmt->type = DEFINE_STATEMENT;
    stmt->define.pattern_name = pattern_name;
    stmt->define.conditions = malloc(count * sizeof(PatternCondition));
for (int i = 0; i < count; i++) {
    PatternCondition* src = &conditions[i];
    PatternCondition* dst = &stmt->define.conditions[i];
    dst->type = src->type;
    switch (src->type) {
        case PC_SAME:
            dst->same.field = src->same.field;
            break;
        case PC_DIFFERENT:
            dst->different.field = src->different.field;
            break;
       case PC_AGGREGATION:
    dst->aggregation.func = src->aggregation.func;
    dst->aggregation.field = src->aggregation.field;
    dst->aggregation.op = src->aggregation.op;
    dst->aggregation.value = src->aggregation.value;
    break;
        case PC_TIMESPAN:
            dst->timespan.op = src->timespan.op;
            dst->timespan.value = src->timespan.value;
            break;
    }
}    stmt->define.condition_count = count;
    return stmt;
}

Statement* ImportStatementSemanticAction(char* filename) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Statement* stmt = calloc(1, sizeof(Statement));
    stmt->type = IMPORT_STATEMENT;
    stmt->import_export.filename = filename;
    stmt->import_export.export_data = NULL;
    return stmt;
}

Statement* ExportStatementSemanticAction(char* data, char* filename) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Statement* stmt = calloc(1, sizeof(Statement));
    stmt->type = EXPORT_STATEMENT;
    stmt->import_export.filename = filename;
    stmt->import_export.export_data = data;
    return stmt;
}

/* ==================== WRAPPERS ==================== */

Statement* CaptureStatementSemanticActionWrapper(Statement* stmt) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return stmt;
}

Statement* ExtractStatementSemanticActionWrapper(Statement* stmt) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return stmt;
}

Statement* FilterStatementSemanticActionWrapper(Statement* stmt) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return stmt;
}

Statement* AlertStatementSemanticActionWrapper(Statement* stmt) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return stmt;
}

Statement* GroupStatementSemanticActionWrapper(Statement* stmt) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return stmt;
}

Statement* DefineStatementSemanticActionWrapper(Statement* stmt) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return stmt;
}

Statement* ImportExportStatementSemanticActionWrapper(Statement* stmt) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return stmt;
}

Field* createSimpleField(char* name, char* protocol) {
    Field* field = calloc(1, sizeof(Field));
    
    if(strcmp(name, "*") == 0) {
        field->name = strdup(name);
        field->protocol = NULL;
        return field;
    }
    field->name = name;
    field->protocol = NULL;
    //free(name);
    return field;
}

Factor* VariableFactorSemanticAction(char* name) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Factor* factor = calloc(1, sizeof(Factor));
    factor->type = VARIABLE_REFERENCE;
    factor->variable_name = name; 
    return factor;
}


VariableDeclaration* VariableDeclarationSemanticAction(char* identifier, Expression* value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    VariableDeclaration* decl = calloc(1, sizeof(VariableDeclaration));
    decl->identifier = identifier;
    decl->value = value; 
    return decl;
}


Statement* VariableDeclarationStatementSemanticAction(VariableDeclaration* decl) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Statement* stmt = calloc(1, sizeof(Statement));
    stmt->type = VARIABLE_DECLARATION_STATEMENT;
    stmt->variable_declaration = *decl;
    free(decl);
    return stmt;
}

Condition* CountPatternConditionExpressionSemanticAction(Field* field, ComparisonOperator op, Expression* value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    return AggregationPatternConditionSemanticAction(field, op, value, AGGREGATION_COUNT);
}

Condition* AggregationPatternConditionSemanticAction(Field* field, ComparisonOperator op, Expression* value, ExpressionType aggType) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    if (field && strcmp(field->name, "*") == 0 && aggType != AGGREGATION_COUNT) {
        logError(_logger, "Aggregation function %d cannot be applied to '*'", aggType);
        releaseField(field);
        releaseExpression(value);
        return NULL;
    }
    Condition* cond = calloc(1, sizeof(Condition));
    printf("[CREATE] AggregationCondition @ %p (Field @ %p, Expression @ %p, Type %d)\n", cond, field, value, aggType);
    cond->type = PATTERN_CONDITION;
    cond->pattern.type = PC_AGGREGATION;
    cond->pattern.aggregation.func = (aggType == AGGREGATION_COUNT) ? AGG_COUNT :
                                    (aggType == AGGREGATION_SUM) ? AGG_SUM :
                                    (aggType == AGGREGATION_AVG) ? AGG_AVG :
                                    (aggType == AGGREGATION_MIN) ? AGG_MIN : AGG_MAX;
    cond->pattern.aggregation.field = field;
    cond->pattern.aggregation.op = op;
    cond->pattern.aggregation.value = value;
    return cond;
}
Expression* AggregationExpressionSemanticAction(Condition* condition, ExpressionType aggType) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    Expression* expr = calloc(1, sizeof(Expression));
    expr->type = aggType;
    expr->aggregation = condition;
    return expr;
}