#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "aoi.h"

struct laoi{
	int radius;
	int ref;
	struct aoi_space *aoi;
};

static int laoi_insertvec(lua_State *L,struct aoi_vec *vec){
	size_t i,cnt;
	cnt=aoi_vec_count(vec);
	if(cnt>0){
		lua_createtable(L,cnt,0);
		for(i=0;i<cnt;i++){
			lua_pushinteger(L,(int)aoi_vec_index(vec,i));
			lua_rawseti(L,-2,i+1);
		}
		aoi_vec_reset(vec);
	}else{
		lua_pushnil(L);
	}
	return 1;
}

static void laoi_AOI_EVENT(lua_State *L,int id,struct laoi *i){
	lua_rawgeti(L,LUA_REGISTRYINDEX,i->ref);
	lua_pushinteger(L,id);
	laoi_insertvec(L,aoi_invec(i->aoi));
	laoi_insertvec(L,aoi_outvec(i->aoi));
	laoi_insertvec(L,aoi_movevec(i->aoi));
	lua_call(L,4,0);
}

static struct laoi* check_aoi(lua_State *L,int idx){
	struct laoi *i=lua_touserdata(L,idx);
	if(!i){
		luaL_error(L,"expected laoi but got a %s(%d)",luaL_typename(L,idx),idx);
	}
	return i;
}

static int laoi_gc(lua_State *L){
	struct laoi *i=lua_touserdata(L,1);
	aoi_destory(i->aoi);
	luaL_unref(L,LUA_REGISTRYINDEX,i->ref);
	return 0;
}

static int laoi_create(lua_State *L){
	struct laoi *i;
	int radius;
	radius=luaL_checkinteger(L,1);
	if(radius<=0){
		luaL_error(L,"radius need >0 but go %d",radius);
	}
	luaL_checktype(L,2,LUA_TFUNCTION);
	i=lua_newuserdata(L,sizeof(*i));
	i->radius=radius;
	i->aoi=aoi_create();
	lua_pushvalue(L,2);
	i->ref=luaL_ref(L,LUA_REGISTRYINDEX);

	lua_createtable(L,0,1);
	lua_pushcfunction(L,laoi_gc);
	lua_setfield(L,-2,"__gc");
	lua_setmetatable(L,-2);
	return 1;
}

static int laoi_add(lua_State *L){
	struct laoi* i=check_aoi(L,1);
	int id=luaL_checkinteger(L,2);
	int x=luaL_checkinteger(L,3);
	int y=luaL_checkinteger(L,4);
	if(aoi_add(i->aoi,id,x,y,i->radius)){
		luaL_error(L,"(%d) already in aoi",id);
	}
	laoi_AOI_EVENT(L,id,i);
	return 0;
}

static int laoi_del(lua_State *L){
	struct laoi* i=check_aoi(L,1);
	int id=luaL_checkinteger(L,2);
	if(aoi_del(i->aoi,id)){
		luaL_error(L,"(%d) not in aoi",id);
	}
	laoi_AOI_EVENT(L,id,i);
	return 0;
}

static int laoi_move(lua_State *L){
	struct laoi* i=check_aoi(L,1);
	int id=luaL_checkinteger(L,2);
	int x=luaL_checkinteger(L,3);
	int y=luaL_checkinteger(L,4);
	if(aoi_move(i->aoi,id,x,y)){
		luaL_error(L,"(%d) not in aoi",id);
	}
	laoi_AOI_EVENT(L,id,i);
	return 0;
}

static int laoi_on_aoi(lua_State *L){
	struct laoi* i=check_aoi(L,1);
	luaL_checktype(L,2,LUA_TFUNCTION);
	luaL_unref(L,LUA_REGISTRYINDEX,i->ref);
	lua_pushvalue(L,2);
	i->ref=luaL_ref(L,LUA_REGISTRYINDEX);
	return 0;
}

static int laoi_memory(lua_State *L){
	lua_pushinteger(L,aoi_statics_memory());
	return 1;
}


int luaopen_laoi_c(lua_State*L){
    static const struct luaL_Reg lib[]={
        {"new",laoi_create},
        {"add",laoi_add},
        {"del",laoi_del},
        {"move",laoi_move},
        {"reg_event",laoi_on_aoi},
        {"memory",laoi_memory},
        {NULL,NULL}
    };
    luaL_newlib(L,lib);
    return 1;
}