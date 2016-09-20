#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <memory.h>
#include <math.h>

#include "aoi.h"
#include "list.h"

#define HASH_ID(set,id) ((set->cap-1)&id)
#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

#define AOI_TYPE_NONE	0
#define AOI_TYPE_MOVE	1
#define AOI_TYPE_IN		2
#define AOI_TYPE_OUT	3

struct obj_set{
	size_t cap;
	size_t cnt;
	struct list_node *slot;
};

struct obj_node{
	int val;
	uint32_t mark;
	uint32_t mver;
	struct list_node objs;
	struct list_node list;
};

struct aoi_obj{
	int id;
	int x;
	int y;
	int radius;
	struct list_node x_list;
	struct obj_node* x_father;
	struct list_node y_list;
	struct obj_node* y_father;
	struct list_node hash;
};

struct aoi_vec{
	size_t cap;
	size_t cnt;
	int slot[1024];
	int *array;
};

struct aoi_space{
	struct obj_set set;
	struct obj_node x_root;
	struct obj_node y_root;
	struct aoi_vec in;
	struct aoi_vec out;
	struct aoi_vec move;
};


#define POOL_DECLARE(name,typename,list) \
static list_node __poll_root_##name;\
static inline void pool_release_##name() {\
	typename* node,*next;\
	list_for_each_entry_safe(node,next,&__poll_root_##name,list){free(node);}\
	init_list_node(&__poll_root_##name);\
}\
static inline void pool_init_##name(){init_list_node(&__poll_root_##name);atexit(pool_release_##name);}\
static inline typename* pool_alloc_##name() {\
	typename* node;\
	if(!__poll_root_##name.next){pool_init_##name();}\
	if(list_empty(&__poll_root_##name)){node=(typename*)malloc(sizeof(typename));}\
	else{node=list_first_entry(&__poll_root_##name,typename,list);list_del(&node->list);}\
	return node;\
}\
static inline void pool_free_##name(typename* node) {assert(__poll_root_##name.next);list_add(&node->list,&__poll_root_##name);}

POOL_DECLARE(aoi_obj,struct aoi_obj,hash)
POOL_DECLARE(obj_node,struct obj_node,list)

void
aoi_vec_init(struct aoi_vec *v){
	v->cap=sizeof(v->slot)/sizeof(v->slot[0]);
	v->cnt=0;
	v->array=NULL;
}

void
aoi_vec_append(struct aoi_vec *v,int val){
	int *array;
	size_t cap;
	if(v->cnt==v->cap){
		cap=v->cap*2;
		array=malloc(cap*sizeof(*v->array)-sizeof(v->slot)/sizeof(v->slot[0]));
		memcpy(array,v->array,sizeof(*v->array)*v->cap);
		free(v->array);
		v->array=array;
		v->cap=cap;
	}
	if(v->cnt<(sizeof(v->slot)/sizeof(v->slot[0]))){
		v->slot[v->cnt++]=val;
	}else{
		v->array[(v->cnt++)-(sizeof(v->slot)/sizeof(v->slot[0]))]=val;
	}
}

int
aoi_vec_index(struct aoi_vec *v,size_t i){
	if(i<v->cap){
		if(i<sizeof(v->slot)/sizeof(v->slot[0])){
			return v->slot[i];
		}else{
			return v->array[i-sizeof(v->slot)/sizeof(v->slot[0])];
		}
	}
	assert(0);
	return -1;
}

size_t
aoi_vec_count(struct aoi_vec *v){
	return v->cnt;
}

void
aoi_vec_reset(struct aoi_vec *v){
	free(v->array);
	aoi_vec_init(v);
}

static inline struct aoi_obj *
objset_get(struct obj_set *set,int id){
	struct aoi_obj *obj;
	struct list_node *slot;
	if(set->cnt>0){
		slot=&set->slot[HASH_ID(set,id)];
		list_for_each_entry(obj,slot,hash){
			if(obj->id==id){
				return obj;
			}
		}
	}
	return NULL;	
}

static inline void objset_add(struct obj_set *set,struct aoi_obj *obj);
static inline void
objset_resize(struct obj_set *set){
	struct list_node *slot;
	struct aoi_obj *obj,*next;
	size_t cap=set->cap;
	size_t i;
	slot=set->slot;
	set->cap=cap?cap*2:8;
	set->cnt=0;
	set->slot=(struct list_node *)malloc(sizeof(*set->slot)*set->cap);
	for(i=0;i<set->cap;i++){
		init_list_node(&set->slot[i]);
	}
	for(i=0;i<cap;i++){
		list_for_each_entry_safe(obj,next,&slot[i],hash){
			objset_add(set,obj);
		}
	}
	free(slot);
}

static inline void 
objset_add(struct obj_set *set,struct aoi_obj *obj){
	if(set->cap*3/4<=set->cnt){
		objset_resize(set);
	}
	list_add_tail(&obj->hash,&set->slot[HASH_ID(set,obj->id)]);
	set->cnt++;
}

static inline struct aoi_obj *
objset_remove(struct obj_set *set,int id){
	struct list_node *slot;
	struct aoi_obj *obj;

	slot=&set->slot[HASH_ID(set,id)];
	list_for_each_entry(obj,slot,hash){
		if(obj->id==id){
			list_del(&obj->hash);
			set->cnt--;
			return obj;
		}
	}
	return NULL;
}

static inline struct aoi_obj *
objset_removeobj(struct obj_set *set,struct aoi_obj *obj){
	list_del(&obj->hash);
	set->cnt--;
	return obj;
}

static inline void
objset_foreach(struct obj_set *set,void(*callback)(struct aoi_obj *,void* ),void* ud){
	struct list_node *slot;
	struct aoi_obj *obj,*next;
	size_t i;
	for(i=0;i<set->cap;i++){
		slot=&set->slot[i];
		list_for_each_entry_safe(obj,next,slot,hash){
			callback(obj,ud);
		}
	}
}

static inline void
objset_release(struct obj_set *set){
	free(set->slot);
}

static inline void
init_obj_set(struct obj_set *set){
	set->cap=set->cnt=0;
	set->slot=NULL;
}

static inline void 
init_obj_node(struct obj_node *node,int v){
	node->val=v;
	node->mver=0;
	node->mark=0;
	init_list_node(&node->objs);
	init_list_node(&node->list);
}

struct aoi_space* 
aoi_create(){
	struct aoi_space* aoi=(struct aoi_space*)malloc(sizeof(*aoi));
	init_obj_set(&aoi->set);
	init_obj_node(&aoi->x_root,-1);
	init_obj_node(&aoi->y_root,-1);
	aoi_vec_init(&aoi->in);
	aoi_vec_init(&aoi->out);
	aoi_vec_init(&aoi->move);
	return aoi;
}

static void
destory_objset_callback(struct aoi_obj *obj,void* ud){
	struct aoi_space* aoi=(struct aoi_space*)ud;
	aoi_del(aoi,obj->id);
}

void 
aoi_destory(struct aoi_space* aoi){
	objset_foreach(&aoi->set,destory_objset_callback,aoi);
	assert(aoi->set.cnt==0);
	objset_release(&aoi->set);
	assert(list_empty(&aoi->x_root.list));
	assert(list_empty(&aoi->y_root.list));
	aoi_vec_reset(&aoi->in);
	aoi_vec_reset(&aoi->out);
	aoi_vec_reset(&aoi->move);
	free(aoi);
	pool_release_obj_node();
	pool_release_aoi_obj();
}

static struct obj_node *
aoi_find_node_right(struct obj_node *root,struct obj_node *head,int val){
	struct obj_node *node,*ret;
	list_for_each_entry(node,&head->list,list){
		if(node->val>=val){
			break;
		}
		if(node==root){
			break;
		}
	}
	if(node->val!=val||node==root){
		ret=pool_alloc_obj_node();
		init_obj_node(ret,val);
		list_add_tail(&ret->list,&node->list);
	}else if(node->val==val){
		ret=node;
	}
	return ret;
}

static inline void 
aoi_add_inner(struct aoi_space *aoi,struct obj_node *x_target,
				struct obj_node *y_target,struct aoi_obj* obj){
	struct obj_node *node;

	node=aoi_find_node_right(&aoi->x_root,x_target?x_target:&aoi->x_root,obj->x);
	list_add(&obj->x_list,&node->objs);
	obj->x_father=node;
	node=aoi_find_node_right(&aoi->y_root,y_target?y_target:&aoi->y_root,obj->y);
	list_add(&obj->y_list,&node->objs);
	obj->y_father=node;
}


static void
aoi_node_mark_around(struct aoi_space *aoi,int id,struct obj_node *root,struct obj_node *head
	,int radius,void (*call)(struct aoi_space *,int ,struct obj_node *)){
	struct obj_node *node;
	if((++root->mver)==0){
		list_for_each_entry(node,&root->list,list){node->mver=0;}
	}
	root->mver++;
	head->mver=root->mver;
	if(call){call(aoi,id,head);}
	list_for_each_entry(node,&head->list,list){
		if(node==root){	
			break;
		}
		if(node->val-head->val<=radius){
			node->mver=root->mver;
			if(call){call(aoi,id,node);}
		}
	}
	list_for_each_entry_reverse(node,&head->list,list){
		if(node==root){	
			break;
		}
		if(head->val-node->val<=radius){
			node->mver=root->mver;
			if(call){call(aoi,id,node);}
		}
	}
}

static void // just for y_root
aoi_add_callback(struct aoi_space *aoi,int id,struct obj_node *node){
	struct obj_node *x_node,*x_root;
	struct aoi_obj *obj;
	x_root=&aoi->x_root;
	list_for_each_entry(obj,&node->objs,y_list){
		if(obj->id!=id){
			x_node=obj->x_father;
			if(x_node->mver==x_root->mver){
				aoi_vec_append(&aoi->in,obj->id);
			}
		}
	}
}


int
aoi_add(struct aoi_space *aoi,int id,int x,int y,int radius){
	struct aoi_obj* obj;
	if(objset_get(&aoi->set,id)){
		return -1;
	}
	obj=pool_alloc_aoi_obj();
	obj->id=id;
	obj->x=x;
	obj->y=y;
	obj->radius=radius;
	init_list_node(&obj->x_list);
	init_list_node(&obj->y_list);
	init_list_node(&obj->hash);
	obj->x_father=obj->y_father=NULL;

	objset_add(&aoi->set,obj);
	aoi_add_inner(aoi,NULL,NULL,obj);
	aoi_node_mark_around(aoi,id,&aoi->x_root,obj->x_father,radius,NULL);
	aoi_node_mark_around(aoi,id,&aoi->y_root,obj->y_father,radius,aoi_add_callback);
	return 0;
}

static void // just for y_root
aoi_del_callback(struct aoi_space *aoi,int id,struct obj_node *node){
	struct obj_node *x_node,*x_root;
	struct aoi_obj *obj;
	x_root=&aoi->x_root;
	list_for_each_entry(obj,&node->objs,y_list){
		if(obj->id!=id){
			x_node=obj->x_father;
			if(x_node->mver==x_root->mver){
				aoi_vec_append(&aoi->out,obj->id);
			}
		}
	}
}

static inline void 
aoi_del_inner(struct aoi_space *aoi,struct aoi_obj* obj){
	struct obj_node *node;
	int radius=obj->radius;

	aoi_node_mark_around(aoi,obj->id,&aoi->x_root,obj->x_father,radius,NULL);
	aoi_node_mark_around(aoi,obj->id,&aoi->y_root,obj->y_father,radius,aoi_del_callback);

	list_del(&obj->x_list);
	list_del(&obj->y_list);
	node=obj->y_father;
	if(list_empty(&node->objs)){
		list_del(&node->list);
		pool_free_obj_node(node);
	}
	node=obj->x_father;
	if(list_empty(&node->objs)){
		list_del(&node->list);
		pool_free_obj_node(node);
	}
	obj->x_father=obj->y_father=NULL;
}

int
aoi_del(struct aoi_space *aoi,int id){
	struct aoi_obj* obj=objset_get(&aoi->set,id);
	if(!obj){
		return -1;
	}
	objset_removeobj(&aoi->set,obj);
	aoi_del_inner(aoi,obj);
	pool_free_aoi_obj(obj);
	return 0;
}
static inline void build_calc_struct(int p[6],int from,int to,int radius){
	if(from<=to){
		p[0]=from-radius;				p[1]=min(to-radius,from+radius);
		p[2]=to-radius;					p[3]=from+radius;
		p[4]=max(to-radius,from+radius);p[5]=to+radius;
		p[6]=p[0];						p[7]=p[5];
	}else{
		p[0]=max(to+radius,from-radius);p[1]=from+radius;
		p[2]=from-radius;				p[3]=to+radius;
		p[4]=to-radius;					p[5]=min(to+radius,from-radius);
		p[6]=p[4];						p[7]=p[1];
	}
}

static inline int calc_aoi_type(int p[6],int val){
	if(val>=p[2] && val<=p[3]){
		return AOI_TYPE_MOVE;
	}else if(val>=p[4] && val<=p[5]){
		return AOI_TYPE_IN;
	}else if(val>=p[0] && val<=p[1]){
		return AOI_TYPE_OUT;
	}else {
		return AOI_TYPE_NONE;
	}
}

static struct obj_node *
aoi_node_mark(struct aoi_space *aoi,int id,struct obj_node *root,struct obj_node *head
				,int p[5],int val,void (*call)(struct aoi_space *,int,struct obj_node *)){
	struct obj_node *node,*suit;
	if((++root->mver)==0){
		list_for_each_entry(node,&root->list,list){node->mver=0;}
	}
	root->mver++;

	node=head;
	node->mark=calc_aoi_type(p,node->val);
	node->mver=root->mver;
	if(call){call(aoi,id,node);}

	suit=root;
	list_for_each_entry(node,&head->list,list){
		if(node==root){	
			break;
		}
		if(node->val==val||(node->val>=val&&suit->val>=val&&node->val<suit->val)){
			suit=node;
		}
		if(node->val>p[7]){
			break;
		}
		node->mark=calc_aoi_type(p,node->val);
		if(node->mark==AOI_TYPE_NONE){
			continue;
		}
		node->mver=root->mver;
		if(call){call(aoi,id,node);}
	}
	list_for_each_entry_reverse(node,&head->list,list){
		if(node==root){	
			break;
		}
		if(node->val<p[6]){
			break;
		}
		if(node->val==val||(node->val>=val&&suit->val>=val&&node->val<suit->val)){
			suit=node;
		}
		node->mark=calc_aoi_type(p,node->val);
		if(node->mark==AOI_TYPE_NONE){
			continue;
		}
		node->mver=root->mver;
		if(call){call(aoi,id,node);}
	}
	
	return suit;
}

static void // just for y_root
aoi_move_callback(struct aoi_space *aoi,int id,struct obj_node *node){
	struct obj_node *x_node,*x_root;
	struct aoi_obj *obj;

	x_root=&aoi->x_root;
	if(node->mark==AOI_TYPE_OUT){
		list_for_each_entry(obj,&node->objs,y_list){
			if(obj->id!=id){
				x_node=obj->x_father;
				if(x_node->mver==x_root->mver){
					aoi_vec_append(&aoi->out,obj->id);
				}
			}
		}
	}else if(node->mark==AOI_TYPE_IN){
		list_for_each_entry(obj,&node->objs,y_list){
			if(obj->id!=id){
				x_node=obj->x_father;
				if(x_node->mver==x_root->mver){
					if(x_node->mark==AOI_TYPE_OUT){
						aoi_vec_append(&aoi->out,obj->id);
					}else{
						aoi_vec_append(&aoi->in,obj->id);
					}
				}
			}
		}
	}else{
		list_for_each_entry(obj,&node->objs,y_list){
			if(obj->id!=id){
				x_node=obj->x_father;
				if(x_node->mver==x_root->mver){
					if(x_node->mark==AOI_TYPE_IN){
						aoi_vec_append(&aoi->in,obj->id);
					}else if(x_node->mark==AOI_TYPE_OUT){
						aoi_vec_append(&aoi->out,obj->id);
					}else{
						aoi_vec_append(&aoi->move,obj->id);
					}
				}
			}
		}
	}
}

int 
aoi_move(struct aoi_space *aoi,int id,int _x,int _y){
	struct obj_node *x_node,*y_node,*node;
	int x,y,r;
	int v[8];

	struct aoi_obj* obj=objset_get(&aoi->set,id);
	if(!obj){
		return -1;
	}
	
	x=obj->x;
	y=obj->y;
	r=obj->radius;

	build_calc_struct(v,x,_x,r);
	x_node=aoi_node_mark(aoi,id,&aoi->x_root,obj->x_father,v,x,NULL);

	build_calc_struct(v,y,_y,r);
	y_node=aoi_node_mark(aoi,id,&aoi->y_root,obj->y_father,v,y,aoi_move_callback);

	node=obj->x_father;
	list_del(&obj->x_list);
	if(list_empty(&node->objs)){
		list_del(&node->list);
		pool_free_obj_node(node);
	}
	node=obj->y_father;
	list_del(&obj->y_list);
	if(list_empty(&node->objs)){
		list_del(&node->list);
		pool_free_obj_node(node);
	}
	obj->x=_x;
	obj->y=_y;
	aoi_add_inner(aoi,x_node,y_node,obj);
	return 0;
}

struct aoi_vec* aoi_outvec(struct aoi_space* aoi){
	return &aoi->out;
}
struct aoi_vec* aoi_invec(struct aoi_space* aoi){
	return &aoi->in;
}
struct aoi_vec* aoi_movevec(struct aoi_space* aoi){
	return &aoi->move;
}

int aoi_getpos(struct aoi_space* aoi,int id,int *x,int *y){
	struct aoi_obj* obj=objset_get(&aoi->set,id);
	if(!obj){
		return -1;
	}
	*x=obj->x;
	*y=obj->y;
	return 0;
}

void aoi_printx(struct aoi_space* aoi){
	struct obj_node *node;
	printf("%s:","x");
	list_for_each_entry(node,&aoi->x_root.list,list){
		printf("%02d,",node->val);
	}
	printf("\n");
}
void aoi_printy(struct aoi_space* aoi){
	struct obj_node *node;
	printf("%s:","y");
	list_for_each_entry(node,&aoi->y_root.list,list){
		printf("%02d,",node->val);
	}
	printf("\n");
}