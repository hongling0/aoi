#ifndef LIST_H
#define LIST_H

typedef struct list_node{
	struct list_node *next,*prev;
} list_node;

static inline void init_list_node(struct list_node *list){
	list->prev=list->next=list;
}

static inline void init_zero_list_node(struct list_node *list){
	list->prev=list->next=(void*)0;
}

static inline void __list_add(struct list_node *node,struct list_node *prev,struct list_node *next){
	next->prev =prev->next=node;
	node->prev =prev;
	node->next =next;
}

static inline void __list_del(struct list_node *prev,struct list_node *next){
	next->prev =prev;
	prev->next =next;
}

static inline void list_add(struct list_node* node,struct list_node* head){
	__list_add(node,head,head->next);
}

static inline void list_add_tail(struct list_node* node,struct list_node* head){
	__list_add(node,head->prev,head);
}

static inline void list_del(struct list_node* node){
	__list_del(node->prev,node->next);
}

static inline void list_replace(struct list_node* old,struct list_node* node){
	node->next       =old->next;
	node->prev       =old->prev;
	node->next->prev =node->prev->next=node;
}

static inline void list_replace_init(struct list_node* old,struct list_node* node){
	list_replace(old,node);
	init_list_node(old);
}

static inline int list_empty(const struct list_node *head){
	return head->next==head;
}

#define offsetof(TYPE,MEMBER) __builtin_offsetof(TYPE,MEMBER)


#define container_of(ptr,type,member) ({\
	const typeof(((type*)0)->member) *__mptr=(ptr);\
	(type*)((char *)__mptr-offsetof(type,member));})

#define list_entry(ptr,type,member) container_of(ptr,type,member)

#define list_first_entry(ptr,type,member) list_entry((ptr)->next,type,member)

#define list_for_each_entry(pos,head,member)\
	for(pos=list_entry((head)->next,typeof(*pos),member);\
		__builtin_prefetch(pos->member.next),&pos->member!=(head);\
		pos=list_entry(pos->member.next,typeof(*pos),member))

#define list_for_each_entry_safe(pos,n,head,member)\
		for(pos=list_entry((head)->next,typeof(*pos),member),\
			n=list_entry(pos->member.next,typeof(*pos),member);\
			 &pos->member!=(head);\
			 pos=n,n=list_entry(n->member.next,typeof(*n),member))

#define list_for_each_entry_reverse(pos,head,member)\
	for(pos=list_entry((head)->prev,typeof(*pos),member);\
		__builtin_prefetch(pos->member.prev),&pos->member!=(head);\
		pos=list_entry(pos->member.prev,typeof(*pos),member))

#define list_for_each_entry_reverse_safe(pos,n,head,member)\
		for(pos=list_entry((head)->prev,typeof(*pos),member),\
			n=list_entry(pos->member.prev,typeof(*pos),member);\
			 &pos->member!=(head);\
			 pos=n,n=list_entry(n->member.prev,typeof(*n),member))

#endif //LIST_H
