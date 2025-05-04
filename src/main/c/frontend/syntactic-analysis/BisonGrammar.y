%{

#include "BisonActions.h"

%}

// You touch this, and you die.
%define api.value.union.name SemanticValue
%union {
    /** Terminals */
    char* string;
    int integer;
    Token token;

    /** Non-terminals */
    Constant* constant;
    Expression* expression;
    Factor* factor;
    Program* program;
    Statement* statement;
    Field* field;
    FieldList* fieldList;
    Condition* condition;
}

/** Destructors */
%destructor { releaseConstant($$); } <constant>
%destructor { releaseExpression($$); } <expression>
%destructor { releaseFactor($$); } <factor>
%destructor { releaseField($$); } <field>
%destructor { free($$); } <string>
%destructor { releaseStatement($$); } <statement>
%destructor { releaseFieldList($$); } <fieldList>
%destructor { releaseCondition($$); } <condition>

/** Tokens */
%token <integer> INTEGER
%token <string> IDENTIFIER STRING

/** Protocol tokens */
%token <token> PROTOCOL PROTO_HTTP PROTO_TCP PROTO_UDP PROTO_DNS

/** Data operation tokens */
%token <token> EXTRACT CAPTURE FILTER GROUP COUNT WHERE HAVING ORDER BY FROM AS

/** Alert tokens */
%token <token> ALERT WHEN SEND TO

/** Comparison tokens */
%token <token> SAME DIFFERENT

/** Definition tokens */
%token <token> DEFINE IMPORT EXPORT TABLE TIMESPAN

/** Control flow tokens */
%token <token> IF ELSE WHILE

/** Data type tokens */
%token <token> PACKET_TYPE ADDRESS_TYPE BOOLEAN TIMESTAMP

/** Operator tokens */
%token <token> ADD SUB MUL DIV
%token <token> ASSIGN EQUALS NOT_EQUALS LESS_THAN GREATER_THAN
%token <token> AND OR

/** Delimiter tokens */
%token <token> OPEN_PARENTHESIS CLOSE_PARENTHESIS
%token <token> OPEN_BRACE CLOSE_BRACE
%token <token> SEMICOLON COMMA DOT

/** Error token */
%token <token> UNKNOWN

/** Non-terminals */
%type <program> program
%type <statement> statement capture_statement extract_statement filter_statement
%type <statement> alert_statement group_statement define_statement import_export_statement
%type <condition> condition having_clause pattern_conditions pattern_condition
%type <condition> comparison_expression logical_expression
%type <expression> expression
%type <factor> factor
%type <constant> constant
%type <fieldList> field_list
%type <field> field

/** Precedence and associativity */
%left COMMA
%right ASSIGN
%left OR
%left AND
%left EQUALS NOT_EQUALS
%left LESS_THAN GREATER_THAN
%left ADD SUB
%left MUL DIV
%left OPEN_PARENTHESIS

%%

program: 
    statement { 
        $$ = StatementProgramSemanticAction(currentCompilerState(), $1); 
    }
    | program statement { 
        $$ = MultiStatementProgramSemanticAction(currentCompilerState(), $1, $2); 
    }
    ;

statement:
    capture_statement { $$ = CaptureStatementSemanticActionWrapper($1); }
    | extract_statement { $$ = ExtractStatementSemanticActionWrapper($1); }
    | filter_statement { $$ = FilterStatementSemanticActionWrapper($1); }
    | alert_statement { $$ = AlertStatementSemanticActionWrapper($1); }
    | group_statement { $$ = GroupStatementSemanticActionWrapper($1); }
    | define_statement { $$ = DefineStatementSemanticActionWrapper($1); }
    | import_export_statement { $$ = ImportExportStatementSemanticActionWrapper($1); }
    ;

alert_statement:
    ALERT WHEN condition SEND STRING SEMICOLON {
        $$ = AlertStatementSemanticAction($3, $5); 
    }
    ;

group_statement:
    GROUP BY field_list having_clause SEMICOLON {
        $$ = GroupStatementSemanticAction($3, $4); 
    }
    ;

having_clause:
    %empty { $$ = EmptyHavingClauseSemanticAction(); }
    | HAVING condition { $$ = HavingClauseSemanticAction($2); }
    ;

define_statement:
    DEFINE IDENTIFIER OPEN_BRACE pattern_conditions CLOSE_BRACE SEMICOLON {
        PatternCondition* patterns = convertConditionsToPatterns($4);
        $$ = DefineStatementSemanticAction($2, patterns, countConditions($4));
        free(patterns);
    }
    ;

pattern_condition:
    SAME IDENTIFIER { $$ = SamePatternConditionSemanticAction($2); }
    | DIFFERENT IDENTIFIER { $$ = DifferentPatternConditionSemanticAction($2); }
    | COUNT OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS EQUALS INTEGER {
        $$ = CountPatternConditionSemanticAction($3, EQUALS_OP, $6); 
    }
    | COUNT OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS NOT_EQUALS INTEGER {
        $$ = CountPatternConditionSemanticAction($3, NOT_EQUALS_OP, $6); 
    }
    | COUNT OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS LESS_THAN INTEGER {
        $$ = CountPatternConditionSemanticAction($3, LESS_THAN_OP, $6); 
    }
    | COUNT OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS GREATER_THAN INTEGER {
        $$ = CountPatternConditionSemanticAction($3, GREATER_THAN_OP, $6); 
    }
    | TIMESPAN EQUALS INTEGER { $$ = TimespanConditionSemanticAction(EQUALS_OP, $3); }
    | TIMESPAN NOT_EQUALS INTEGER { $$ = TimespanConditionSemanticAction(NOT_EQUALS_OP, $3); }
    | TIMESPAN LESS_THAN INTEGER { $$ = TimespanConditionSemanticAction(LESS_THAN_OP, $3); }
    | TIMESPAN GREATER_THAN INTEGER { $$ = TimespanConditionSemanticAction(GREATER_THAN_OP, $3); }
    ;

