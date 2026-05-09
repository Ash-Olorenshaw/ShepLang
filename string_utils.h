#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdbool.h>

char *ltrim(char *str);
char *rtrim(char *str);
char *trim(char *str);

bool starts_with(const char *str, const char *start);
char *substr(const char *str, int start, int end);
int find_closing_bracket(const char *str, int start, bool rev);
int seek_char(const char *str, int start, char c);
enum {
	CHAR_ALPHA,
	CHAR_NUM,
	CHAR_SYMBOL,
	CHAR_SPACE,
	CHAR_UNKNOWN
} char_type(char c);
char *remove_unnecessary_whitespace(char *str);
int char_count(const char *str, int start, char c);
// returns pos of 'find' in 'str'
int str_find(const char *str, const char *find);
bool str_alnum(const char *str);
bool str_isspace(const char *str);

#endif
