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

#include "../../config.hpp"
#include "../../include/api-server.hpp"
#include "../util/logger.hpp"
#include "session.hpp"
#include "../model/model.hpp"

#include <boost/asio.hpp>
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
        explicit Impl(const uint16_t port)
            : acceptor_{io_, {tcp::v4(), port}}
        {
            acceptor_.listen(1024);
        }

        ~Impl()
        {
            stop();
        }

        void start()
        {
            loggerinit();
            do_accept();
            threadscreater();
            util::log(util::LogFile::server, util::Level::INFO)
                    << "Server listening on port: " << acceptor_.local_endpoint().port()
                    << " Threads: " << numThreads;
            clearandprintlogo(std::to_string(acceptor_.local_endpoint().port()),
                      std::to_string(numThreads)
            );
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
            acceptor_.async_accept([this](auto ec, tcp::socket socket)
                {
                    if (ec)
                    {
                        util::log(util::LogFile::server, util::Level::ERROR)
                                << "Accept failed: " << ec.message();
                        std::cerr << "Accept failed: " << ec.message() << '\n';
                    }
                    std::cout << util::current_timestamp()
                        << "nginx connection: "
                        << socket.remote_endpoint().address().to_string() << ':'
                        << socket.remote_endpoint().port() << '\n';
                    std::make_shared<Session>(std::move(socket))->start();
                    do_accept();
                }
            );
        }

        static void loggerinit()
        {
            util::LoggerManager::init(util::LogFile::server,SERVER_LOGFILE_PATH);
            util::LoggerManager::init(util::LogFile::access, ACCESS_LOGFILE_PATH);
        }

        void threadscreater()
        {
#if  SERVER_THREAD_TYPE == 0
            numThreads = 1;
            io_.run();
#else
            if (numThreads == 0) numThreads = 2;
#if  SERVER_THREAD_TYPE == 1
            workers.reserve(numThreads);
            for (unsigned int i = 0; i < numThreads; ++i)
                workers.emplace_back([this](const std::stop_token &) { io_.run(); });
#elif SERVER_THREAD_TYPE == 2
            pool_ = std::make_unique<thread_pool>(numThreads);
            for (unsigned int i = 0; i < numThreads; ++i)
                post(*pool_, [ctx = &io_]{ ctx->run(); });
#endif
#endif
        }

        static void clearandprintlogo(const std::string &port, const std::string &threadnumbers)
        {
            system("clear");
            std::cout
                    << " _____   ______  _____            _____          __\n"
                    << "|  __ \\ |  ____||  __ \\          / ____|        / _|\n"
                    << "| |__) || |__   | |  | | ______ | (___    __ _ | |_   ___\n"
                    << "|  _  / |  __|  | |  | ||______| \\___ \\  / _` ||  _| / _ \\\n"
                    << "| | \\ \\ | |____ | |__| |         ____) || (_| || |  |  __/\n"
                    << "|_|  \\_\\|______||_____/         |_____/  \\__,_||_|   \\___|\n"
                    << "-----------------------------------------------------------\n"; {
                constexpr int total_width = 59;
                constexpr int inner_width = total_width - 2;
                const std::string port_msg = "Server listening on port: " + port;
                const int pad1 = (inner_width - static_cast<int>(port_msg.size())) / 2;
                const int pad2 = inner_width - pad1 - static_cast<int>(port_msg.size());
                std::cout << "-"
                        << std::string(pad1, ' ') << port_msg << std::string(pad2, ' ')
                        << "-" << "\n";
                const std::string thread_msg = "Threads: " + threadnumbers;
                const int pad3 = (inner_width - static_cast<int>(thread_msg.size())) / 2;
                const int pad4 = inner_width - pad3 - static_cast<int>(thread_msg.size());
                std::cout << "-"
                        << std::string(pad3, ' ') << thread_msg << std::string(pad4, ' ')
                        << "-" << "\n";
            }
            std::cout << "-----------------------------------------------------------\n";
        }

        io_context                      io_;
        ip::tcp::acceptor               acceptor_;
        std::unique_ptr<thread_pool>    pool_;
        std::vector<std::jthread>       workers;
        unsigned int numThreads = std::thread::hardware_concurrency();
    };

    Server::Server(const uint16_t port) : impl_{std::make_unique<Impl>(port)}
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
