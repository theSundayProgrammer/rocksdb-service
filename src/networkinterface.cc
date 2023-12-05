#include <networkinterface.hpp>
using tcp=asio::ip::tcp;
      void session::read_rest(char* buffer, std::size_t length){
        auto self(shared_from_this());
       socket_.async_read_some(asio::buffer(buffer, length),
           [this, buffer, length, self](std::error_code ec, std::size_t len) {
            if (!ec) {
              if(len < length)
               read_rest(buffer+len, length -len);
              else
                 do_write(this->length);
            }
            else
              delete [] inp_data_;
         });
        }
            void session::do_read() {
            auto self(shared_from_this());
            try {
                size_t reply_length = asio::read(socket_,
                asio::buffer(&length, sizeof( uint32_t)));
              } catch(std::exception &ex){
                 //std::cout<<ex.what();
                 return;
              }
            if (length > 1024 *1024) {// greater than 1MB
                    //ToDo: Log message
                    return;
                    }
            inp_data_ = new char[length];
            socket_.async_read_some(asio::buffer(inp_data_, length),
                [this,  self](std::error_code ec, std::size_t len)
                {
                 if (!ec) {
                  if(len < (std::size_t)length)
                   read_rest(inp_data_+len, length -len);
                  else
                   do_write(length);
                 }
                });
	           }

      void session::do_write(uint32_t len)
      {
        auto self(shared_from_this());
        out_data_ = new char[len+sizeof(uint32_t)];
        strncpy(out_data_+sizeof(uint32_t),inp_data_, len);
        *(uint32_t*)out_data_ = length;
        //ToDO use hton
        asio::async_write(socket_, asio::buffer(out_data_, len+sizeof(uint32_t)),
            [this, self](std::error_code ec, std::size_t )
            {
            delete [] inp_data_;
            delete [] out_data_;
            if (!ec)
            {
            do_read();
            }
            });
      }


server::server(asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
        socket_(io_context)
    {
      do_accept();
    }

      void server::do_accept()
      {
        acceptor_.async_accept(socket_,
            [this](std::error_code ec)
            {
            if (!ec)
            {
            std::make_shared<session>(std::move(socket_))->start();
            }

            do_accept();
            });
      }


