#include "shell.h"

#include "str2ptr.h"
#include "user.h"
#include "str.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct environment {
	struct environment *previous;
	struct str2ptr     *variables;
	int                argc;
	char               **argv;
};

static struct str2ptr *_commands;
static struct str2ptr *_aliases;
static struct environment *_environment_root;
static struct environment *_environment = NULL;

static void _push_environment(void) {
	struct environment *previous = _environment;
	_environment = malloc(sizeof(*_environment));
	_environment->previous = previous;
	_environment->variables = new_str2ptr();
}

static void _pop_environment(void) {
	struct environment *previous = _environment->previous;
	const char *key;
	char *value;

	while((value = str2ptr_pop(_environment->variables, &key))) {
		free(value);
	}
	str2ptr_free(_environment->variables);

	free(_environment);
	_environment = previous;
}

static int alias_command(int argc, char *argv[]) {
	if(argc >= 3) {
		str2ptr_set(_aliases, argv[1], (void *)string_dup(argv[2]));
		return 0;
	}
	return -1;
}

int shell_init(int argc, char *argv[]) {
	_commands = new_str2ptr();
	_aliases = new_str2ptr();

	_environment = NULL;
	_push_environment();
	_environment->argc = argc;
	_environment->argv = argv;

	_environment_root = _environment;

	shell_register_command("alias", alias_command);
	
	return 1;
}

command_fun shell_register_command(const char *name, command_fun fun) {
	if(fun == NULL) {
		return (command_fun)str2ptr_del(_commands, name);
	} else {
		return (command_fun)str2ptr_set(_commands, name, fun);
	}
}

void shell_register_alias(const char *name, const char *replacement) {
	char *old = (char *)str2ptr_set(_aliases, name, (void *)string_dup(replacement));
	if(old) {
		free(old);
	}
}

void shell_setenv_root(const char *name, char *value) {
	struct environment *environment = _environment;
	char *old;
	while(environment->previous) {
		environment = environment->previous;
	}

	old = (char *)str2ptr_set(environment->variables, name, (void *)string_dup(value));
	if(old) {
		free(old);
	}
}

void shell_setenv(const char *name, const char *value) {
	char *old = (char *)str2ptr_set(_environment->variables, name, (void *)string_dup(value));
	if(old) {
		free(old);
	}
}

const char *shell_getenv(const char *name) {
	struct environment *environment = _environment;
	const char *value;
	while(environment) {
		value = str2ptr_get(_environment->variables, name);
		if(value) {
			return value;
		}
		environment = environment->previous;
	}
	return NULL;
}

static int _isvar(int c) {
	return isalnum(c) || c == '_';
}

struct read_input {
	struct read_input *parent;
	const char *string;
	int offset;
};

struct read {
	struct read_input *input;
	char c;
};

void _read_next(struct read *read) {
	struct read_input *input;
	while((input = read->input)) {
		read->c = input->string[input->offset++];
		if(read->c) {
			return;
		}
		read->input = input->parent;
		free(input);
	}
	return;
}

void _read_push(struct read *read, const char *string) {
	struct read_input *input = malloc(sizeof(*input));
	input->parent = read->input;
	input->string = string;
	input->offset = 0;
	if(read->input && read->input->offset) {
		--read->input->offset;
	}
	read->input = input;
	_read_next(read);
}

void _read_free(struct read *read) {
	struct read_input *input;
	while((input = read->input)) {
		read->input = input->parent;
		free(input);
	}
}

// skips space and tabs
static void _read_ws(struct read *read) {
	while(read->c && (read->c == ' ' || read->c == '\t')) _read_next(read);
}

