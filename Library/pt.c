#include "pt.h"

void pt_message_free(struct pt_message *message) {
	if(message->free) {
		message->free(message);
	}
}

void pt_init(struct pt *pt, int (*run)(struct pt *)) {
	pt->line = 0;
	pt->run = run;
	inline_list_init(&pt->messages);
}

int pt_run(struct pt *pt) {
	return pt->run(pt) < __pt__abort_code;
}

struct pt_message *pt_receive(struct pt *pt, const char *message_id) {
	inline_list_each_entry(message, &pt->messages, struct pt_message, list) {
		if(message_id == message->id || strcmp(message_id, message->id) == 0) {
			inline_list_del(&message->list);
			return message;
		}
	}
}

void pt_send(struct pt *pt, struct pt_message *message) {
	inline_list_move(&message->list, &pt->messages);
}

static int _pt_pool_run(struct pt *pt) {
	struct pt_pool *pt_pool = (struct pt_pool *)pt;
	struct pt **ptp;
	
	array_for(pt_pool->pts, ptp) {
		if(!pt_run(*ptp)) {
			// todo, remove
		}
	}
}

void pt_pool_init(struct pt_pool *pt_pool) {
	pt_init(&pt_pool->pt, _pt_pool_run);
	array_init(pt_pool->pts);
}

void pt_pool_add(struct pt_pool *pt_pool, struct pt *pt) {
	array_append(pt_pool->pts, pt);
}