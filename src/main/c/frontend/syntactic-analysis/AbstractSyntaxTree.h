#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../shared/Logger.h"
#include <stdlib.h>

/** Initialize module's internal state. */
void initializeAbstractSyntaxTreeModule();

/** Shutdown module's internal state. */
void shutdownAbstractSyntaxTreeModule();

/**
 * This typedefs allows self-referencing types.
 */

typedef enum ExpressionType ExpressionType;
typedef enum FactorType FactorType;

typedef struct Constant Constant;
typedef struct Expression Expression;
typedef struct Factor Factor;
typedef struct Program Program;
typedef struct Statement Statement;
typedef struct FieldList FieldList;
typedef struct Condition Condition;


/**
 * Node types for the Abstract Syntax Tree (AST).
 */

 typedef enum StatementType {
	CAPTURE_STATEMENT,
	EXTRACT_STATEMENT,
	FILTER_STATEMENT
} StatementType;

typedef struct Statement {
	StatementType type;
	union {
		struct {
			char * source;
			struct Condition * condition;
		} capture;

		struct {
			struct FieldList * fields;
			char * source;
			struct Condition * condition;
		} extract;

		struct {
			struct Condition * condition;
		} filter;
	};
} Statement;

typedef struct FieldList {
	char * name;
	char * alias; // Puede ser NULL si no tiene alias
	struct FieldList * next;
} FieldList;

typedef enum ConditionType {
	COMPARISON,
	LOGICAL_AND,
	LOGICAL_OR
} ConditionType;

typedef enum ComparisonOperator {
	EQUALS_OP,
	NOT_EQUALS_OP,
	LESS_THAN_OP,
	GREATER_THAN_OP
} ComparisonOperator;

typedef enum LogicalOperator {
	AND_OP,
	OR_OP
} LogicalOperator;

typedef struct Condition {
	ConditionType type;
	union {
		struct {
			struct Expression * left;
			struct Expression * right;
			ComparisonOperator op;
		} comparison;

		struct {
			struct Condition * left;
			struct Condition * right;
			LogicalOperator op;
		} logical;
	};
} Condition;


enum ExpressionType {
	ADDITION,
	DIVISION,
	FACTOR,
	MULTIPLICATION,
	SUBTRACTION
};

enum FactorType {
	CONSTANT,
	EXPRESSION
};

struct Constant {
	int value;
};

struct Factor {
	union {
		Constant * constant;
		Expression * expression;
	};
	FactorType type;
};

struct Expression {
	union {
		Factor * factor;
		struct {
			Expression * leftExpression;
			Expression * rightExpression;
		};
	};
	ExpressionType type;
};

struct Program {
	Expression * expression;
};

/**
 * Node recursive destructors.
 */
void releaseConstant(Constant * constant);
void releaseExpression(Expression * expression);
void releaseFactor(Factor * factor);
void releaseProgram(Program * program);

#endif
