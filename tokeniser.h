#ifndef TOKENISER_H
#define TOKENISER_H

#include "./types.h"

typedef struct {
	enum {
		LITERAL_NUMBER,
		LITERAL_CHAR,
		LITERAL_STRING,
		IDENTIFIER,
		IDENTIFIER_IF,
		IDENTIFIER_ELSE,
		IDENTIFIER_ELSEIF,
		IDENTIFIER_WHILE,
		IDENTIFIER_FOR,
		IDENTIFIER_RETURN,
		IDENTIFIER_BREAK,
		IDENTIFIER_CONTINUE,
		IDENTIFIER_SWITCH,
		IDENTIFIER_CASE,
		IDENTIFIER_DEFAULT,
		IDENTIFIER_GOTO,
		IDENTIFIER_SIZEOF,
		OPERATOR, // + - * / % ++ -- > < <= >= & | ^ ~ << >> == != && || ! `? :` -> . * &
		OPERATOR_ASSIGN, // = += -= *= /= %= &= |= ^= <<= >>=
		OPERATOR_SLICE, // []
		CONTAINER, // ()
		BLOCK_START, // {
		BLOCK_END, // }
	} type;
	union {
		char *content;
		rarray *tkn_content;
	};
} tkn;

typedef enum {
	VAR_DEFINITION, // TODO: typedef
	FUNC_DEFINITION,
	BLOCK,
	MACRO,
	STATEMENT, // break, return, jmp, also maybe control flow? continue, goto, etc?
	PROTOTYPE,
	LABEL,
	// EXTERN_VAR
} tkn_type;

typedef struct {
	tkn_type type;
	union {
		struct {
			char *name;
			c_type *type;
			bool simple; // true = val, false = val_lines
			rarray *val; // tkn
			rarray *val_lines; // tkn_line
		} var_definition;
		struct {
			char *name;
			c_type *type;
			rarray *tkn_lines; // tkn_line
		} func_definition;
		struct {
			tkn *type;
			tkn *content;
			rarray *tkn_lines; // tkn_line
		} block;
		struct {
			char *content;
		} macro;
		struct {
			rarray *tkns; // tkn
		} statement;
		struct {
			c_type *type;
			char *name;
		} prototype;
		struct {
			char *name;
		} label;
	};
} tkn_line;

extern char *tkn_names[];
extern char *line_names[];

void print_tokenised_line(tkn_line *line, int depth);
rarray **tokenise(rarray *file_lines);

#endif
