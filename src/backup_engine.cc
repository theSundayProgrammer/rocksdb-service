#include "db.h"
#include "rocksdb/utilities/backup_engine.h"
#include <functional>
using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::BackupEngine;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Status;

using ROCKSDB_NAMESPACE::BackupEngine;
using ROCKSDB_NAMESPACE::BackupEngineOptions;
using ROCKSDB_NAMESPACE::BackupEngineReadOnly;
using ROCKSDB_NAMESPACE::BackupInfo;
using ROCKSDB_NAMESPACE::Env;
namespace {
    int l_purge_old_backups(lua_State *L);
    int l_restore_db_from_latest_backup(lua_State *L);
    int l_get_backup_info_count(lua_State *L);
    int l_get_backup_info(lua_State *L);
    int l_close(lua_State *L);
    const  char* table="backup_engine_table";

    static const struct luaL_Reg reg[] = {
        { "purge", l_purge_old_backups },
        { "restore_latest", l_restore_db_from_latest_backup },
        { "get_info_count", l_get_backup_info_count },
        { "get_info", l_get_backup_info },
        { "close", l_close },
        { "__gc", l_close },
        { NULL, NULL }
    };

    struct backup_engine_t {
        BackupEngine* backup_engine;
    };

    backup_engine_t *l_get_backup_engine(lua_State *L, int index) {
        backup_engine_t *o = (backup_engine_t*)
            luaL_checkudata(L, index, table);
        luaL_argcheck(L, o != NULL && o->backup_engine!= NULL, index, "backup_engine expected");
        return o;
    }


    int l_purge_old_backups(lua_State *L) {
        backup_engine_t *be = l_get_backup_engine(L, 1);
        uint32_t num_backups_to_keep = luaL_checknumber(L, 2);
        Status  status = be->backup_engine->PurgeOldBackups( num_backups_to_keep);
        if(!status.ok()) {
            luaL_error(L, status.ToString().c_str());
            return 0;
        }
        return 1;
    }

    int l_restore_db_from_latest_backup(lua_State *L) {
        backup_engine_t *be = l_get_backup_engine(L, 1);

        uint32_t backup_id = luaL_checknumber(L, 2);
        std::string  db_dir = lrocks::get_str(L, 3);
        std::string  wal_dir = lrocks::get_str(L, 4);
        Status s = be->backup_engine->RestoreDBFromBackup(backup_id,db_dir,wal_dir);
        if(!s.ok()) {
            luaL_error(L, s.ToString().c_str());
            return 0;
        }
        return 1;
    }

    int l_get_backup_info_count(lua_State *L) {
        backup_engine_t *be = l_get_backup_engine(L, 1);
        std::vector<BackupInfo> backup_info;
        be->backup_engine->GetBackupInfo(&backup_info);
        int count = backup_info.size();
        lua_pushnumber(L, count);
        return 1;
    }
    int l_get_backup_info(lua_State *L) {
        backup_engine_t *be = l_get_backup_engine(L, 1);
        std::vector<BackupInfo> backup_info;
        be->backup_engine->GetBackupInfo(&backup_info);
        lua_newtable(L);
        int k =0;
        for (auto& info: backup_info)
        {
            lua_newtable(L);
            lua_pushnumber(L,info.timestamp);
            lua_setfield(L, -2, "timestamp");
            lua_pushnumber(L, info.backup_id);
            lua_setfield(L, -2, "id");
            lua_pushnumber(L, info.size);
            lua_setfield(L, -2, "size");
            lua_pushnumber(L, info.number_files);
            lua_setfield(L, -2, "number_files");
            lua_rawseti(L,-2,++k);
        }
        return 1;
    }

    int l_close(lua_State *L) {
        backup_engine_t *be = l_get_backup_engine(L, 1);
        if(be->backup_engine != NULL) {
            delete be->backup_engine;
            be->backup_engine = NULL;
        }
        return 1;
    }
}
static bool init_backup(lua_State* L) {
    static bool init = [=]() {
        lrocks::createmeta(L, table, reg);
        return true;
    }();
    return init;
}
static int create_backup_engine_( lua_State *L, const std::string& path, std::function<Status (BackupEngine*)> func) {
    if (!init_backup(L))
        return 0;
    BackupEngine* backup_engine;
    Status s = BackupEngine::Open(Env::Default(),
            BackupEngineOptions(path),
            &backup_engine);
    assert(s.ok());
    if(!s.ok()) {
        luaL_error(L, "unable to create backup engine");
        return 0;
    }
    s = func(backup_engine);
    if(!s.ok()) {
        luaL_error(L, "unable to backup ");
        return 0;
    }
    backup_engine_t *b = new (lua_newuserdata(L, sizeof(backup_engine_t))) backup_engine_t ;
    b->backup_engine = backup_engine ;
    //push meta table onto stack
    luaL_getmetatable(L, table);
    //assign meta table to user data and return
    lua_setmetatable(L, -2);
    return 1;
}
namespace lrocks {
    int create_backup_engine( lua_State *L) {
        std::string path= lrocks::get_str(L,1);
        return create_backup_engine_(L,path, [](BackupEngine*){
                return Status();
                });
    }
    int backup_engine(
        lua_State *L, 
        ROCKSDB_NAMESPACE::DB* db,
        std::string const& path
        ) {
        return create_backup_engine_(L,path, [db](BackupEngine* backup_engine){
                //fprintf(stderr,"backed up\n");
                return backup_engine->CreateNewBackup(db);
                });
    }
}
#if 0
int main() {
  assert(s.ok());

  // create backup
  BackupEngine* backup_engine;
  s = BackupEngine::Open(Env::Default(),
                         BackupEngineOptions("/tmp/rocksdb_example_backup"),
                         &backup_engine);
  assert(s.ok());


  std::vector<BackupInfo> backup_info;
  backup_engine->GetBackupInfo(&backup_info);

  s = backup_engine->VerifyBackup(1);
  assert(s.ok());

  // Put key-value
  db->Put(WriteOptions(), "key2", "value2");
  assert(s.ok());

  db->Close();
  delete db;
  db = nullptr;

  // restore db to backup 1
  BackupEngineReadOnly* backup_engine_ro;
  s = BackupEngineReadOnly::Open(
      Env::Default(), BackupEngineOptions("/tmp/rocksdb_example_backup"),
      &backup_engine_ro);
  assert(s.ok());

  s = backup_engine_ro->RestoreDBFromBackup(1, "/tmp/rocksdb_example",
                                            "/tmp/rocksdb_example");
  assert(s.ok());

  // open db again
  s = DB::Open(options, kDBPath, &db);
  assert(s.ok());

  std::string value;
  s = db->Get(ReadOptions(), "key1", &value);
  assert(!s.IsNotFound());

  s = db->Get(ReadOptions(), "key2", &value);
  assert(s.IsNotFound());

  delete backup_engine;
  delete backup_engine_ro;
  delete db;

  return 0;
}
#endif
