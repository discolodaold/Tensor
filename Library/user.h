#ifndef CORE_USER_H
#define CORE_USER_H

#include <stdarg.h>

#define NO_INPUT -1

typedef int (*user_input_fun)(unsigned int, char *);
typedef int (*user_output_fun)(unsigned int, const char *);

int user_init(void);

void user_in_push(user_input_fun);
user_input_fun user_in_pop(void);
int user_in(unsigned int, char *);

void user_out_push(user_output_fun);
user_output_fun user_out_pop(void);
void user_outv(const char *, va_list);
void user_out(const char *, ...);
void user_coutv(unsigned int, const char *, va_list);
void user_cout(unsigned int, const char *, ...);

void user_err_push(user_output_fun);
user_output_fun user_err_pop(void);
void user_errv(const char *, va_list);
void user_err(const char *, ...);
void user_cerrv(unsigned int, const char *, va_list);
void user_cerr(unsigned int, const char *, ...);

#endif
