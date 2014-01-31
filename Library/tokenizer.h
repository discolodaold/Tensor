#ifndef CORE_TOKENIZER_H
#define CORE_TOKENIZER_H

struct token {
	unsigned int length;
	const char   *ptr;
};

struct tokenizer *new_tokenizer(const char *);
unsigned int tokenizer_next(struct tokenizer *, struct token *, unsigned int);
void tokenizer_unget(struct tokenizer *);
unsigned int tokenizer_available(struct tokenizer *);
void tokenizer_free(struct tokenizer *);

unsigned int token_is(struct token *, const char *);
const char *token_copy(struct token *);

#endif
