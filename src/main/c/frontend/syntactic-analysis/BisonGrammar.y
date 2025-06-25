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
    GroupStatement groupStatement;
    ComparisonOperator comparisonOperator;
    VariableDeclaration* variableDeclaration;
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


%token <token> INT_TYPE

/** Data operation tokens */
%token <token> EXTRACT CAPTURE FILTER GROUP COUNT WHERE HAVING ORDER BY FROM 
%token <token> SUM AVG MIN MAX

/** Alert tokens */
%token <token> ALERT WHEN SEND TO

/** Comparison tokens */
%token <token> SAME DIFFERENT

/** Definition tokens */
%token <token> DEFINE IMPORT EXPORT TIMESPAN 
%token <token> MATCHES


/** Data type tokens */
%token <token> PACKET_TYPE ADDRESS_TYPE BOOLEAN TIMESTAMP

/** Operator tokens */
%token <token> ADD SUB MUL DIV
%token <token> ASSIGN EQUALS NOT_EQUALS LESS_THAN GREATER_THAN
%token <token> AND OR
%token <token> GREATER_THAN_OR_EQUALS LESS_THAN_OR_EQUALS



/** Delimiter tokens */
%token <token> OPEN_PARENTHESIS CLOSE_PARENTHESIS
%token <token> OPEN_BRACE CLOSE_BRACE
%token <token> SEMICOLON COMMA DOT

/** Error token */
%token <token> UNKNOWN

/** Non-terminals */
%type <program> program
%type <statement> statement capture_statement extract_statement filter_statement
%type <statement> alert_statement define_statement import_statement export_statement
%type <condition> condition having_clause pattern_conditions pattern_condition
%type <condition> comparison_expression logical_expression
%type <expression> expression
%type <factor> factor
%type <constant> constant
%type <fieldList> field_list
%type <field> field
%type <condition> where_clause
%type <groupStatement> group_clause
%type <condition> aggregation_pattern
%type <comparisonOperator> comparison_operator
%type <variableDeclaration> variable_declaration

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
    | import_statement { $$ = ImportExportStatementSemanticActionWrapper($1); }
    | variable_declaration { $$ = VariableDeclarationStatementSemanticAction($1); }
    | extract_statement { $$ = ExtractStatementSemanticActionWrapper($1); }
    | filter_statement { $$ = FilterStatementSemanticActionWrapper($1); }
    | alert_statement { $$ = AlertStatementSemanticActionWrapper($1); }
    | define_statement { $$ = DefineStatementSemanticActionWrapper($1); }
    | export_statement { $$ = ImportExportStatementSemanticActionWrapper($1); }
    ;

alert_statement:
ALERT WHEN condition SEND TO STRING SEMICOLON {
    $$ = AlertStatementSemanticAction($3, $6);
}
;

variable_declaration:
    INT_TYPE IDENTIFIER ASSIGN expression SEMICOLON {
        $$ = VariableDeclarationSemanticAction($2, $4);
    }
    ;

comparison_operator:
      EQUALS        { $$ = EQUALS_OP; }
    | NOT_EQUALS    { $$ = NOT_EQUALS_OP; }
    | LESS_THAN     { $$ = LESS_THAN_OP; }
    | LESS_THAN_OR_EQUALS { $$ = LESS_THAN_OR_EQUALS_OP; }
    | GREATER_THAN  { $$ = GREATER_THAN_OP; }
    | GREATER_THAN_OR_EQUALS { $$ = GREATER_THAN_OR_EQUALS_OP; }
;


