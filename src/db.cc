#define LROCKSDB_VERSION "nwc-rocksdb 0.0.1"
#define LROCKSDB_COPYRIGHT "Copyright (C) 2023 Joe Mariadassou"
#define LROCKSDB_DESCRIPTION "RocksDB C++ Lua binding"
#include "db.h"
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
//#include <sstream>
#include "networkinterface.hpp"
std::string exec_json(std::string const&);
ROCKSDB_NAMESPACE::Options options ;
using ROCKSDB_NAMESPACE::PinnableSlice;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteBatch;
using ROCKSDB_NAMESPACE::WriteOptions;
using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
 ROCKSDB_NAMESPACE::Options options_from_json(const Json::Value& json) ;
using asio::ip::tcp;
struct config{
  std::string name;
  std::string path;

  ROCKSDB_NAMESPACE::Options options;
  bool is_open=false;
  ROCKSDB_NAMESPACE::DB *db=nullptr;
  config(std::string const& name_, std::string const& path_, ROCKSDB_NAMESPACE::Options& options_):
    name(name_), path(path_), options(options_){ }
  Json::Value open(){
    Json::Value err;
    err["error"] = 0;
    if (is_open)
      return err;
    ROCKSDB_NAMESPACE::Status status = ROCKSDB_NAMESPACE::DB::Open(options, path, &db);
    if(!status.ok()){
      std::cout << "unable to open db "<< status.ToString() << std::endl;
      err["error"] = 4;
    } else {
      is_open = true;
    }
    return err;
  }
  Json::Value get(const std::string& key) 
  {
    Json::Value err;
    if (db==nullptr){
      err["error"] = -1;
      return err;
    }
    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
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
    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    return err;
  }
  Json::Value put(const std::string& key, const std::string& value) 
  {
    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
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
};
namespace
{
  config read_config(std::string const &name, Json::Value const& root)
  {
    std::string path ;
    ROCKSDB_NAMESPACE::Options options;
    for(Json::Value::const_iterator itr=root.begin(); itr!=root.end(); ++itr)
    {
      const std::string key = itr.key().asString();
      if(key=="dbname")
        path= (*itr).asString();
      else if (key=="options"){

        options = options_from_json(*itr);
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        std::cout << options.compression << std::endl;
        std::cout << root << std::endl;
        std::cout << path << std::endl;
      }
    }
    return config(name,path,options);
  }
}
  // Register signal handlers so that the daemon may be shut down. You may
  // also want to register for other signals, such as SIGHUP to trigger a
  // re-read of a configuration file.

  std::vector<config> configs;
  int main(int argc, char* argv[]) 
{
  Json::Value root;
  std::ifstream ifs;
  if (argc < 2){
    std::cerr << "usage: <app> <config file>.json" << std::endl;
    return 0;
  }
  ifs.open(argv[1]);

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
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  if (!parseFromStream(builder, ifs, &root, &errs)) {
    std::cout << errs << std::endl;
    return EXIT_FAILURE;
  }
    for(Json::Value::const_iterator itr=root.begin(); itr!=root.end(); ++itr){
      configs.push_back(read_config(itr.key().asString(),*itr));
    }
  int port = 2153; //ToDo pick port number from configuration
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
    std::string dbName = root["name"].asString();
    if (dbName.empty()){
      reply["error"] = 6;
      break;
    }
    auto item_itr = std::find_if(configs.begin(),configs.end(), [&dbName](config const& item){ return item.name == dbName;});
    if(item_itr == configs.end()) {
      reply["error"] = 3;
      break;
    } 
    reply= item_itr->open();
    if(!item_itr->is_open) {
      break;
    } 
    std::string op = root["op"].asString();
    if (op == "put") {
      std::string key = root["key"].asString();
      std::string value = root["value"].asString();
      reply = item_itr->put(key,value) ;
    } else if (op == "get"){
      std::string key = root["key"].asString();
      reply= item_itr->get(key) ;
    } else {
      reply["error"] = 7;
    }

  }while(0);
    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    Json::FastWriter writer;
    const std::string retval = writer.write(reply);
    std::cout << "retval = " <<retval << std::endl;
    return retval;
  }
