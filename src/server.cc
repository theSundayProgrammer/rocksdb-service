#define LROCKSDB_VERSION "nwc-rocksdb 0.0.1"
#define LROCKSDB_COPYRIGHT "Copyright (C) 2023 Joe Mariadassou"
#define LROCKSDB_DESCRIPTION "RocksDB C++ Lua binding"
//#include <cstdio>
#include <string>
#include <json/json.h>
#include <fstream>
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <memory>
#include <utility>
#include <asio/signal_set.hpp>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#include <sstream>
#include "networkinterface.hpp"
std::string exec_json(std::string const&);
//ROCKSDB_NAMESPACE::Options options ;
using ROCKSDB_NAMESPACE::PinnableSlice;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteOptions;
using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
using asio::ip::tcp;
using std::string;

namespace RDB=ROCKSDB_NAMESPACE;

 RDB::Options options_from_json(const Json::Value& json) ;

 struct config{
   typedef std::vector<std::string> strings;
   string name;
   string path;
   RDB::Options options;
   bool is_open=false;
   strings   cf_names;
   bool is_cf;
   std::vector<ColumnFamilyHandle*> handles;
   RDB::DB* db;
   config(string const& name_, string const& path_, RDB::Options& options_, strings const& cf_names_ ):
     name(name_),
     path(path_), 
     options(options_),
     cf_names(cf_names_),
     is_cf(!cf_names_.empty())
   { }

   Json::Value open(){
     Json::Value err;
     err["error"] = 0;
     if (is_open)
       return err;
     ROCKSDB_NAMESPACE::Status status;
     using ROCKSDB_NAMESPACE::DBOptions;
     if (!is_cf)
       status= ROCKSDB_NAMESPACE::DB::Open(options, path, &db);
     else{

       namespace fs = std::filesystem;
       fs::path f{ path };
       if(! fs::exists(f)){

         RDB::Options options;
         options.create_if_missing = true;
         status= RDB::DB::Open(options, path, &db);
         std::cout << path << std::endl;
         std::cout << status.ToString() << std::endl;
         assert(status.ok());
         for(auto& cf_name : cf_names){

           ColumnFamilyHandle* cf;
           if(cf_name != RDB::kDefaultColumnFamilyName)
             status = db->CreateColumnFamily(RDB::ColumnFamilyOptions(), cf_name, &cf);
           assert(status.ok());
           std::cout << status.ToString() << std::endl;

         }
         delete db;
       }
       std::vector<ROCKSDB_NAMESPACE::ColumnFamilyDescriptor> column_families;
       // have to open default column family
       column_families.push_back(
           RDB::ColumnFamilyDescriptor( 
             RDB::kDefaultColumnFamilyName, 
             RDB::ColumnFamilyOptions()));
       for(auto& cf_name : cf_names)
       {
         std::cerr << cf_name << std::endl;
         if(cf_name != RDB::kDefaultColumnFamilyName)
           column_families.push_back(
               RDB::ColumnFamilyDescriptor( cf_name, RDB::ColumnFamilyOptions()));
       }
       status  = RDB::DB::Open(DBOptions(), path, column_families, &handles, &db);
     }
     if(!status.ok()){
       std::cout << "unable to open db: "<< status.ToString() << std::endl;
       err["error"] = 4;
     } else {
       is_open = true;
     }
     return err;
   }
   Json::Value get(const std::string& key) {
     Json::Value err;
     if (db==nullptr){
       err["error"] = -1;
       return err;
     }
     std::string value;
     using ROCKSDB_NAMESPACE::Slice;
     ROCKSDB_NAMESPACE::Status status  = db->Get(ReadOptions(), Slice(key), &value);
     if(!status.ok()) {
       err["error"] = -2;
     } else {
       err["error"] = 0;

       std::cout <<  "value="<< value << std::endl;
       err["value"] = value;
     }
     return err;
   }
   ColumnFamilyHandle* get_handle(const std::string& str){
     ColumnFamilyHandle  *handle = nullptr;
     auto iter = std::find(cf_names.begin(),cf_names.end(),str);
     if(iter!=cf_names.end())
       handle=*(handles.begin() +(iter - cf_names.begin()));
     return handle;

   }
   Json::Value put_cf(const string& cf_name, const string& key,const string& value){
     Json::Value err;
     if (db==nullptr){
       err["error"] = -1;
       return err;
     }
     using ROCKSDB_NAMESPACE::Slice;
     auto handle = get_handle(cf_name);
     if(handle != nullptr){

       ROCKSDB_NAMESPACE::Status status  = 
         db->Put(WriteOptions(), handle , Slice(key), Slice(value));
       if(!status.ok()) {
         err["error"] = -2;
       } else {
         err["error"] = 0;

       }
     } else {
       err["error"] = 7;
     }
     return err;
   } 
   Json::Value get_cf(const string& cf_name, const string& key){
     Json::Value err;
     if (db==nullptr){
       err["error"] = -1;
       return err;
     }
     string value;
     using ROCKSDB_NAMESPACE::Slice;
     auto handle = get_handle(cf_name);
     if(handle != nullptr){

       ROCKSDB_NAMESPACE::Status status  = 
         db->Get(ReadOptions(), handle , Slice(key), &value);
       if(!status.ok()) {
         err["error"] = -2;
       } else {
         err["error"] = 0;

         std::cout <<  "value="<< value << std::endl;
         err["value"] = value;
       }
     } else {
       err["error"] = 7;
     }
     return err;
   } 
   Json::Value put(const std::string& key, const std::string& value) 
  {
     Json::Value err;
     if (db==nullptr){
       err["error"] = -1;
       return err;
     }

     using ROCKSDB_NAMESPACE::Slice;
     ROCKSDB_NAMESPACE::Status s = db->Put(WriteOptions(), Slice(key), Slice(value));
     if(!s.ok()) {
       err["error"] = -2;
     } else {
       err["error"] = 0;
     }
     return err;
   }
   Json::Value write_batch(const Json::Value &items)
   {
     Json::Value err;
     if (db == nullptr)
     {
       err["error"] = -1;
       return err;
     }
     ROCKSDB_NAMESPACE::WriteBatch batch;
     using ROCKSDB_NAMESPACE::Slice;
     for (auto const& key : items.getMemberNames()) {
       std::string value = items[key].asString();
       batch.Put(Slice(key), Slice(value));
     }
     ROCKSDB_NAMESPACE::Status s = db->Write(WriteOptions(), &batch);
     if (!s.ok())
     {
       err["error"] = -2;
     }
     else
     {
       err["error"] = 0;
     }
     return err;
   }
 };
