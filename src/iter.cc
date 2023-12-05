#include "db.h"
namespace {

    int valid(lua_State *L);
    int seek_to_first(lua_State *L);
    int seek_to_last(lua_State *L);
    int seek(lua_State *L);
    int next(lua_State *L);
    int prev(lua_State *L);
    int key(lua_State *L);
    int value(lua_State *L);
    int get_error(lua_State *L);
    int destroy(lua_State *L);
    const char* table="iterator";
    const struct luaL_Reg reg[] = {
        { "valid", valid },
        { "seek_to_first", seek_to_first },
        { "seek_to_last", seek_to_last },
        { "seek", seek },
        { "next", next },
        { "prev", prev },
        { "key", key },
        { "value", value },
//        { "get_error", get_error },
        { "destroy", destroy },
        { "__gec", destroy },
        { NULL, NULL }
    };
    struct iterator_t{
        ROCKSDB_NAMESPACE::Iterator* iter;
    } ;

    iterator_t *get_iter(lua_State *L, int index) {
        iterator_t *i = (iterator_t *) luaL_checkudata(L, index, table);
        luaL_argcheck(L, i != NULL && i->iter != NULL, index, "iterator expected");
        return i;
    }

    int valid(lua_State *L) {
        iterator_t *i = get_iter(L, 1);
        lua_pushboolean(L, i->iter->Valid());
        return 1;
    }

    int seek_to_first(lua_State *L) {
        iterator_t *i = get_iter(L, 1);
        i->iter->SeekToFirst();
        return 1;
    }

    int seek_to_last(lua_State *L) {
        iterator_t *i = get_iter(L, 1);
        i->iter->SeekToLast();
        return 1;
    }

    int seek(lua_State *L) {
        iterator_t *i = get_iter(L, 1);
        std::string str= lrocks::get_str(L,2);
        i->iter->Seek(str);
        return 1;
    }

    int next(lua_State *L) {
        iterator_t *i = get_iter(L, 1);
        i->iter->Next();
        return 1;
    }

    int prev(lua_State *L) {
        iterator_t *i = get_iter(L, 1);
        i->iter->Prev();
        return 1;
    }
    int key(lua_State *L) {
        iterator_t *i = get_iter(L, 1);
        lrocks::lrocksdb_assert(L,i->iter->Valid(), "invalid iterator");
        std::string key = i->iter->key().ToString();
        if(key.empty() ) {
            lua_pushnil(L);
        }
        else {
            lua_pushlstring(L, key.data(), key.length());
        }
        return 1;
    }

    int value(lua_State *L) {
        iterator_t *i = get_iter(L, 1);
        lrocks::lrocksdb_assert(L,i->iter->Valid(), "invalid iterator");
        std::string value = i->iter->value().ToString();
        if(value.empty() ) {
            lua_pushnil(L);
        }
        else {
            lua_pushlstring(L, value.data(), value.length());
        }
        return 1;
    }
    int get_error(lua_State *L) {
        iterator_t *i = get_iter(L, 1);
        std::string value = i->iter->status().getState();
        if(value.empty() ) {
            lua_pushnil(L);
        }
        else {
            lua_pushlstring(L, value.data(), value.length());
        }
        return 1;
    }

    int destroy(lua_State *L) {
        iterator_t *i = get_iter(L, 1);
        if(i->iter != NULL) {
            delete i->iter;
            i->iter = NULL;
        }
        return 1;
    }
    bool make_table(lua_State* L)
    {
        static bool init = [](lua_State* L) {
            lrocks::createmeta(L, table, reg);
            //fprintf(stderr,"write meta table created\n");
            return true;
        }(L);
        return init;
    }

}
namespace lrocks {

    int make_iterator(lua_State *L,ROCKSDB_NAMESPACE::DB* db){
        if(!make_table(L))
            return 0;
        auto iter = new (lua_newuserdata(L, sizeof(iterator_t))) iterator_t() ;
        iter->iter = db->NewIterator(ROCKSDB_NAMESPACE::ReadOptions());
        luaL_getmetatable(L, table);
        lua_setmetatable(L, -2);
        return 1;
    }
    int make_cf_iterator(
            lua_State* L, 
            ROCKSDB_NAMESPACE::DB* db,
            ROCKSDB_NAMESPACE::ColumnFamilyHandle* h){
        if(!make_table(L))
            return 0;
        auto iter = new (lua_newuserdata(L, sizeof(iterator_t))) iterator_t() ;
        iter->iter = db->NewIterator(ROCKSDB_NAMESPACE::ReadOptions(),h);
        luaL_getmetatable(L, table);
        lua_setmetatable(L, -2);
        return 1;
    }
}
