#include <stdio.h>
#include <math.h>
#include "aoi.h"

#define AOI_RADIUS 2

struct test_meta{
	int d[5];
};

static int m[][5]={
{1	,7	,8	,0	,0	},
{2	,1	,8	,0	,0	},
{3	,1	,10	,0	,0	},
{4	,8	,8	,0	,0	},
{5	,5	,7	,0	,0	},
{6	,3	,3	,0	,0	},
{7	,5	,8	,0	,0	},
{8	,10	,2	,0	,0	},
{9	,0	,7	,0	,0	},
{10	,8	,0	,0	,0	},
};

static void
print_vec(struct aoi_vec* vec,const char* tpnm){
	size_t i;
	size_t cnt=aoi_vec_count(vec);
	int _id;
	if(cnt>0){
		printf("%s\t:",tpnm);
		for(i=0;i<cnt;i++){
			_id=(int)aoi_vec_index(vec,i);
			printf("%02d[%02d,%02d],",_id,m[_id-1][3],m[_id-1][4]);
		}
		printf("\n");
	}
	aoi_vec_reset(vec);
}

static void
print_aoi_result(int id,struct aoi_space* aoi,const char *type){
	printf("%02d[%02d,%02d] %s--------------------------------\n",id,m[id-1][3],m[id-1][4],type);
	print_vec(aoi_invec(aoi),"in");
	print_vec(aoi_outvec(aoi),"out");
}

int main(){
	size_t i=0;
	struct aoi_space *aoi=aoi_create();
	for(i=0;i<sizeof(m)/sizeof(m[0]);i++){
		aoi_add(aoi,m[i][0],m[i][1],m[i][2],AOI_RADIUS);
		aoi_getpos(aoi,m[i][0],&m[i][3],&m[i][4]);
		print_aoi_result(m[i][0],aoi,"add");
	}
	i=1;
	aoi_printx(aoi);aoi_printy(aoi);
	aoi_move(aoi,i,5,5);
	aoi_getpos(aoi,m[i-1][0],&m[i-1][3],&m[i-1][4]);
	print_aoi_result(i,aoi,"mov");
	aoi_printx(aoi);aoi_printy(aoi);
	aoi_destory(aoi);
	return 0;
}