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
#include <boost/asio/ssl.hpp>


namespace redsafe::network
{
    class APIServer
    {
    public:
        APIServer(boost::asio::io_context& io_context,
            boost::asio::ssl::context& ssl_ctx,
            unsigned short port);
            
        void start();
    private:
        void do_accept();
        boost::asio::io_context&        io_context_;
        boost::asio::ssl::context&      ssl_ctx_;
        boost::asio::ip::tcp::acceptor  acceptor_;
    };
}