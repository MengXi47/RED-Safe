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

#include "session.hpp"

#include <iostream>

namespace redsafe::network
{
    Session::Session(boost::asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
    {
    }

    void Session::start()
    {
        do_read();
    }

    void Session::do_read()
    {
        auto self = shared_from_this();
        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [self](boost::system::error_code ec, std::size_t length)
            {
                self->on_read(ec, length);
            }
        );
    }

    void Session::on_read(boost::system::error_code ec, std::size_t length)
    {
        if (ec)
        {
            if (ec == boost::asio::error::eof)
                std::cout << redsafe::util::current_timestamp()
                          << "Client disconnected: "
                          << socket_.remote_endpoint().address().to_string()
                          << ":" << socket_.remote_endpoint().port()
                          << "\n";
            else
                std::cerr << redsafe::util::current_timestamp()
                          << "Read error: " << ec.message()
                          << "\n";
            return;
        }
    
        // 處理讀到的資料 (例如 echo 回傳)
        auto self = shared_from_this();
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(buffer_.data(), length),
            [self](boost::system::error_code ec2, std::size_t )
            {
                if (ec2)
                    std::cerr << redsafe::util::current_timestamp()
                              << "Write error: " << ec2.message()
                              << "\n";
            }
        );
    
        // 繼續等待下一次讀取
        do_read();
    }
}