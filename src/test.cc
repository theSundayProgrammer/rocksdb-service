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
    tcp::socket s(io_context);
    tcp::resolver resolver(io_context);
    asio::connect(s, resolver.resolve(argv[1], argv[2]));
    const uint prefix_len = sizeof(uint32_t);
    char out_data_[max_length +prefix_len];
		Json::Value root;
		root["op"] = "open";
    root["name"] = "testdb";
    Json::StreamWriterBuilder builder;
    std::string request = Json::writeString(builder, root);
		uint32_t length = request.length();
		strncpy(out_data_+prefix_len ,request.data(), length);
		*(uint32_t*)out_data_ = length;
    std::cout << "length " << length << std::endl;
    asio::write(s, asio::buffer(out_data_, length+prefix_len));
    std::cout << "Message sent" << std::endl;
    char reply[max_length];
    size_t reply_length = asio::read(s,asio::buffer(&length, prefix_len));
    std::cout << "length " << std::hex << length << std::endl;
    reply_length = asio::read(s,asio::buffer(reply, length));
    std::cout << "Reply is: ";
    std::cout.write(reply, reply_length);
    std::cout << "\n";
		root["value"] = "Australia";
		root["op"] = "put";
		root["key"] = "Chakra";
		root["value"] = "Australia";
    request = Json::writeString(builder, root);
		length = request.length();
		strncpy(out_data_+prefix_len ,request.data(), length);
		*(uint32_t*)out_data_ = length;
    std::cout << "length " << length << std::endl;
    asio::write(s, asio::buffer(out_data_, length+prefix_len));
    std::cout << "Message sent" << std::endl;
    reply_length = asio::read(s,asio::buffer(&length, prefix_len));
    std::cout << "length " << std::hex << length << std::endl;
    reply_length = asio::read(s,asio::buffer(reply, length));
    std::cout << "Reply from server on put: ";
    std::cout.write(reply, reply_length);
    std::cout << "\n";
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