// reads an argument
// performs variable substitution
static char *_read_arg(struct read *read) {
	char *result = NULL, *var;
	const char *var_start = NULL, *var_end = NULL, *replace;
	unsigned int len = 0;
	int q = 0;
	struct read_input *input;

	_read_ws(read);

	if(read->c == ';') {
		return NULL;
	}

#define EMIT(C) do {                   \
	result = realloc(result, len + 1); \
	result[len++] = C;                 \
} while(0)

	while(read->c && (q ? 1 : !isspace(read->c) && read->c != ';')) {
		// string
		if(read->c == '\"') {
			if(q == '\'') {
				EMIT('\"');
			} else {
				q ^= '\"';
			}
			_read_next(read);
			continue;
		}

		// literal string
		if(read->c == '\'') {
			if(q == '\"') {
				EMIT('\'');
			} else {
				q ^= '\'';
			}
			_read_next(read);
			continue;
		}

		// variable expansion
		if(read->c == '$') {
			_read_next(read);
			if(q == '\'') {
				EMIT('$');
				continue;
			}

			input = read->input;
			var_start = input->string;
			while(input == read->input && _isvar(read->c)) {
				var_end = input->string;
				_read_next(read);
			}

			var = malloc(sizeof(char) * ((var_end - var_start) + 1));
			memcpy(var, var_start, (var_end - var_start));
			var[var_end - var_start] = '\0';

			replace = shell_getenv(var);

			free(var);

			if(replace) {
				while(*replace) {
					EMIT(*replace);
					++replace;
				}
			}

			continue;
		}

		// escaping
		if(read->c == '\\') {
			_read_next(read);
			if(q == '\'') {
				EMIT('\\');
			} else if(q == '\"') {
				switch(read->c) {
				case 'n': EMIT('\n'); break;
				case 'r': EMIT('\r'); break;
				default:
					EMIT(read->c);
				}
				_read_next(read);
			} else {
				EMIT(read->c);
				_read_next(read);
			}
			continue;
		}

		EMIT(read->c);
		_read_next(read);
	}

	if(len) {
		EMIT('\0');
		return result;
	}

	return NULL;
}

static int _eval_set_environment_variable(int *environment_set, char *argument) {
	char *variable = argument, *c = variable;
	while(*c && _isvar(*c)) {
		++c;
	}
	if(*c++ == '=') {
		if(*environment_set == 0) {
			_push_environment();
			*environment_set = 1;
		}
		shell_setenv(variable, c);
		return 1;
	}
	return 0;
}

static int _eval_alias(struct read *read, const char *alias_name) {
	const char *replacement = str2ptr_get(_aliases, alias_name);
	if(replacement) {
		_read_push(read, replacement);
		return 1;
	}
	return 0;
}

int shell_do(int argc, char *argv[]) {
	command_fun fun = str2ptr_get(_commands, argv[0]);
	if(fun) {
		return fun(argc, argv);
	} else {
		return SHELL_NOT_FOUND;
	}
}

static int _do_or_die(int argc, char *argv[]) {
	int result = shell_do(argc, argv);
	if(result == SHELL_NOT_FOUND) {
		user_err("shell: Could not find function `%s`", argv[0]);
	}
	return result;
}

void shell_parse(const char *command, command_fun fun) {
	int argc = 0;
	char **argv = NULL;
	char *arg;
	int environment_set = 0;
	struct read read;

	memset(&read, 0, sizeof(read));

	_read_push(&read, command);

	while(read.input) {
		argc = 0;
		argv = NULL;

		while(read.c && (arg = _read_arg(&read))) {
			if(strlen(arg)) {
				argv = (char **)realloc(argv, sizeof(char *) * (argc + 1));
				argv[argc++] = arg;
			}

			if(argc == 1) {
				if(_eval_alias(&read, argv[0]) || _eval_set_environment_variable(&environment_set, argv[0])) {
					--argc;
					free(argv[0]);
				}
			}

			_read_ws(&read);
		}

		if(argc) {
			fun(argc, argv);

			while(argc--) {
				free(argv[argc]);
			}
			free(argv);
		}

		if(environment_set) {
			_pop_environment();
			environment_set = 0;
		}

		_read_ws(&read);
		if(read.c && (read.c == ';' || isspace(read.c))) {
			_read_next(&read);
			continue;
		}
	}

	_read_free(&read);
}

void shell_eval(const char *command) {
	shell_parse(command, _do_or_die);
}
