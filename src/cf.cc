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
    int open_cf(lua_State *L);
    namespace {
        const char* table="column_family";
        int close(lua_State *L) ;
        int get(lua_State* L) ;
        int put(lua_State* L) ;
        int remove(lua_State* L);
        int batch_begin (lua_State* L);
        int iterator (lua_State* L);
        const struct luaL_Reg  cf_reg[] = {
            { "remove", remove },
            { "get", get },
            { "put", put },
            { "batch_begin", batch_begin },
            { "close", close },
            {"iterator", iterator},
            { NULL, NULL }
        };



        struct rocks_cf { 
            DB* db=nullptr;
            bool open=false;
            Options options;
            std::vector<std::string> cf_names;
            std::vector<ColumnFamilyHandle*> handles;
            ColumnFamilyHandle* get_handle(const std::string& str){
                ColumnFamilyHandle  *handle = nullptr;
                auto iter_handle = handles.begin();
                for( auto iter_name = cf_names.begin();
                        iter_name != cf_names.end() && *iter_name != str;
                        ++iter_name) 
                    ++iter_handle;
                if (iter_handle != handles.end())
                    handle = *iter_handle;
                return handle;

            }


            ~rocks_cf () {
                for (auto handle : handles) {
                    Status s = db->DestroyColumnFamilyHandle(handle);
                    assert(s.ok());
                }
                delete db;
            }
        };
        int batch_begin(lua_State* L) {
            int argc=0;
            rocks_cf *d = (rocks_cf*) luaL_checkudata(L, ++argc, table);
            std::string cf_name = lrocks::get_str(L, ++argc);
            lrocks::cf_writebatch_create(L, d->db, d->get_handle(cf_name));
            return 1;
        }
        int put(lua_State* L) {
            using ROCKSDB_NAMESPACE::Slice;
            using lrocks::get_str;
            int argc=0;
            rocks_cf *d = (rocks_cf*) luaL_checkudata(L, ++argc, table);
            std::string cf_name = get_str(L, ++argc);
            std::string key = get_str(L, ++argc);
            std::string value = get_str(L, ++argc);
            Status s = d->db->Put(WriteOptions(), d->get_handle(cf_name), Slice(key), Slice(value));
            if(!s.ok()) {
                fprintf(stderr,"status = %s\n", s.getState());
                luaL_error(L, "failed to put");
                return 0;
            }
            return 1;
        }
        int remove(lua_State* L){
            using ROCKSDB_NAMESPACE::Slice;
            using lrocks::get_str;
            int argc=0;
            rocks_cf *d = (rocks_cf*) luaL_checkudata(L, ++argc, table);
            std::string cf_name = get_str(L, ++argc);
            std::string key = get_str(L, ++argc);
            Status s = d->db->Delete(WriteOptions(), d->get_handle(cf_name), Slice(key));
            if(!s.ok()) {
                fprintf(stderr,"status = %s\n", s.getState());
                luaL_error(L, "failed to delete");
                return 0;
            }
            return 1;
        }
        int iterator(lua_State* L) {
            using ROCKSDB_NAMESPACE::Slice;
            using lrocks::get_str;
            int argc=0;
            rocks_cf *d = (rocks_cf*) luaL_checkudata(L, ++argc, table);
            std::string cf_name = get_str(L, ++argc);
            return lrocks::make_cf_iterator(L, d->db, d->get_handle(cf_name));
        }
        int get(lua_State* L) {
            using ROCKSDB_NAMESPACE::Slice;
            using lrocks::get_str;
            int argc=0;
            rocks_cf *d = (rocks_cf*) luaL_checkudata(L, ++argc, table);
            std::string cf_name = get_str(L, ++argc);
            std::string key = get_str(L, ++argc);
            std::string value ;
            Status s = d->db->Get(ReadOptions(), d->get_handle(cf_name), Slice(key), &value);
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
        int close(lua_State *L) {
            int index = 1;
            rocks_cf *d = (rocks_cf*) luaL_checkudata(L, index, table);
            luaL_argcheck(L, d != NULL && d->db != NULL, index, "cf expected");
            d->~rocks_cf(); ;

            return 1;
        }
    }


int open_cf(lua_State* L){
    static bool init = [=]() {
    lrocks::createmeta(L, table, cf_reg);
    return true;
    }();
    DB* db;
    int argc = 0;
    using ROCKSDB_NAMESPACE::ColumnFamilyDescriptor;
    Options options = options_from_table(L, ++argc);
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();

    const char *path = luaL_checkstring(L, ++argc);

    lua_pushvalue(L, ++argc);
    lua_pushnil(L);
    // open DB with two column families
    std::vector<ColumnFamilyDescriptor> column_families;
    // have to open default column family
    std::vector<std::string> cf_names;
    while (lua_next(L, -2))
    {
        lua_pushvalue(L, -2);
        int index = luaL_checkint(L, -1);
        const char *value = lua_tostring(L, -2);
        cf_names.push_back(value);
        column_families.push_back(
                ColumnFamilyDescriptor( value, ROCKSDB_NAMESPACE::ColumnFamilyOptions())
                );
        //fprintf(stderr,"%d=%s\n", index,value);
        lua_pop(L, 2);
    }
    lua_pop(L, 1);
    /* int options */
    std::vector<ColumnFamilyHandle*> handles;
    Status s = DB::Open(options, path, column_families, &handles, &db);
    if(!s.ok()) {
        fprintf(stderr,"status = %s\n", s.getState());
        luaL_error(L, "failed to open");
        return 0;
    }
    rocks_cf *d = new (lua_newuserdata(L, sizeof(rocks_cf))) rocks_cf();
    d->options=options;
    d->db = db;
    d->open = true;
    d->handles.swap(handles);
    d->cf_names.swap(cf_names);
    assert(cf_names.size() == handles.size());
    lrocks::setmeta(L, table);

    return 1;
}

