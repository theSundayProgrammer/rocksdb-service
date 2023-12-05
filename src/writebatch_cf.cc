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

static const char* table="cfbatchtable";
namespace {

    struct writebatch_t{
        DB * db;
        WriteBatch batch;
        ColumnFamilyHandle* cfh;
        void write() {
            db->Write(WriteOptions(),&batch);
        }
    };
    writebatch_t *get_writebatch(lua_State *L, int index) ;
    int put(lua_State *L);
    int clear(lua_State *L);
    int count(lua_State *L);
    int merge(lua_State *L);
    int destroy(lua_State *L);
    int write(lua_State *L);
    const struct luaL_Reg reg[] = {
        { "put", put },
        { "clear", clear },
        { "count", count },
        { "merge", merge },
        { "write", write },
        { "destroy", destroy },
        //{ "__gc", destroy },
        { NULL, NULL }
    };
    int write(lua_State *L) {
        writebatch_t *rb = get_writebatch(L, 1);
        rb->write();
        return 0;
    }
    int destroy(lua_State *L) {
        writebatch_t *rb = get_writebatch(L, 1);
        rb->~writebatch_t();
        return 0;
    }

    int clear(lua_State *L) {
        writebatch_t *rb = get_writebatch(L, 1);
        rb->batch.Clear();
        return 0;
    }

    int count(lua_State *L) {
        writebatch_t *rb = get_writebatch(L, 1);
        int count = rb->batch.Count();
        lua_pushnumber(L, count);
        return 1;
    }

    int put(lua_State *L) {
        writebatch_t *rb = get_writebatch(L, 1);
        std::string key = lrocks::get_str(L, 2);
        std::string val = lrocks::get_str(L, 3);
        rb->batch.Put(rb->cfh,key,val);
        return 0;
    }

    int merge(lua_State *L) {
        writebatch_t *rb = get_writebatch(L, 1);
        std::string key = lrocks::get_str(L, 2);
        std::string val = lrocks::get_str(L, 3);
        rb->batch.Merge(rb->cfh,key,val);
        return 0;
    }
    writebatch_t *get_writebatch(lua_State *L, int index) {
        writebatch_t *o = (writebatch_t*) luaL_checkudata(L, index, table);
        luaL_argcheck(L, o != NULL , index, "writebatch expected");
        return o;
    }
}

namespace lrocks {

    int cf_writebatch_create(lua_State *L,
            ROCKSDB_NAMESPACE::DB* db,
            ROCKSDB_NAMESPACE::ColumnFamilyHandle* cfh){
        static bool init = [](lua_State* L) {
            lrocks::createmeta(L, table, reg);
            //fprintf(stderr,"write meta table created\n");
            return true;
        }(L);
        auto wb = new (lua_newuserdata(L, sizeof(writebatch_t))) writebatch_t() ;
        wb->db = db;
        wb->cfh = cfh;
        luaL_getmetatable(L, table);
        lua_setmetatable(L, -2);
        return 1;
    }
}
