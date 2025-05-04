%{
#include "FlexActions.h"
%}

/**
 * Enable start condition manipulation functions.
 */
%option stack

/**
 * Flex contexts (a.k.a. start conditions).
 *
 * @see https://westes.github.io/flex/manual/Start-Conditions.html
 */
%x MULTILINE_COMMENT
%x STRING_LITERAL

/**
 * Reusable patterns.
 *
 * @see https://westes.github.io/flex/manual/Matching.html
 * @see https://westes.github.io/flex/manual/Patterns.html
 */
digit           [0-9]
letter          [a-zA-Z]
identifier      {letter}({letter}|{digit})*
whitespace      [ \t\n\r]
string          \"[^\"]*\"

%%

"/*"                        { BEGIN(MULTILINE_COMMENT); BeginMultilineCommentLexemeAction(createLexicalAnalyzerContext()); }
<MULTILINE_COMMENT>"*/"     { EndMultilineCommentLexemeAction(createLexicalAnalyzerContext()); BEGIN(INITIAL); }
<MULTILINE_COMMENT>[^*]+    { IgnoredLexemeAction(createLexicalAnalyzerContext()); }
<MULTILINE_COMMENT>"*"      { IgnoredLexemeAction(createLexicalAnalyzerContext()); }
"//".*                      { IgnoredLexemeAction(createLexicalAnalyzerContext()); }

"PROTOCOL"                  { return KeywordLexemeAction(createLexicalAnalyzerContext(), PROTOCOL); }
"HTTP"                      { return ProtocolLexemeAction(createLexicalAnalyzerContext(), PROTO_HTTP); }
"TCP"                       { return ProtocolLexemeAction(createLexicalAnalyzerContext(), PROTO_TCP); }
"UDP"                       { return ProtocolLexemeAction(createLexicalAnalyzerContext(), PROTO_UDP); }
"DNS"                       { return ProtocolLexemeAction(createLexicalAnalyzerContext(), PROTO_DNS); }

"extract"                   { return KeywordLexemeAction(createLexicalAnalyzerContext(), EXTRACT); }
"capture"                   { return KeywordLexemeAction(createLexicalAnalyzerContext(), CAPTURE); }
"filter"                    { return KeywordLexemeAction(createLexicalAnalyzerContext(), FILTER); }
"group"                     { return KeywordLexemeAction(createLexicalAnalyzerContext(), GROUP); }
"count"                     { return KeywordLexemeAction(createLexicalAnalyzerContext(), COUNT); }
"where"                     { return KeywordLexemeAction(createLexicalAnalyzerContext(), WHERE); }
"having"                    { return KeywordLexemeAction(createLexicalAnalyzerContext(), HAVING); }
"order"                     { return KeywordLexemeAction(createLexicalAnalyzerContext(), ORDER); }
"by"                        { return KeywordLexemeAction(createLexicalAnalyzerContext(), BY); }
"from"                      { return KeywordLexemeAction(createLexicalAnalyzerContext(), FROM); }
"as"                        { return KeywordLexemeAction(createLexicalAnalyzerContext(), AS); }

"alert"                     { return KeywordLexemeAction(createLexicalAnalyzerContext(), ALERT); }
"when"                      { return KeywordLexemeAction(createLexicalAnalyzerContext(), WHEN); }
"send"                      { return KeywordLexemeAction(createLexicalAnalyzerContext(), SEND); }
"to"                        { return KeywordLexemeAction(createLexicalAnalyzerContext(), TO); }

"same"                      { return KeywordLexemeAction(createLexicalAnalyzerContext(), SAME); }
"different"                 { return KeywordLexemeAction(createLexicalAnalyzerContext(), DIFFERENT); }

"define"                    { return KeywordLexemeAction(createLexicalAnalyzerContext(), DEFINE); }
"import"                    { return KeywordLexemeAction(createLexicalAnalyzerContext(), IMPORT); }
"export"                    { return KeywordLexemeAction(createLexicalAnalyzerContext(), EXPORT); }
"table"                     { return KeywordLexemeAction(createLexicalAnalyzerContext(), TABLE); }
"timespan"                  { return KeywordLexemeAction(createLexicalAnalyzerContext(), TIMESPAN); }

"if"                        { return KeywordLexemeAction(createLexicalAnalyzerContext(), IF); }
"else"                      { return KeywordLexemeAction(createLexicalAnalyzerContext(), ELSE); }
"while"                     { return KeywordLexemeAction(createLexicalAnalyzerContext(), WHILE); }

"packet"                    { return TypeLexemeAction(createLexicalAnalyzerContext(), PACKET_TYPE); }
"address"                   { return TypeLexemeAction(createLexicalAnalyzerContext(), ADDRESS_TYPE); }

"+"                         { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), ADD); }
"-"                         { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), SUB); }
"*"                         { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), MUL); }
"/"                         { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), DIV); }
"="                         { return AssignmentOperatorLexemeAction(createLexicalAnalyzerContext(), ASSIGN); }
"=="                        { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), EQUALS); }
"!="                        { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), NOT_EQUALS); }
"<"                         { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), LESS_THAN); }
">"                         { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), GREATER_THAN); }

"("                         { return ParenthesisLexemeAction(createLexicalAnalyzerContext(), OPEN_PARENTHESIS); }
")"                         { return ParenthesisLexemeAction(createLexicalAnalyzerContext(), CLOSE_PARENTHESIS); }
"{"                         { return BraceLexemeAction(createLexicalAnalyzerContext(), OPEN_BRACE); }
"}"                         { return BraceLexemeAction(createLexicalAnalyzerContext(), CLOSE_BRACE); }
";"                         { return SemicolonLexemeAction(createLexicalAnalyzerContext(), SEMICOLON); }
","                         { return CommaLexemeAction(createLexicalAnalyzerContext(), COMMA); }

{digit}+                    { return IntegerLexemeAction(createLexicalAnalyzerContext()); }
{identifier}                { return IdentifierLexemeAction(createLexicalAnalyzerContext()); }
{string}                    { return StringLexemeAction(createLexicalAnalyzerContext()); }

{whitespace}+               { IgnoredLexemeAction(createLexicalAnalyzerContext()); }
.                           { return UnknownLexemeAction(createLexicalAnalyzerContext()); }

"and"                       { return KeywordLexemeAction(createLexicalAnalyzerContext(), AND); }
"or"                        { return KeywordLexemeAction(createLexicalAnalyzerContext(), OR); }

%%

#include "FlexExport.h"
