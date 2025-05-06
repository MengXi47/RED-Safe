/******************************************************************************
   Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights Reserved.
  
   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly prohibited.
  
   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source code;
     3. Distribute, display, or otherwise use this source code or its derivatives in any form.
  
   For licensing inquiries or to obtain a formal license, please contact:
*******************************************************************************/

#include "../include/api-server.hpp"

#include "session.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>

using namespace boost::asio;
using tcp = ip::tcp;

namespace redsafe::apiserver
{
class Server::Impl
    {
    public:
        Impl(const Server::Options& opt) 
            : io_(),
              ssl_ctx_{ssl::context::tlsv12_server},
              acceptor_{io_, {tcp::v4(), opt.port}},
              options_{opt}
        {
            ssl_ctx_.use_certificate_chain_file(opt.cert_file);
            ssl_ctx_.use_private_key_file(opt.key_file, ssl::context::pem);
        }

        void start()
        {
            std::cout << "Server listening on port " 
                      << acceptor_.local_endpoint().port() 
                      << " …\n";
            do_accept();
            io_.run();
        }

        void stop()
        {
            io_.stop();
        }

    private:
        void do_accept()
        {
            acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) 
            {
                    if (!ec) 
                    {
                        // 把 socket 包到 SSL Stream
                        auto stream = std::make_shared<ssl::stream<tcp::socket>>
                            (std::move(socket), ssl_ctx_);
    
                        // 非同步 TLS 握手
                        stream->async_handshake(ssl::stream_base::server,
                            [stream](auto ec2) 
                            {
                                if (!ec2) // 握手完成後啟動 Session
                                {
                                    auto& sock = stream->lowest_layer();
                                    std::cout << redsafe::apiserver::util::current_timestamp()
                                              << "New connection from " 
                                              << sock.remote_endpoint().address().to_string()
                                              << ":" 
                                              << sock.remote_endpoint().port() 
                                              << "\n";
    
                                    std::make_shared<redsafe::apiserver::session::Session>
                                        (stream)->start();
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

        boost::asio::io_context           io_;
        boost::asio::ssl::context         ssl_ctx_;
        boost::asio::ip::tcp::acceptor    acceptor_;
        Options                           options_;
    };

    Server::Server() : Server(Options{})
    {
        
    }

    Server::Server(const Options& opt) : impl_{std::make_unique<Impl>(opt)}
    {
    }

    Server::~Server() = default;

    void Server::start()
    {
        impl_->start();
    }

    void Server::stop() 
    {
        impl_->stop(); 
    }
}