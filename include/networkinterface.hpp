
#ifndef NETWORK_INTERFACE
#define NETWORK_INTERFACE
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
  class session
    : public std::enable_shared_from_this<session> {
    public:
      session(asio::ip::tcp::socket socket)
        : socket_(std::move(socket)) {
      }

      void start() {
        do_read();
      }

    private:
      void read_rest(char* buffer, std::size_t length);
      void do_read() ;

      void do_write(uint32_t len);
      

      asio::ip::tcp::socket socket_;
      enum { max_length = 1024 };
      char* inp_data_=nullptr;
      char* out_data_=nullptr;
      uint32_t length;
  };

  class server {
    public:
      server(asio::io_context& io_context, short port);

    private:
      void do_accept();

      asio::ip::tcp::acceptor acceptor_;
      asio::ip::tcp::socket socket_;
  };
#endif