aggregation_pattern:
    COUNT OPEN_PARENTHESIS field CLOSE_PARENTHESIS comparison_operator expression {
        $$ = AggregationPatternConditionSemanticAction($3, $5, $6, AGGREGATION_COUNT);
    }
    | SUM OPEN_PARENTHESIS field CLOSE_PARENTHESIS comparison_operator expression {
        $$ = AggregationPatternConditionSemanticAction($3, $5, $6, AGGREGATION_SUM);
    }
    | AVG OPEN_PARENTHESIS field CLOSE_PARENTHESIS comparison_operator expression {
        $$ = AggregationPatternConditionSemanticAction($3, $5, $6, AGGREGATION_AVG);
    }
    | MIN OPEN_PARENTHESIS field CLOSE_PARENTHESIS comparison_operator expression {
        $$ = AggregationPatternConditionSemanticAction($3, $5, $6, AGGREGATION_MIN);
    }
    | MAX OPEN_PARENTHESIS field CLOSE_PARENTHESIS comparison_operator expression {
        $$ = AggregationPatternConditionSemanticAction($3, $5, $6, AGGREGATION_MAX);
    }
    ;

define_statement:
    DEFINE IDENTIFIER OPEN_BRACE pattern_conditions CLOSE_BRACE SEMICOLON {
        int condition_count = countConditions($4);
        PatternCondition* patterns = convertConditionsToPatterns($4); 
        $$ = DefineStatementSemanticAction($2, patterns, condition_count);
        free(patterns);
    }
    ;

pattern_condition:
    SAME field { $$ = SamePatternConditionSemanticAction($2); }
    | DIFFERENT field { $$ = DifferentPatternConditionSemanticAction($2); }
    | comparison_expression { $$ = $1; }  
    | aggregation_pattern { $$ = $1; }
    | TIMESPAN comparison_operator expression { $$ = TimespanPatternConditionSemanticAction($2, $3); }
    ;

pattern_conditions:
    pattern_condition { 
        $$ = $1; 
    }
    | pattern_conditions COMMA pattern_condition { 
        $$ = MultiplePatternConditionsSemanticAction($1, $3); 
    }
    ;


import_statement:
     IMPORT FROM STRING SEMICOLON { 
        $$ = ImportStatementSemanticAction($3); 
    }
    ;

export_statement:
        EXPORT STRING TO STRING SEMICOLON { 
        $$ = ExportStatementSemanticAction($2, $4); 
    }
    ;

field:
    IDENTIFIER { $$ = FieldSemanticAction(NULL, $1); }
    | IDENTIFIER DOT IDENTIFIER { $$ = FieldSemanticAction($1, $3); }
    ;

condition:
    comparison_expression { $$ = $1; }
    | logical_expression { $$ = $1; }
    | OPEN_PARENTHESIS condition CLOSE_PARENTHESIS { $$ = ParenthesizedConditionSemanticAction($2); }
    | aggregation_pattern { $$ = $1; }
   | COUNT OPEN_PARENTHESIS MUL CLOSE_PARENTHESIS comparison_operator expression {
        $$ = AggregationPatternConditionSemanticAction(createSimpleField("*", NULL), $5, $6, AGGREGATION_COUNT);
    }
    | SUM OPEN_PARENTHESIS MUL CLOSE_PARENTHESIS comparison_operator expression {
        $$ = AggregationPatternConditionSemanticAction(createSimpleField("*", NULL), $5, $6, AGGREGATION_SUM);
    }
    | AVG OPEN_PARENTHESIS MUL CLOSE_PARENTHESIS comparison_operator expression {
        $$ = AggregationPatternConditionSemanticAction(createSimpleField("*", NULL), $5, $6, AGGREGATION_AVG);
    }
    | MIN OPEN_PARENTHESIS MUL CLOSE_PARENTHESIS comparison_operator expression {
        $$ = AggregationPatternConditionSemanticAction(createSimpleField("*", NULL), $5, $6, AGGREGATION_MIN);
    }
    | MAX OPEN_PARENTHESIS MUL CLOSE_PARENTHESIS comparison_operator expression {
        $$ = AggregationPatternConditionSemanticAction(createSimpleField("*", NULL), $5, $6, AGGREGATION_MAX);
    }
    | SAME field { $$ = SameConditionSemanticAction($2); }
    | DIFFERENT field { $$ = DifferentConditionSemanticAction($2); }
    | TIMESPAN comparison_operator expression { $$ = TimespanPatternConditionSemanticAction($2, $3); }
    ;

