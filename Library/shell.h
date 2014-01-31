#ifndef CORE_SHELL_H
#define CORE_SHELL_H

enum {
	SHELL_NOT_FOUND = -1000
};

typedef int (*command_fun)(int argc, char *argv[]);

int shell_init(int argc, char *argv[]);
command_fun shell_register_command(const char *, command_fun);
void shell_register_alias(const char *, const char *);
void shell_setenv_root(const char *name, char *value);
void shell_setenv(const char *name, const char *value);
const char *shell_getenv(const char *name);

int shell_do(int, char **);
void shell_parse(const char *, command_fun);
void shell_eval(const char *);

#endif
