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

#include "../config.hpp"
#include "../include/api-server.hpp"

#include "session.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/thread_pool.hpp>
#include <memory>
#include <thread>

using namespace boost::asio;
using tcp = ip::tcp;

namespace redsafe::apiserver
{
    class Server::Impl
    {
    public:
        explicit Impl(const Server::Options& opt)
            : io_(),
              ssl_ctx_{ssl::context::tlsv12_server},
              acceptor_{io_, {tcp::v4(), opt.port}},
              options_{opt}
        {
            ssl_ctx_.use_certificate_chain_file(opt.cert_file);
            ssl_ctx_.use_private_key_file(opt.key_file, ssl::context::pem);
        }

        ~Impl()
        {
            stop();
        }

        void start()
        {
            std::cout << "Server listening on port: "
                      << acceptor_.local_endpoint().port() 
                      << std::endl;

            do_accept();

#if  SERVER_THREAD_TYPE == 0
            std::cout << "IO Threads: 1" << std::endl;
            io_.run();
#else
            unsigned int numThreads = std::thread::hardware_concurrency();
            if (numThreads == 0) numThreads = 2;
            std::cout << "IO Threads: " << numThreads << std::endl;
#if  SERVER_THREAD_TYPE == 1
            std::vector<std::jthread> workers;
            workers.reserve(numThreads);
            for (unsigned int i = 0; i < numThreads; ++i)
                workers.emplace_back([this](const std::stop_token&){ io_.run(); });
#elif SERVER_THREAD_TYPE == 2
            pool_ = std::make_unique<thread_pool>(numThreads);
            for (unsigned int i = 0; i < numThreads; ++i)
                post(*pool_, [ctx = &io_]{ ctx->run(); });
#endif
#endif
        }

        void stop()
        {
            io_.stop();
#if SERVER_THREAD_TYPE == 2
            if (pool_)
                pool_->join();
#endif
        }

    private:
        void do_accept()
        {
            acceptor_.async_accept([this](const boost::system::error_code &ec, tcp::socket socket)
            {
                if (!ec)
                {
                    auto stream = std::make_shared<ssl::stream<tcp::socket>>
                        (std::move(socket), ssl_ctx_);

                        stream->async_handshake(ssl::stream_base::server, [stream](auto ec2)
                        {
                            if (!ec2)
                            {
                                std::make_shared<redsafe::apiserver::Session>(stream)->start();

                                const auto& sock = stream->lowest_layer();
                                std::cout << redsafe::apiserver::util::current_timestamp()
                                          << "New connection from "
                                          << sock.remote_endpoint().address().to_string()
                                          << ":"
                                          << sock.remote_endpoint().port()
                                          << "\n";
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

        io_context                     io_;
        ssl::context                   ssl_ctx_;
        ip::tcp::acceptor              acceptor_;
        Options                        options_;
        std::unique_ptr<thread_pool>   pool_;
        executor_work_guard<io_context::executor_type> guard_{io_.get_executor()};
    };

    Server::Server() : Server(Options{})
    {
        
    }

    Server::Server(const Options& opt) : impl_{std::make_unique<Impl>(opt)}
    {
    }

    Server::~Server() = default;

    void Server::start() const
    {
        impl_->start();
    }

    void Server::stop() const
    {
        impl_->stop(); 
    }
}