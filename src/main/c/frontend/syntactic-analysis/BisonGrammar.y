%{

#include "BisonActions.h"

%}

// You touch this, and you die.
%define api.value.union.name SemanticValue

%union {
	/** Terminals. */
	
	char * string;
	int integer;
	Token token;

	/** Non-terminals. */

	Constant * constant;
	Expression * expression;
	Factor * factor;
	Program * program;
	Statement * statement;
	FieldList * fieldList;
	Condition * condition;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parse succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
%destructor { releaseConstant($$); } <constant>
%destructor { releaseExpression($$); } <expression>
%destructor { releaseFactor($$); } <factor>
%destructor { free($$); } <string>
%destructor { releaseStatement($$); } <statement>
%destructor { releaseFieldList($$); } <fieldList>
%destructor { releaseCondition($$); } <condition>

/** Terminals. */
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
%token <token> PACKET_TYPE ADDRESS_TYPE

/** Operator tokens */
%token <token> ADD SUB MUL DIV
%token <token> ASSIGN EQUALS NOT_EQUALS LESS_THAN GREATER_THAN
%token <token> AND OR

/** Delimiter tokens */
%token <token> OPEN_PARENTHESIS CLOSE_PARENTHESIS
%token <token> OPEN_BRACE CLOSE_BRACE
%token <token> SEMICOLON COMMA

/** Error token */
%token <token> UNKNOWN

/** Non-terminals. */
%type <program> program
%type <statement> statement capture_statement extract_statement filter_statement
%type <fieldList> field_list field
%type <condition> condition comparison_expression logical_expression
%type <expression> expression
%type <factor> factor
%type <constant> constant

/**
 * Precedence and associativity.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */
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

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program: statement                                    { $$ = StatementProgramSemanticAction(currentCompilerState(), $1); }
	| program statement                              { $$ = MultiStatementProgramSemanticAction(currentCompilerState(), $1, $2); }
	;

statement: capture_statement                         { $$ = $1; }
	| extract_statement                             { $$ = $1; }
	| filter_statement                              { $$ = $1; }
	;

capture_statement: CAPTURE FROM STRING WHERE condition SEMICOLON 
													{ $$ = CaptureStatementSemanticAction($3, $5); }
	;

extract_statement: EXTRACT OPEN_BRACE field_list CLOSE_BRACE 
													FROM STRING 
													WHERE condition SEMICOLON          { $$ = ExtractStatementSemanticAction($3, $6, $8); }
	;

filter_statement: FILTER condition SEMICOLON        { $$ = FilterStatementSemanticAction($2); }
	;

field_list: field                                  { $$ = $1; }
	| field_list COMMA field                       { $$ = FieldListSemanticAction($1, $3); }
	;

field: IDENTIFIER                                  { $$ = FieldSemanticAction($1, NULL); }
	| IDENTIFIER AS IDENTIFIER                     { $$ = FieldSemanticAction($1, $3); }
	;

condition: comparison_expression                    { $$ = $1; }
	| logical_expression                           { $$ = $1; }
	| OPEN_PARENTHESIS condition CLOSE_PARENTHESIS { $$ = $2; }
	;

logical_expression:
	condition AND condition                        { $$ = LogicalConditionSemanticAction($1, $3, AND_OP); }
	| condition OR condition                       { $$ = LogicalConditionSemanticAction($1, $3, OR_OP); }
	;

comparison_expression: 
	expression EQUALS expression                   { $$ = ComparisonSemanticAction($1, $3, EQUALS); }
	| expression NOT_EQUALS expression             { $$ = ComparisonSemanticAction($1, $3, NOT_EQUALS); }
	| expression LESS_THAN expression              { $$ = ComparisonSemanticAction($1, $3, LESS_THAN); }
	| expression GREATER_THAN expression           { $$ = ComparisonSemanticAction($1, $3, GREATER_THAN); }
	;

expression: expression[left] ADD expression[right]					{ $$ = ArithmeticExpressionSemanticAction($left, $right, ADDITION); }
	| expression[left] DIV expression[right]						{ $$ = ArithmeticExpressionSemanticAction($left, $right, DIVISION); }
	| expression[left] MUL expression[right]						{ $$ = ArithmeticExpressionSemanticAction($left, $right, MULTIPLICATION); }
	| expression[left] SUB expression[right]						{ $$ = ArithmeticExpressionSemanticAction($left, $right, SUBTRACTION); }
	| factor														{ $$ = FactorExpressionSemanticAction($1); }
	;

factor: OPEN_PARENTHESIS expression CLOSE_PARENTHESIS				{ $$ = ExpressionFactorSemanticAction($2); }
	| constant														{ $$ = ConstantFactorSemanticAction($1); }
	;

constant: INTEGER													{ $$ = IntegerConstantSemanticAction($1); }
	;

%%
