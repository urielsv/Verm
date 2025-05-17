#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../shared/Logger.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/** Initialize module's internal state. */
void initializeAbstractSyntaxTreeModule();

/** Shutdown module's internal state. */
void shutdownAbstractSyntaxTreeModule();

/**
 * Forward declarations
 */
typedef struct Constant Constant;
typedef struct Expression Expression;
typedef struct Factor Factor;
typedef struct Program Program;
typedef struct Statement Statement;
typedef struct FieldList FieldList;
typedef struct Condition Condition;
typedef struct Field Field;

/**
 * Operator types
 */
 typedef enum ComparisonOperator {
    EQUALS_OP,
    NOT_EQUALS_OP,
    LESS_THAN_OP,
    GREATER_THAN_OP,
    GREATER_THAN_OR_EQUALS_OP,
    LESS_THAN_OR_EQUALS_OP
} ComparisonOperator;

typedef enum LogicalOperator {
    AND_OP,
    OR_OP
} LogicalOperator;

typedef enum ConditionType {
    COMPARISON = 0,
    LOGICAL_AND,
    LOGICAL_OR,
    LOGICAL_NOT,
    PATTERN_CONDITION
} ConditionType;

typedef enum ExpressionType {
    ADDITION,
    DIVISION,
    FACTOR,
    MULTIPLICATION,
    SUBTRACTION,
    AGGREGATION_COUNT,
    AGGREGATION_SUM,
    AGGREGATION_AVG,
    AGGREGATION_MIN,
    AGGREGATION_MAX
} ExpressionType;

typedef enum FactorType {
    CONSTANT,
    EXPRESSION,
    FIELD,
    VARIABLE_REFERENCE
} FactorType;

/**
 * Node types for the Abstract Syntax Tree (AST).
 */
typedef enum StatementType {
    CAPTURE_STATEMENT,
    EXTRACT_STATEMENT,
    FILTER_STATEMENT,
    ALERT_STATEMENT,
    DEFINE_STATEMENT,
    IMPORT_STATEMENT,
    EXPORT_STATEMENT,
    VARIABLE_DECLARATION_STATEMENT
} StatementType;

typedef enum ValueType {
    INTEGER_TYPE,
    STRING_TYPE,
    BOOLEAN_TYPE,
    TIMESTAMP_TYPE
} ValueType;


typedef struct {
    ValueType type;
    union {
        int integer;
        char* string;
        bool boolean;
        time_t timestamp;
    };
} Value;

typedef struct {
    char* identifier;
    Expression* value;
} VariableDeclaration;

struct Constant {
        Value value;   
};

struct Factor {
    union {
        Constant* constant;
        Expression* expression;
        Field* field;
        char* variable_name;
    };
    FactorType type;
};

struct Expression {
    ExpressionType type;
    union {
        Factor* factor;
        struct {
            Expression* leftExpression;  // Nombre que espera el generador
            Expression* rightExpression; // Nombre que espera el generador
        };
        Condition* aggregation; 
    };
};

typedef struct {
    Condition* trigger;
    char* message;
} AlertStatement;

typedef struct {
    FieldList* group_fields;
    Condition* having;
} GroupStatement;

typedef enum {
    PC_SAME = 6, 
    PC_DIFFERENT,
    PC_AGGREGATION, 
    PC_TIMESPAN
} PatternConditionType;

typedef enum {
    AGG_COUNT,
    AGG_SUM,
    AGG_AVG,
    AGG_MIN,
    AGG_MAX
} AggregationFunction;

typedef struct {
    PatternConditionType type;
    union {
        struct { Field* field; } same;
        struct { Field* field; } different;
       struct {
        AggregationFunction func;
        Field* field;
        ComparisonOperator op;
        Expression* value;
        } aggregation;
        struct {
            ComparisonOperator op;
            Expression* value; 
        } timespan;
    };
} PatternCondition;

typedef struct {
    char* pattern_name;
    PatternCondition* conditions;
    int condition_count;
} DefineStatement;

typedef struct {
    char* filename;
    char* export_data;
} ImportExportStatement;

typedef struct {
    char* interface;
    Condition* filter;
} CaptureStatement;

typedef struct {
    FieldList* fields;
    Condition* filter;   
} ExtractStatement;

typedef struct {
    Condition* condition;
} FilterStatement;


typedef struct Field {
    char* protocol;  
    char* name;

} Field;

typedef struct FieldList {
    Field* field;
    struct FieldList* next;
} FieldList;

struct Condition {
    ConditionType type;
    union {
        struct {
            Expression* left;
            Expression* right;
            ComparisonOperator op;
        } comparison;
        struct {
            Condition* left;
            Condition* right;
            LogicalOperator op;
        } logical;
		PatternCondition pattern;
    };
};

typedef struct Statement {
    StatementType type;
    union {
        CaptureStatement capture;
        ExtractStatement extract;
        FilterStatement filter;
        AlertStatement alert;
        DefineStatement define;
        ImportExportStatement import_export;
        VariableDeclaration variable_declaration;
    };
    GroupStatement group;  
} Statement;

typedef struct StatementList {
    Statement* statement;
    struct StatementList* next;
} StatementList;

typedef enum {
    PROGRAM_STATEMENTS,
    PROGRAM_EXPRESSION
} ProgramType;

typedef struct Program {
    ProgramType type;
    union {
        StatementList* statements;
        Expression* expression;
    };
} Program;

/**
 * Node recursive destructors.
 */
void releaseConstant(Constant* constant);
void releaseExpression(Expression* expression);
void releaseFactor(Factor* factor);
void releaseProgram(Program* program);
void releaseStatement(Statement* statement);
void releaseFieldList(FieldList* fieldList);
void releaseCondition(Condition* condition);
void releaseField(Field* field);

#endif