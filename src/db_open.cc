#define LROCKSDB_VERSION "nwc-rocksdb 0.0.1"
#define LROCKSDB_COPYRIGHT "Copyright (C) 2022 Joe Mariadassou"
#define LROCKSDB_DESCRIPTION "RocksDB C++ Lua binding"
#include "db.h"
#include <cstdio>
#include <string>


using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::PinnableSlice;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteBatch;
using ROCKSDB_NAMESPACE::WriteOptions;
using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
 ROCKSDB_NAMESPACE::Options options_from_table(lua_State *L, int index) ;
    int open_db(lua_State *L);
    int open_cf(lua_State *L);
    
    int backup_engine( lua_State *L, DB* db, std::string const& path) ;
    namespace {
        const char* table="nwc_db";
        int get(lua_State* L) ;
        int put(lua_State* L) ;
        int close(lua_State *L);
        int batch_begin(lua_State *L);
        int remove(lua_State *L);
        int backup(lua_State* L) ;
        int iterator(lua_State* L) ;
        const struct luaL_Reg  db_reg[] = {
            { "get", get },
            { "put", put },
            { "backup", backup },
            { "remove", remove },
            { "iterator", iterator },
            { "batch_begin", batch_begin },
            { "close", close },
            { NULL, NULL }
        };

        const struct luaL_Reg  funcs[] = {
            { "open", open_db },
            { "open_cf", open_cf },
            { "create_backup_engine", lrocks::create_backup_engine},
            { NULL, NULL }
        };

        struct rocks_db {
            DB* db=nullptr;
            Options options;
            std::string path;
            ~rocks_db () {
                delete db;
                db=nullptr;
            }
        };

        int close(lua_State *L) {
            int index = 1;
            //fprintf(stderr,"%s=%d\n", __FILE__,__LINE__);
            rocks_db *d = (rocks_db*) luaL_checkudata(L, index, table);
            luaL_argcheck(L, d != NULL , index, "db expected");
            if(d->db)
                d->~rocks_db(); ;

            return 1;
        }
        int remove(lua_State* L) {
            using ROCKSDB_NAMESPACE::Slice;
            using lrocks::get_str;
            int argc=0;
            rocks_db *d = (rocks_db*) luaL_checkudata(L, ++argc, table);
            std::string key = get_str(L, ++argc);
            Status s = d->db->Delete(WriteOptions(), Slice(key));
            if(!s.ok()) {
                fprintf(stderr,"status = %s\n", s.getState());
                luaL_error(L, "failed to delete");
                return 0;
            }
            return 1;
        }
        int iterator(lua_State* L) {
            int argc=0;
            rocks_db *d = (rocks_db*) luaL_checkudata(L, ++argc, table);
            lrocks::make_iterator(L, d->db);
            return 1;
        }
        int backup(lua_State* L) {
            using lrocks::get_str;
            int argc=0;
            rocks_db *d = (rocks_db*) luaL_checkudata(L, ++argc, table);
            std::string path = get_str(L, ++argc);

            return lrocks::backup_engine(L,d->db, path);
        }
        int batch_begin(lua_State* L) {
            int argc=0;
            rocks_db *d = (rocks_db*) luaL_checkudata(L, ++argc, table);
            lrocks::writebatch_create(L, d->db);
            return 1;
        }
        int put(lua_State* L) {
            using ROCKSDB_NAMESPACE::Slice;
            using lrocks::get_str;
            int argc=0;
            rocks_db *d = (rocks_db*) luaL_checkudata(L, ++argc, table);
            //fprintf(stderr,"path=%s\n", d->path.c_str());
            std::string key = get_str(L, ++argc);
            std::string value = get_str(L, ++argc);
            Status s = d->db->Put(WriteOptions(), Slice(key), Slice(value));
            if(!s.ok()) {
                fprintf(stderr,"status = %s\n", s.getState());
                luaL_error(L, "failed to put");
                return 0;
            }
            return 1;
        }
        int get(lua_State* L) {
            using ROCKSDB_NAMESPACE::Slice;
            using lrocks::get_str;
            int argc=0;
            rocks_db *d = (rocks_db*) luaL_checkudata(L, ++argc, table);
            std::string key = get_str(L, ++argc);
            std::string value ;
            Status s = d->db->Get(ReadOptions(), Slice(key), &value);
            if(!s.ok()) {
                fprintf(stderr,"status = %s\n", s.getState());
                lua_pushnil(L);
            }
            else if(!value.empty() ) {
                lua_pushlstring(L, value.data(), value.size());
            }
            else {
                lua_pushnil(L);
            }
            return 1;
        }
    }
int open_db(lua_State *L) {
    DB* db;
    static bool init = [=]() {
        lrocks::createmeta(L, table, db_reg);
        return true;
    }();
    int argc = 0;
    Options options = options_from_table(L, ++argc);
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();

    //fprintf(stderr," create = %s\n", options.create_if_missing ? "true": "false");
    //fprintf(stderr," create = %s\n", options.create_missing_column_families? "true": "false");
    // open DB

    std::string path = lrocks::get_str(L, ++argc);

    Status s = DB::Open(options, path, &db);
    //assert(s.ok());
    if(!s.ok()) {
        fprintf(stderr,"status = %s\n", s.getState());
        luaL_error(L, "failed to open");
        return 0;
    }
    rocks_db *d = new (lua_newuserdata(L, sizeof(rocks_db))) rocks_db();
    d->options=options;
    d->db = db;
    path.swap(d->path);
    //lrocks::setmeta(L, table);
    luaL_getmetatable(L, table);
    lua_setmetatable(L, -2);

    return 1;
}




#ifdef __cplusplus 
extern "C"
#endif
DLL_PUBLIC int luaopen_nwcrocks(lua_State *L) {

    /* register classes */

    luaL_newlib(L,funcs);
    lua_pushliteral(L, LROCKSDB_VERSION);
    lua_setfield(L, -2, "_VERSION");
    lua_pushliteral(L, LROCKSDB_COPYRIGHT);
    lua_setfield(L, -2, "_COPYRIGHT");
    lua_pushliteral(L, LROCKSDB_VERSION);
    lua_setfield(L, -2, "_DESCRIPTION");

    return 1;
}