pattern_conditions:
    pattern_condition { 
        $$ = $1; 
    }
    | pattern_conditions COMMA pattern_condition { 
        $$ = MultiplePatternConditionsSemanticAction($1, $3); 
    }
    ;

import_export_statement:
    IMPORT FROM STRING SEMICOLON { 
        $$ = ImportStatementSemanticAction($3); 
    }
    | EXPORT STRING TO STRING SEMICOLON { 
        $$ = ExportStatementSemanticAction($2, $4); 
    }
    ;

field:
    IDENTIFIER { $$ = FieldSemanticAction($1, NULL); }
    | IDENTIFIER DOT IDENTIFIER { $$ = FieldSemanticAction($1, $3); }
    | IDENTIFIER AS IDENTIFIER { $$ = FieldSemanticActionWithAlias($1, $3); }
    | IDENTIFIER DOT IDENTIFIER AS IDENTIFIER { 
        $$ = FieldSemanticActionWithFullAlias($1, $3, $5); 
    }
    ;

condition:
    comparison_expression { $$ = $1; }
    | logical_expression { $$ = $1; }
    | OPEN_PARENTHESIS condition CLOSE_PARENTHESIS { $$ = ParenthesizedConditionSemanticAction($2); }
    | IDENTIFIER { $$ = IdentifierConditionSemanticAction($1); }
    | IDENTIFIER DOT IDENTIFIER { $$ = FieldConditionSemanticAction($1, $3); }
    | COUNT OPEN_PARENTHESIS condition CLOSE_PARENTHESIS {
        $$ = CountConditionSemanticAction($3, EQUALS_OP, 0); 
    }
    | SAME IDENTIFIER { $$ = SameConditionSemanticAction($2); }
    | DIFFERENT IDENTIFIER { $$ = DifferentConditionSemanticAction($2); }
    | TIMESPAN EQUALS INTEGER { $$ = TimespanPatternConditionSemanticAction(EQUALS_OP, $3); }
    | TIMESPAN NOT_EQUALS INTEGER { $$ = TimespanPatternConditionSemanticAction(NOT_EQUALS_OP, $3); }
    | TIMESPAN LESS_THAN INTEGER { $$ = TimespanPatternConditionSemanticAction(LESS_THAN_OP, $3); }
    | TIMESPAN GREATER_THAN INTEGER { $$ = TimespanPatternConditionSemanticAction(GREATER_THAN_OP, $3); }
    ;

capture_statement: 
    CAPTURE FROM STRING WHERE condition SEMICOLON {
        $$ = CaptureStatementSemanticAction($3, $5); 
    }
    ;

extract_statement: 
    EXTRACT OPEN_BRACE field_list CLOSE_BRACE FROM STRING WHERE condition SEMICOLON {
        $$ = ExtractStatementSemanticAction($3, $6, $8); 
    }
    ;

filter_statement: 
    FILTER condition SEMICOLON { 
        $$ = FilterStatementSemanticAction($2); 
    }
    ;

field_list: 
    field { $$ = createFieldList($1); }
    | field_list COMMA field { $$ = appendToFieldList($1, $3); }
    ;

logical_expression:
    condition AND condition { $$ = LogicalConditionSemanticAction($1, $3, AND_OP); }
    | condition OR condition { $$ = LogicalConditionSemanticAction($1, $3, OR_OP); }
    ;

comparison_expression: 
    expression EQUALS expression { $$ = ComparisonSemanticAction($1, $3, EQUALS_OP); }
    | expression NOT_EQUALS expression { $$ = ComparisonSemanticAction($1, $3, NOT_EQUALS_OP); }
    | expression LESS_THAN expression { $$ = ComparisonSemanticAction($1, $3, LESS_THAN_OP); }
    | expression GREATER_THAN expression { $$ = ComparisonSemanticAction($1, $3, GREATER_THAN_OP); }
    ;

expression: 
    expression ADD expression { $$ = ArithmeticExpressionSemanticAction($1, $3, ADDITION); }
    | expression DIV expression { $$ = ArithmeticExpressionSemanticAction($1, $3, DIVISION); }
    | expression MUL expression { $$ = ArithmeticExpressionSemanticAction($1, $3, MULTIPLICATION); }
    | expression SUB expression { $$ = ArithmeticExpressionSemanticAction($1, $3, SUBTRACTION); }
    | factor { $$ = FactorExpressionSemanticAction($1); }
    ;

factor: 
    OPEN_PARENTHESIS expression CLOSE_PARENTHESIS { $$ = ExpressionFactorSemanticAction($2); }
    | constant { $$ = ConstantFactorSemanticAction($1); }
    | IDENTIFIER DOT IDENTIFIER { 
        Field* f = FieldSemanticAction($1, $3);
        $$ = FieldFactorSemanticAction(f);
    }
    | ADDRESS_TYPE STRING { $$ = AddressTypeFactorSemanticAction($2); }
    | PACKET_TYPE STRING { $$ = PacketTypeFactorSemanticAction($2); }
    ;

constant: 
    INTEGER { $$ = IntegerConstantSemanticAction($1); }
    | STRING { $$ = StringConstantSemanticAction($1); }
    | TIMESTAMP { $$ = TimestampConstantSemanticAction($1); }
    | BOOLEAN { $$ = BooleanConstantSemanticAction($1); }
    ;

%%
