#ifndef FLEX_ACTIONS_HEADER
#define FLEX_ACTIONS_HEADER

#include "../../shared/Environment.h"
#include "../../shared/Logger.h"
#include "../../shared/String.h"
#include "../../shared/Type.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"
#include "LexicalAnalyzerContext.h"
#include <stdio.h>
#include <stdlib.h>

/** Initialize module's internal state. */
void initializeFlexActionsModule();

/** Shutdown module's internal state. */
void shutdownFlexActionsModule();

/**
 * Flex lexeme processing actions.
 */

/* Comment Actions */
void BeginMultilineCommentLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);
void EndMultilineCommentLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);
void IgnoredLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);

/* Keyword Actions */
Token KeywordLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);
Token ProtocolLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);
Token TypeLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);

/* Operator Actions */
Token ArithmeticOperatorLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);
Token RelationalOperatorLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);
Token AssignmentOperatorLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);

/* Delimiter Actions */
Token ParenthesisLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);
Token BraceLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);
Token SemicolonLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);
Token CommaLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);

/* Literal Actions */
Token IntegerLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);
Token IdentifierLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);
Token StringLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);

/* Error Handling */
Token UnknownLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);

#endif
