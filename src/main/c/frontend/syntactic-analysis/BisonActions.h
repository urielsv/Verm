#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../shared/CompilerState.h"
#include "../../shared/Logger.h"
#include "../../shared/Type.h"
#include "AbstractSyntaxTree.h"
#include "SyntacticAnalyzer.h"
#include <stdlib.h>

/** Initialize module's internal state. */
void initializeBisonActionsModule();

/** Shutdown module's internal state. */
void shutdownBisonActionsModule();

/**
 * Bison semantic actions.
 */

Constant * IntegerConstantSemanticAction(const int value);
Expression * ArithmeticExpressionSemanticAction(Expression * leftExpression, Expression * rightExpression, ExpressionType type);
Expression * FactorExpressionSemanticAction(Factor * factor);
Factor * ConstantFactorSemanticAction(Constant * constant);
Factor * ExpressionFactorSemanticAction(Expression * expression);
Program * ExpressionProgramSemanticAction(CompilerState * compilerState, Expression * expression);

Constant * StringConstantSemanticAction(char * value);
Constant * BooleanConstantSemanticAction(bool value);
Constant * TimestampConstantSemanticAction(time_t value);
Constant * PacketConstantSemanticAction(char * data, size_t length);

/* Expressions and Factors */
Factor * AddressTypeFactorSemanticAction(char * address);
Factor * PacketTypeFactorSemanticAction(char * data);
Factor * FieldFactorSemanticAction(Field * field);

/* Programs */
Program * StatementProgramSemanticAction(CompilerState * state, Statement * statement);
Program * MultiStatementProgramSemanticAction(CompilerState * state, Program * program, Statement * statement);

/* Fields */
Field * FieldSemanticAction(char * name, char * protocol);
FieldList * createFieldList(Field * field);
FieldList * appendToFieldList(FieldList * list, Field * newField);

/* Conditions */
Condition * ComparisonConditionSemanticAction(Expression * left, Expression * right, ComparisonOperator op);
Condition * LogicalConditionSemanticAction(Condition * left, Condition * right, LogicalOperator op);
Condition * SameConditionSemanticAction(Field * field);
Condition * DifferentConditionSemanticAction(Field * field);
Condition* TimespanPatternConditionSemanticAction(ComparisonOperator op, Expression* value);
Condition * EmptyHavingClauseSemanticAction();
Condition * HavingClauseSemanticAction(Condition * condition);
Condition * ParenthesizedConditionSemanticAction(Condition * condition);
Condition * IdentifierConditionSemanticAction(char * identifier);
Condition * FieldConditionSemanticAction(char * field1, char * field2);

/* Statements */
Statement * CaptureStatementSemanticAction(char * source, Condition * condition);
Statement* ExtractStatementSemanticAction(FieldList* fields, Condition* where, FieldList* groupFields, Condition* having);
Statement * FilterStatementSemanticAction(Condition * condition);
Statement * AlertStatementSemanticAction(Condition * trigger, char * message);
Statement * GroupStatementSemanticAction(FieldList * group_fields, Condition * having);
Statement * DefineStatementSemanticAction(char * pattern_name, PatternCondition * conditions, int count);
Statement * ImportStatementSemanticAction(char * filename);
Statement * ExportStatementSemanticAction(char * data, char * filename);

Condition * SamePatternConditionSemanticAction(Field * field);
Condition * DifferentPatternConditionSemanticAction(Field * field);
Condition * CountPatternConditionSemanticAction(Field * field, ComparisonOperator op, int value);
Condition * MultiplePatternConditionsSemanticAction(Condition * conditions, Condition * newCondition);
Condition * ComparisonSemanticAction(Expression * left, Expression * right, ComparisonOperator op);



Expression* CountExpressionSemanticAction(Condition* inner);

Condition* ExpressionConditionSemanticAction(Expression* expression);

/* Wrappers */
Statement * CaptureStatementSemanticActionWrapper(Statement * stmt);
Statement * ExtractStatementSemanticActionWrapper(Statement * stmt);
Statement * FilterStatementSemanticActionWrapper(Statement * stmt);
Statement * AlertStatementSemanticActionWrapper(Statement * stmt);
Statement * GroupStatementSemanticActionWrapper(Statement * stmt);
Statement * DefineStatementSemanticActionWrapper(Statement * stmt);
Statement * ImportExportStatementSemanticActionWrapper(Statement * stmt);

Condition* CountPatternConditionExpressionSemanticAction(Field* field, ComparisonOperator op, Expression* value);

Factor* VariableFactorSemanticAction(char* name);
VariableDeclaration* VariableDeclarationSemanticAction(char* identifier, Expression* value);
Statement* VariableDeclarationStatementSemanticAction(VariableDeclaration* decl);

Condition* AggregationPatternConditionSemanticAction(Field* field, ComparisonOperator op, Expression* value, ExpressionType aggType);
Expression* AggregationExpressionSemanticAction(Condition* condition, ExpressionType aggType);

/* utils */
PatternCondition* convertConditionsToPatterns(Condition* cond); 
int countConditions(Condition* cond);
Field* createSimpleField(char* name, char* protocol);
#endif