capture_statement: 
    CAPTURE FROM STRING WHERE condition SEMICOLON {
        $$ = CaptureStatementSemanticAction($3, $5); 
    }
    ;

group_clause:
    GROUP BY field_list having_clause {
        GroupStatement gs;
        gs.group_fields = $3;
        gs.having = $4;
        $$ = gs;
    }
    | %empty {
        GroupStatement gs;
        gs.group_fields = NULL;
        gs.having = NULL;
        $$ = gs;
    }
    ;


having_clause: 
     %empty { $$ = NULL; }
    | HAVING condition { $$ = $2; }
    ;

where_clause: 
    %empty           { $$ = NULL; }
    | WHERE condition { $$ = $2; }
    ;

extract_statement:
    EXTRACT OPEN_BRACE field_list CLOSE_BRACE where_clause group_clause SEMICOLON {
        $$ = ExtractStatementSemanticAction($3, $5, $6.group_fields, $6.having);
    }
    ;



filter_statement: 
    FILTER condition SEMICOLON { 
        $$ = FilterStatementSemanticAction($2); 
    }
    | FILTER OPEN_BRACE condition CLOSE_BRACE SEMICOLON {
        $$ = FilterStatementSemanticAction($3);
    }
    | FILTER MATCHES IDENTIFIER SEMICOLON {
        Condition* cond = IdentifierConditionSemanticAction($3);
        $$ = FilterStatementSemanticAction(cond);
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
    expression comparison_operator expression { $$ = ComparisonSemanticAction($1, $3, $2); }
    ;

expression: 
    expression ADD expression { $$ = ArithmeticExpressionSemanticAction($1, $3, ADDITION); }
    | expression DIV expression { $$ = ArithmeticExpressionSemanticAction($1, $3, DIVISION); }
    | expression MUL expression { $$ = ArithmeticExpressionSemanticAction($1, $3, MULTIPLICATION); }
    | expression SUB expression { $$ = ArithmeticExpressionSemanticAction($1, $3, SUBTRACTION); }
    | factor { $$ = FactorExpressionSemanticAction($1); }
    | COUNT OPEN_PARENTHESIS comparison_expression CLOSE_PARENTHESIS {
        $$ = AggregationExpressionSemanticAction($3, AGGREGATION_COUNT);
    }
    | SUM OPEN_PARENTHESIS comparison_expression CLOSE_PARENTHESIS {
        $$ = AggregationExpressionSemanticAction($3, AGGREGATION_SUM);
    }
    | AVG OPEN_PARENTHESIS comparison_expression CLOSE_PARENTHESIS {
        $$ = AggregationExpressionSemanticAction($3, AGGREGATION_AVG);
    }
    | MIN OPEN_PARENTHESIS comparison_expression CLOSE_PARENTHESIS {
        $$ = AggregationExpressionSemanticAction($3, AGGREGATION_MIN);
    }
    | MAX OPEN_PARENTHESIS comparison_expression CLOSE_PARENTHESIS {
        $$ = AggregationExpressionSemanticAction($3, AGGREGATION_MAX);
    }
    ;

factor: 
    OPEN_PARENTHESIS expression CLOSE_PARENTHESIS { $$ = ExpressionFactorSemanticAction($2); }
    | constant { $$ = ConstantFactorSemanticAction($1); }
    | IDENTIFIER { 
        $$ = VariableFactorSemanticAction($1); 
    }
    | IDENTIFIER DOT IDENTIFIER { 
        Field* f = FieldSemanticAction($1, $3); 
        $$ = FieldFactorSemanticAction(f); 
    }
    ;

constant: 
    INTEGER { $$ = IntegerConstantSemanticAction($1); }
    | STRING { $$ = StringConstantSemanticAction($1); }
    | TIMESTAMP { $$ = TimestampConstantSemanticAction($1); }
    | BOOLEAN { $$ = BooleanConstantSemanticAction($1); }
    ;

%%
