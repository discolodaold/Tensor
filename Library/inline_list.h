#ifndef INLINE_LIST_H
#define INLINE_LIST_H

#include "stddef.h"

struct inline_list {
	struct inline_list *prev;
	struct inline_list *next;
};

void inline_list_init(struct inline_list *list);
void inline_list_add(struct inline_list *list, struct inline_list *head);
void inline_list_del(struct inline_list *list);
void inline_list_move(struct inline_list *list, struct inline_list *head);

#define inline_list_entry(list, T, member) ((T *)(((unsigned char *)list) - offsetof(T, member)))

#define inline_list_each_entry(NAME, head, T, member) for(T *NAME = inline_list_entry((head)->next, T, member), *__ilee__next = NAME; &NAME->member != (head); NAME = __ilee__next, __ilee__next = inline_list_entry(NAME->member.next, T, member))

#endif