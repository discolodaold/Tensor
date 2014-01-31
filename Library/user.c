#include "user.h"

#include "mem.h"
#include "str.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct input_stack {
	struct input_stack *parent;
	user_input_fun     fun;
};

struct output_stack {
	struct output_stack *parent;
	user_output_fun     fun;
};

static struct input_stack *_in_stack = NULL;
static struct output_stack *_out_stack = NULL;
static struct output_stack *_err_stack = NULL;

static int _in(unsigned int size, char *buffer) {
	char *result = fgets(buffer, size, stdin);
	return result ? strlen(buffer) : NO_INPUT;
}

static int _out(unsigned int code, const char *buffer) {
	fwrite(buffer, strlen(buffer), 1, stdout);
	return 1;
}

static int _err(unsigned int code, const char *buffer) {
	fwrite(buffer, strlen(buffer), 1, stderr);
	exit(1);
	return 1;
}

int user_init(void) {
	user_in_push(_in);
	user_out_push(_out);
	user_err_push(_err);
	return 1;
}

// ============================================================================

void user_in_push(user_input_fun fun) {
	struct input_stack *top = malloc(sizeof(*top));
	top->parent = _in_stack;
	top->fun = fun;
	_in_stack = top;
}

user_input_fun user_in_pop(void) {
	struct input_stack *top = _in_stack;
	if(_in_stack->parent) {
		user_input_fun result = top->fun;
		_in_stack = top->parent;
		free(top);
		return result;
	}
	return NULL;
}

int user_in(unsigned int size, char *buffer) {
	struct input_stack *top = _in_stack;
	int result;
	while(top && (result = top->fun(size, buffer)) != NO_INPUT) {
		top = top->parent;
	}
	return result;
}

// ============================================================================

void user_out_push(user_output_fun fun) {
	struct output_stack *top = malloc(sizeof(*top));
	top->parent = _out_stack;
	top->fun = fun;
	_out_stack = top;
}

user_output_fun user_out_pop(void) {
	struct output_stack *top = _out_stack;
	if(_in_stack->parent) {
		user_output_fun result = top->fun;
		_out_stack = top->parent;
		free(top);
		return result;
	}
	return NULL;
}

void user_outv(const char *format, va_list ap) {
	user_coutv(0, format, ap);
}

void user_out(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	user_coutv(0, format, ap);
	va_end(ap);
}

void user_coutv(unsigned int code, const char *format, va_list ap) {
	struct output_stack *top = _out_stack;
	char *buffer = NULL;
	size_t length;

	length = string_formatv(NULL, 0, format, ap);
	if(length == 0) {
		return;
	}

	buffer = alloca(length);
	string_formatv(buffer, length + 1, format, ap);

	while(top && 0 == top->fun(code, buffer)) {
		top = top->parent;
	}
}

void user_cout(unsigned int code, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	user_coutv(code, format, ap);
	va_end(ap);
}

// ============================================================================

void user_err_push(user_output_fun fun) {
	struct output_stack *top = malloc(sizeof(*top));
	top->parent = _err_stack;
	top->fun = fun;
	_err_stack = top;
}

user_output_fun user_err_pop(void) {
	struct output_stack *top = _err_stack;
	if(_in_stack->parent) {
		user_output_fun result = top->fun;
		_err_stack = top->parent;
		free(top);
		return result;
	}
	return NULL;
}

void user_errv(const char *format, va_list ap) {
	user_cerrv(0, format, ap);
}

void user_err(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	user_cerrv(0, format, ap);
	va_end(ap);
}

void user_cerrv(unsigned int code, const char *format, va_list ap) {
	struct output_stack *top = _err_stack;
	char *buffer = NULL;
	size_t length;

	length = string_formatv(NULL, 0, format, ap);
	if(length == 0) {
		return;
	}

	buffer = alloca(length);
	string_formatv(buffer, length + 1, format, ap);

	while(top && 0 == top->fun(code, buffer)) {
		top = top->parent;
	}
}

void user_cerr(unsigned int code, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	user_cerrv(code, format, ap);
	va_end(ap);
}
