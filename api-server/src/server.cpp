/******************************************************************************
   Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights Reserved.
  
   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly prohibited.
  
   Without the prior written permission of [Copyright Owner], you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source code;
     3. Distribute, display, or otherwise use this source code or its derivatives in any form.
  
   For licensing inquiries or to obtain a formal license, please contact:
*******************************************************************************/

#include "server.hpp"
#include "session.hpp"
#include "util.hpp"

using namespace boost::asio;
using tcp = ip::tcp;

namespace redsafe::network
{
    APIServer::APIServer(boost::asio::io_context& io_context,
                         boost::asio::ssl::context& ssl_ctx,
                         unsigned short port)
        : io_context_   (io_context),
          ssl_ctx_      (ssl_ctx),
          acceptor_     (io_context, { tcp::v4(), port })
    {
        std::cout << "APIServer init Successfully.\n";
    }

    void APIServer::start()
    {
        std::cout << "Server listening on port " 
                  << acceptor_.local_endpoint().port() 
                  << " …\n";
        do_accept();
    }

    void APIServer::do_accept()
    {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) 
        {
                if (!ec) 
                {
                    // 把 socket 包到 SSL Stream
                    auto stream = std::make_shared<boost::asio::ssl::stream<tcp::socket>>
                        (std::move(socket), ssl_ctx_);

                    // 非同步 TLS 握手
                    stream->async_handshake(boost::asio::ssl::stream_base::server,
                        [stream](auto ec2) 
                        {
                            if (!ec2) // 握手完成後啟動 Session
                            {
                                auto& sock = stream->lowest_layer();
                                std::cout << redsafe::util::current_timestamp()
                                          << "New connection from " 
                                          << sock.remote_endpoint().address().to_string()
                                          << ":" 
                                          << sock.remote_endpoint().port() 
                                          << "\n";

                                std::make_shared<Session>(stream)->start();
                            }
                            else
                                std::cerr << "Handshake failed: " << ec2.message() << "\n";
                        });
                } 
                else
                    std::cerr << "Accept failed: " << ec.message() << "\n";

                do_accept();
        });
    }
}