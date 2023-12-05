#pragma once
#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#include <luajit/lua.hpp>
#include <string>
namespace rocksdb{
    class DB;
    class ColumnFamilyHandle;
}
namespace lrocks {
    int createmeta(lua_State *L, const char *name, const luaL_Reg *methods) ;
    void setfuncs (lua_State *L, const luaL_Reg *l) ;
    void setmeta(lua_State *L, const char *name) ;
    void lrocksdb_assert(lua_State *L, int cond, const char *msg) ;
    std::string get_str(lua_State* L, int index);
    int cf_writebatch_create(lua_State *L, rocksdb::DB*, rocksdb::ColumnFamilyHandle*);
    int writebatch_create(lua_State *L, rocksdb::DB*);
    int make_iterator(lua_State *L,rocksdb::DB* db);
    int backup_engine(lua_State *L, rocksdb::DB *db, std::string const&);
    int create_backup_engine( lua_State *L) ;
    int make_cf_iterator(
            lua_State* L, 
            rocksdb::DB* db,
            rocksdb::ColumnFamilyHandle* h);
}

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
