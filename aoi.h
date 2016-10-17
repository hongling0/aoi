#ifndef AOI_H_
#define AOI_H_


#include <stdint.h>

struct aoi_space* aoi_create();
void aoi_destory(struct aoi_space* aoi);


int aoi_add(struct aoi_space *aoi,int id,int x,int y,int radius);
int aoi_del(struct aoi_space *aoi,int id);
int aoi_move(struct aoi_space *aoi,int id,int x,int y);

struct aoi_vec;
struct aoi_vec* aoi_outvec(struct aoi_space* aoi);
struct aoi_vec* aoi_invec(struct aoi_space* aoi);

int aoi_vec_index(struct aoi_vec *v,size_t i);
void aoi_vec_init(struct aoi_vec *v);
void aoi_vec_append(struct aoi_vec *v,int val);
void aoi_vec_reset(struct aoi_vec *v);
size_t aoi_vec_count(struct aoi_vec *v);
int aoi_getpos(struct aoi_space* aoi,int id,int *x,int *y);

void aoi_printx(struct aoi_space* aoi);
void aoi_printy(struct aoi_space* aoi);

size_t aoi_statics_memory();

#endif
