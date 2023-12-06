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
#include <sstream>
#include "networkinterface.hpp"
std::string exec_json(std::string const&);
ROCKSDB_NAMESPACE::Options options ;
ROCKSDB_NAMESPACE::DB *db=nullptr;
using ROCKSDB_NAMESPACE::PinnableSlice;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteBatch;
using ROCKSDB_NAMESPACE::WriteOptions;
using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
 ROCKSDB_NAMESPACE::Options options_from_json(const Json::Value& json) ;
using asio::ip::tcp;
namespace { }
  // Register signal handlers so that the daemon may be shut down. You may
  // also want to register for other signals, such as SIGHUP to trigger a
  // re-read of a configuration file.
  int main(int argc, char* argv[]) 
{
  Json::Value root;
  std::ifstream ifs;
  ifs.open(argv[1]);

  asio::io_context io_context;
  asio::signal_set signals(io_context, SIGINT, SIGTERM);
  signals.async_wait([&](
        const asio::error_code& error,
        int signal_number
        ){
      delete db;
      io_context.stop();
      });
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  if (!parseFromStream(builder, ifs, &root, &errs)) {
    std::cout << errs << std::endl;
    return EXIT_FAILURE;
  }
  std::string path = root["dbname"].asString();
  ROCKSDB_NAMESPACE::Options options = options_from_json(root["options"]);
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  std::cout << options.compression << std::endl;
  std::cout << root << std::endl;
  ROCKSDB_NAMESPACE::Status status = ROCKSDB_NAMESPACE::DB::Open(options, path, &db);
  int port = 2153; //ToDo pick port number from configuration
  server server(io_context, port);
  io_context.run();
  return EXIT_SUCCESS;
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
Json::Value get(const std::string& key) 
{
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
    err["value"] = value;
  }
  return err;
}
std::string exec_json(std::string const& inp)
{
  std::stringstream istr(inp);
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  Json::Value root;
  JSONCPP_STRING errs;
  Json::Value reply;
  if (!parseFromStream(builder, istr, &root, &errs)) {
    std::cout << errs << std::endl;
    reply["error"] = 1;
  } else {
    reply["error"] = 0;
    std::string op = root["op"].asString();
    if (op == "put"){
      std::string key = root["key"].asString();
      std::string value = root["value"].asString();
      reply = put(key,value) ;
    } else if (op == "get"){
      std::string key = root["key"].asString();
      reply = get(key) ;
    } else {
      reply["error"] = 2;
    }
  }
  Json::StreamWriterBuilder wbuilder;
  return Json::writeString(wbuilder, root);
}
