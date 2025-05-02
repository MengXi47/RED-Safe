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

#include "../header/server.hpp"

#include <iostream>
#include <memory>
#include <chrono>
#include <ctime>
#include <iomanip>

using namespace boost::asio;
using tcp = ip::tcp;

namespace redsafe::network
{
    TCPServer::TCPServer(io_context& io_context, unsigned short port)
        : io_context_(io_context), 
          acceptor_(io_context, { tcp::v4(), port })
    {
        std::cout << "TCPServer init Successfully.\n";
    }

    void TCPServer::start()
    {
        std::cout << "Server listening on port " 
                  << acceptor_.local_endpoint().port() 
                  << " …\n";
        do_accept();
    }

    void TCPServer::do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << current_timestamp()
                              << "New connection from " 
                              << socket.remote_endpoint().address().to_string()
                              << ":" 
                              << socket.remote_endpoint().port() 
                              << "\n";
                } else {
                    std::cerr << "Accept failed: " << ec.message() << "\n";
                }
                do_accept();
            }
        );
    }

    inline std::string TCPServer::current_timestamp()
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        auto in_time_t = system_clock::to_time_t(now);
        std::tm buf;
#ifdef _WIN32 || _WIN64
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif
        std::ostringstream oss;
        oss << '['
            << std::put_time(&buf, "%Y-%m-%d %H:%M:%S")
            << ':' << std::setw(3) << std::setfill('0') << ms.count()
            << "] ";
        return oss.str();
    }
}