#include <catch2/catch_test_macros.hpp>
#include "asio.hpp"
#include <tuple>
#include <json/json.h>
using asio::ip::tcp;
enum {max_length = 1024};
std::tuple<bool,Json::Value> query(Json::Value& root)
{
      asio::io_context io_context;
      tcp::socket s(io_context);
      tcp::resolver resolver(io_context);
      asio::connect(s, resolver.resolve("localhost", "2153"));
      const uint prefix_len = sizeof(uint32_t);
      Json::StreamWriterBuilder builder;
      std::string request = Json::writeString(builder, root);
      char out_data_[max_length ];
      uint32_t length = request.length();
      strncpy(out_data_+prefix_len ,request.data(), length);
      *(uint32_t*)out_data_ = length;
      asio::write(s, asio::buffer(out_data_, length+prefix_len));
      char reply[max_length];
      size_t reply_length = asio::read(s,asio::buffer(&length, prefix_len));
      reply_length = asio::read(s,asio::buffer(reply, length));

      JSONCPP_STRING errs;
      Json::CharReaderBuilder readerBuilder;
      std::stringstream istr(std::string(reply, reply_length));
      Json::Value root0;
      bool retval = parseFromStream(readerBuilder, istr, &root0, &errs) ;
      return std::make_tuple(retval,root0);
    }
TEST_CASE( "Test get, put, putcf, getcf", "[rocks]" ) {
    // This setup will be done 4 times in total, once for each section
    

    SECTION( "simple put" ) {
      Json::Value root;
      root["dbname"] = "testdb";
      root["op"] = "put";
      root["key"] = "JeanMarie";
      root["value"] = "USA";
      bool retval;
      Json::Value root0;
      std::tie(retval,root0)= query(root);
      REQUIRE(retval);

      REQUIRE(root0["error"].asInt() == 0);
    }
    SECTION("write batch"){
      Json::Value root;
      root["dbname"] = "testdb";
      root["op"] = "write_batch";
      root["items"]["Jack1"] = "USA";
      root["items"]["Jack2"] = "USA";
      root["items"]["Jack3"] = "USA";
      bool retval;
      Json::Value root0;
      std::tie(retval,root0)= query(root);
      REQUIRE(retval);

      REQUIRE(root0["error"].asInt() == 0);
    }
    SECTION( "simple get" ) {
      Json::Value root;
      root["dbname"] = "testdb";
      root["op"] = "get";
      root["key"] = "JeanMarie";
      bool retval;
      Json::Value root0;
      std::tie(retval,root0)= query(root);
      REQUIRE(retval);

      REQUIRE(root0["error"].asInt() == 0);
      REQUIRE(root0["value"].asString() == "USA");
    }
    SECTION( "cf put" ){
      Json::Value root;
      root["dbname"] = "testcf";
      root["column"] = "users";
      root["value"] = "NSW";
      root["op"] = "put_cf";
      root["key"] = "Joe";
      bool retval;
      Json::Value root0;
      std::tie(retval,root0)= query(root);
      REQUIRE(retval);

      REQUIRE(root0["error"].asInt() == 0);
    } 
    SECTION( "cf get" ) {
      Json::Value root;
      root["dbname"] = "testcf";
      root["column"] = "users";
      root["op"] = "get_cf";
      root["key"] = "Joe";
      bool retval;
      Json::Value root0;
      std::tie(retval,root0)= query(root);
      REQUIRE(retval);

      REQUIRE(root0["error"].asInt() == 0);
      REQUIRE(root0["value"].asString() == "NSW");
    }
}
