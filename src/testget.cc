//
// blocking_tcp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "asio.hpp"

#include <json/json.h>
using asio::ip::tcp;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
      return 1;
    }

    asio::io_context io_context;
    const uint prefix_len = sizeof(uint32_t);
		Json::Value root;
		root["op"] = "get";
		root["key"] = "Chakra";
    Json::StreamWriterBuilder builder;
    std::string request = Json::writeString(builder, root);
    tcp::socket s(io_context);
    tcp::resolver resolver(io_context);
    asio::connect(s, resolver.resolve(argv[1], argv[2]));
    char out_data_[max_length ];
		uint32_t length = request.length();
		strncpy(out_data_+prefix_len ,request.data(), length);
		*(uint32_t*)out_data_ = length;
    std::cout << "length " << *(uint32_t*)out_data_ << std::endl;
    asio::write(s, asio::buffer(out_data_, length+prefix_len));
    std::cout << "Message sent" << std::endl;
    char reply[max_length];
    size_t reply_length = asio::read(s,asio::buffer(&length, prefix_len));
    std::cout << "length " << std::hex << length << std::endl;
    reply_length = asio::read(s,asio::buffer(reply, length));
    std::cout << "Reply is: ";
    std::cout.write(reply, reply_length);
    std::cout << "\n";
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
/**
#include "json/json.h"
#include <iostream>
 \brief Write a Value object to a string.
 * Example Usage:
 * $g++ stringWrite.cpp -ljsoncpp -std=c++11 -o stringWrite
 * $./stringWrite
 * {
 *     "action" : "run",
 *     "data" :
 *     {
 *         "number" : 1
 *     }
 * }
int main() {
  Json::Value root;
  Json::Value data;
  constexpr bool shouldUseOldWay = false;
  root["action"] = "run";
  data["number"] = 1;
  root["data"] = data;

  if (shouldUseOldWay) {
    Json::FastWriter writer;
    const std::string json_file = writer.write(root);
    std::cout << json_file << std::endl;
  } else {
    Json::StreamWriterBuilder builder;
    const std::string json_file = Json::writeString(builder, root);
    std::cout << json_file << std::endl;
  }
  return EXIT_SUCCESS;
}

 */