namespace
{
  config read_config(std::string const &name, Json::Value const& root)
  {
    std::string path ;
    ROCKSDB_NAMESPACE::Options options;
    std::vector<std::string> cf;
    for(Json::Value::const_iterator itr=root.begin(); itr!=root.end(); ++itr)
    {
      const std::string key = itr.key().asString();
      if(key=="column_family"){
        Json::Value cf_names = *itr;
        cf.push_back(RDB::kDefaultColumnFamilyName);
        for (unsigned int i = 0; i< cf_names.size(); ++i){
            auto cf_name=cf_names[i].asString();          
            if(cf_name!=RDB::kDefaultColumnFamilyName)
              cf.push_back(cf_name);
        }
      } else if(key=="dbname"){
        path= (*itr).asString();
      } else if (key=="options"){

        options = options_from_json(*itr);
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        std::cout << options.compression << std::endl;
        std::cout << root << std::endl;
        std::cout << path << std::endl;
      }
    }
    return config(name,path,options,cf);
  }
}
  // Register signal handlers so that the daemon may be shut down. You may
  // also want to register for other signals, such as SIGHUP to trigger a
  // re-read of a configuration file.

  std::vector<config> configs;
  int main(int argc, char* argv[]) 
{
  std::ifstream ifs;
  if (argc < 2){
    std::cerr << "usage: <app> <config file>.json" << std::endl;
    return 0;
  }
  ifs.open(argv[1]);

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  Json::Value root0;
  if (!parseFromStream(builder, ifs, &root0, &errs)) {
    std::cout << errs << std::endl;
    return EXIT_FAILURE;
  }
  Json::Value root=root0["dbs"];
    for(Json::Value::const_iterator itr=root.begin(); itr!=root.end(); ++itr){
      configs.push_back(read_config(itr.key().asString(),*itr));
    }
  int port = 2153; //ToDo pick port number from configuration
  asio::io_context io_context;
  asio::signal_set signals(io_context, SIGINT, SIGTERM);
  signals.async_wait([&](
        const asio::error_code& error,
        int signal_number
        ){
      for(auto& item:configs){
        delete item.db;
        item.is_open = false;
      }
      io_context.stop();
      });
  server server(io_context, port);
  io_context.run();
  return EXIT_SUCCESS;
}

std::string exec_json(std::string const& inp)
{
  std::cerr<<inp<< std::endl;
  std::stringstream istr(inp);
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  Json::Value root;
  JSONCPP_STRING errs;
  Json::Value reply;
  do {

    reply["error"] = 0;
    if (!parseFromStream(builder, istr, &root, &errs)) {
      std::cout << errs << std::endl;
      reply["error"] = 1;
      break;
    }
    std::string dbName = root["dbname"].asString();
    if (dbName.empty()){
      reply["error"] = 6;
      break;
    }
    auto item_itr = std::find_if(configs.begin(),configs.end(),
        [&dbName](config const& item){ return item.name == dbName;});
    if(item_itr == configs.end()) {
      reply["error"] = 3;
      break;
    } 
    reply= item_itr->open();
    if(!item_itr->is_open) {
      break;
    } 
    std::string op = root["op"].asString();
    if (op == "put_cf") {
      std::string column = root["column"].asString();
      std::string key = root["key"].asString();
      std::string value = root["value"].asString();
      reply = item_itr->put_cf(column,key,value) ;
    }else if (op == "put") {
      std::string key = root["key"].asString();
      std::string value = root["value"].asString();
      reply = item_itr->put(key,value) ;
    }else if (op=="write_batch"){
      auto items=root["items"];
      reply = item_itr->write_batch(items);  
    } else if (op == "get"){
      std::string key = root["key"].asString();
      reply= item_itr->get(key) ;
    } else if (op == "get_cf"){
      std::string column = root["column"].asString();
      std::string key = root["key"].asString();
      reply= item_itr->get_cf(column,key) ;
    } else {
      reply["error"] = 7;
    }

  }while(0);
    Json::FastWriter writer;
    const std::string retval = writer.write(reply);
    std::cout << "retval = " <<retval << std::endl;
    return retval;
  }
