#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *ltrim(char *str) {
	while(isspace(*str)) str++;
	return str;
}

char *rtrim(char *str) {
	char* back = str + strlen(str);
	while(isspace(*--back));
	*(back + 1) = '\0';
	return str;
}

char *trim(char *str) {
	return rtrim(ltrim(str));
}

// String needs to be freed
char *substr(const char *str, int start, int end) {
	if (str == NULL || start < 0 || end < 0 || start >= end)
		return NULL;

	int len = strlen(str);

	if (start >= len) return strdup("");
	if (end > len) end = len;

	int sublen = end - start;
    char *result = malloc(sublen + 1);

    if (!result) return NULL;

    memcpy(result, str + start, sublen);
    result[sublen] = '\0';
	return result;
}


bool starts_with(const char *str, const char *start) {
	char *temp_str = substr(str, 0, strlen(start));
	bool matches = strcmp(temp_str, start);
	free(temp_str);
	return matches;
}

int find_closing_bracket(const char *str, int start, bool rev) {
	int nested = 0;
	if (rev) {
		for (int i = start; i > -1; i--) {
			if (str[i] == ')')
				nested++;
			else if (str[i] == '(' && !nested)
				return i;
			else if (str[i] == '(' && nested)
				nested--;
		}
	}
	else {
		for (int i = start; i < (int)strlen(str); i++) {
			if (str[i] == '(')
				nested++;
			else if (str[i] == ')' && !nested)
				return i;
			else if (str[i] == ')' && nested)
				nested--;
		}
	}

	return -1;
}

int seek_char(const char *str, int start, char c) {
	for (int i = start; i < (int) strlen(str); i++)
		if (str[i] == c)
			return i;
	return -1;
}

int char_count(const char *str, int start, char c) {
	int count = 0;
	for (int i = start; i < (int) strlen(str); i++)
		if (str[i] == c)
			count++;
	return count;
}

char *remove_unnecessary_whitespace(char *str) {
	char temp_str[strlen(str) + 1];
	int pos = 0, str_len = strlen(str);
	for (int i = 0; i < (int) str_len; i++) {
		if (isspace(str[i]) && i + 1 != str_len && i != 0 && pos != 0 && (isalnum(temp_str[pos - 1]) || isalnum(str[i + 1])))
			temp_str[pos++] = str[i];
		else if (!isspace(str[i]))
			temp_str[pos++] = str[i];

	}
	temp_str[pos] = '\0';

	strcpy(str, temp_str);
	return str;
}

// returns pos of 'find' in 'str'
int str_find(const char *str, const char *find) {
	int pos = -1;
	const int find_len = strlen(find);
	for (int i = 0; i < (int) strlen(str); i++) {
		char *search_str = substr(str, i, i + find_len);
		if (strcmp(search_str, find) == 0) {
			pos = i;
			free(search_str);
			break;
		}
		free(search_str);
	}
	return pos;
}

bool str_alnum(const char *str) {
	if (str == NULL)
		return false;
	for (int i = 0; i < (int) strlen(str); i++)
		if (!(isalnum(str[i]) || str[i] == '_'))
			return false;
	return true;
}

bool str_isspace(const char *str) {
	if (str == NULL)
		return false;
	for (int i = 0; i < (int) strlen(str); i++)
		if (isspace(str[i]))
			return false;
	return true;
}

