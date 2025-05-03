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

#pragma once

#include <boost/asio.hpp>

namespace redsafe::network
{
    class TCPServer
    {
    public:
        TCPServer(boost::asio::io_context& io_context, unsigned short port);
        void start(); // 啟動伺服器
    private:
        void do_accept();
        boost::asio::io_context&        io_context_;  // I/O 事件迴圈
        boost::asio::ip::tcp::acceptor  acceptor_;    // 監聽器
    };
}