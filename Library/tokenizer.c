#include "tokenizer.h"

#include "user.h"

#include <stdlib.h>
#include <string.h>

struct tokenizer {
	unsigned int unget;
	unsigned int line;
	const char   *script;
};

struct tokenizer *new_tokenizer(const char *script) {
	struct tokenizer *result = malloc(sizeof(*result));
	result->unget = 0;
	result->line = 1;
	result->script = script;
	return result;
}

unsigned int tokenizer_next(struct tokenizer *tokenizer, struct token *token, unsigned int crossline) {
	if(tokenizer->unget) {
		return 1;
	}

skipspace:
	while(*tokenizer->script <= 32) {
		if(!*tokenizer->script) {
			if(!crossline) {
				user_err("Line %i is incomplete", tokenizer->line);
			}
			return 0;
		}
		if(*tokenizer->script++ == '\n') {
			if(!crossline) {
				user_err("Line %i is incomplete", tokenizer->line);
			}
			++tokenizer->line;
		}
	}

	if(tokenizer->script[0] == '/' && tokenizer->script[1] == '/') {
		if(!crossline) {
			user_err("Line %i is incomplete", tokenizer->line);
		}
		while(*tokenizer->script++ != '\n') {
			if(!*tokenizer->script) {
				if(!crossline) {
					user_err("Line %i is incomplete", tokenizer->line);
				}
				return 0;
			}
		}
		goto skipspace;
	}

	if(*tokenizer->script == '\"') {
		token->length = 0;
		token->ptr = ++tokenizer->script;
		while(*tokenizer->script != '\"') {
			if(!tokenizer->script) {
				user_err("EOF inside quoted token");
			}
			++token->length;
			++tokenizer->script;
		}
		++tokenizer->script;
	} else {
		token->length = 0;
		token->ptr = tokenizer->script++;
		while(*tokenizer->script > 32) {
			++token->length;
			++tokenizer->script;
		}
	}

	return 1;
}

void tokenizer_unget(struct tokenizer *tokenizer) {
	tokenizer->unget = 1;
}

unsigned int tokenizer_available(struct tokenizer *tokenizer) {
	const char *p = tokenizer->script;

	while(*p <= 32) {
		if(*p == '\n') {
			return 0;
		}
		if(*p == 0) {
			return 0;
		}
		++p;
	}

	return *p != ';';
}

unsigned int token_is(struct token *token, const char *content) {
	return strlen(content) == token->length && strncmp(content, token->ptr, token->length) == 0;
}

const char *token_copy(struct token *token) {
	char *result = malloc(token->length + 1);
	memcpy(result, token->ptr, token->length);
	result[token->length] = '\0';
	return result;
}
