#ifndef PT_H
#define PT_H

#include "scalar.h"
#include "array.h"
#include "inline_list.h"

enum {
	__pt__wait_code,
	__pt__yield_code,
	__pt__abort_code,
	__pt__exit_code
};

struct pt_message {
	struct inline_list list;
	const char *id;
	void (*free)(struct pt_message *);
};

void pt_message_free(struct pt_message *message);

struct pt {
	u2 line;
	int (*run)(struct pt *pt);
	struct inline_list messages;
};

#define PT_ENTER(pt) u1 __pt__yield=0;switch(pt->line){case 0:
#define PT_EXIT(pt) }pt->line=0;return __pt__exit_code;

#define PT_YIELD(pt) __pt__yield=1;pt->line=__LINE__;case __LINE__:if(__pt__yield){return __pt__yield_code;}
#define PT_WAIT(pt, cond) pt->line=__LINE__;case __LINE__:if(!(cond)){return __pt__wait_code;}
#define PT_ABORT(pt) pt->line=0;return __pt__abort_code;

void pt_init(struct pt *, int (*)(struct pt *));
int pt_run(struct pt *);
struct pt_message *pt_receive(struct pt *, const char *);
void pt_send(struct pt *, struct pt_message *);

struct pt_pool {
	struct pt pt;
	array(struct pt *) pts;
};

void pt_pool_init(struct pt_pool *);
void pt_pool_add(struct pt_pool *, struct pt *);

#endif