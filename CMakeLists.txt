# Minimum required version fo CMake. Can be modified at your own risk.
# @see https://cmake.org/download/
cmake_minimum_required(VERSION 3.22)

# Name of the project and language to use (C, in this case).
project(Compiler C)

# Debug: Print the actual compiler ID
message(STATUS "Detected compiler ID: ${CMAKE_C_COMPILER_ID}")

# Compiles the parser with Bison, generating counter-examples in case S/R or R/R conflicts exists.
add_custom_command(
	OUTPUT ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.c ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.h
	COMMAND bison -Wcounterexamples -d ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonGrammar.y --output=${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.c)

# Selects the best strategy according to the available compiler in the system.
# @see https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
	message(NOTICE "The C compiler is GCC.")

	# Options for GCC.
	add_compile_options(-fsanitize=address)
	add_compile_options(-O3)
	add_compile_options(-static-libgcc)
	add_compile_options(-std=gnu99)
	add_link_options(-fsanitize=address)

	# Compiles the scanner with Flex.
	add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c
		COMMAND flex --noyywrap --outfile=${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c --yylineno ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexPatterns.l
		DEPENDS ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.c ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.h)

elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
	message(NOTICE "The C compiler is Clang.")

	# Options for Clang.
	add_compile_options(-O3)
	add_compile_options(-std=c99)
	add_compile_options(-Wall)
	add_compile_options(-Wextra)

	# Compiles the scanner with Flex.
	add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c
		COMMAND flex --noyywrap --outfile=${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c --yylineno ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexPatterns.l
		DEPENDS ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.c ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.h)

elseif (CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
	message(NOTICE "The C compiler is AppleClang.")

	# Options for AppleClang.
	add_compile_options(-O3)
	add_compile_options(-std=c99)
	add_compile_options(-Wall)
	add_compile_options(-Wextra)

	# Compiles the scanner with Flex.
	add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c
		COMMAND flex --noyywrap --outfile=${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c --yylineno ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexPatterns.l
		DEPENDS ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.c ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.h)

elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	message(NOTICE "The C compiler is Microsoft Visual Studio.")

	# Options for MSVC.
	# add_compile_options(...)

	# Compiles the scanner with Flex (Microsoft Windows compatible).
	add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c
		COMMAND flex --noyywrap --outfile=${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c --wincompat --yylineno ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexPatterns.l
		DEPENDS ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.c ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.h)

else ()
	message(NOTICE "The C compiler is unknown: ${CMAKE_C_COMPILER_ID}")
	message(NOTICE "Using default compiler options.")

	# Default options for unknown compiler.
	add_compile_options(-O3)
	add_compile_options(-std=c99)

	# Compiles the scanner with Flex.
	add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c
		COMMAND flex --noyywrap --outfile=${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c --yylineno ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexPatterns.l
		DEPENDS ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.c ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.h)

endif ()

add_custom_target(generated_files DEPENDS 
    ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.c 
    ${CMAKE_SOURCE_DIR}/src/main/c/frontend/syntactic-analysis/BisonParser.h
    ${CMAKE_SOURCE_DIR}/src/main/c/frontend/lexical-analysis/FlexScanner.c
)

# Defines the entry-point of the application, and the source-codes (*.c extension).
# The header files (*.h extension), are automatically included from the source-codes.
add_executable(Compiler
	src/main/c/backend/code-generation/Generator.c
	src/main/c/backend/domain-specific/Calculator.c
	src/main/c/backend/semantic-analysis/SemanticAnalyzer.c
	src/main/c/backend/semantic-analysis/TypeSystem.c
	src/main/c/backend/semantic-analysis/SymbolTable.c
	src/main/c/backend/code-generation/CodeGenerator.c
	src/main/c/backend/runtime/PcapRuntime.c
	src/main/c/backend/runtime/HtmlOutput.c
	src/main/c/backend/runtime/ProtocolExtractors.c
	src/main/c/EntryPoint.c
	src/main/c/frontend/lexical-analysis/FlexActions.c
	src/main/c/frontend/lexical-analysis/LexicalAnalyzerContext.c
	src/main/c/frontend/syntactic-analysis/AbstractSyntaxTree.c
	src/main/c/frontend/syntactic-analysis/BisonActions.c
	src/main/c/frontend/syntactic-analysis/SyntacticAnalyzer.c
	src/main/c/shared/Environment.c
	src/main/c/shared/Logger.c
	src/main/c/shared/String.c
	src/main/c/frontend/syntactic-analysis/BisonParser.c
	src/main/c/frontend/lexical-analysis/FlexScanner.c
)

# Add dependency on generated files
add_dependencies(Compiler generated_files)

# Link final project and libraries.
target_link_libraries(Compiler
    pcap
    m
)
