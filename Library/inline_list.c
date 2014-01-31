#include "inline_list.h"

void inline_list_init(struct inline_list *list) {
	list->next = list->prev = list;
}

static void _add(struct inline_list *list, struct inline_list *prev, struct inline_list *next) {
	prev->next = list;
	list->prev = prev;
	list->next = next;
	next->prev = list;
}

void inline_list_add(struct inline_list *list, struct inline_list *head) {
	_add(list, head, head->next);
}

void inline_list_del(struct inline_list *list) {
	list->prev->next = list->next;
	list->next->prev = list->prev;
	inline_list_init(list);
}

void inline_list_move(struct inline_list *list, struct inline_list *head) {
	inline_list_del(list);
	inline_list_add(list, head);
}