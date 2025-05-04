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

#include "config.hpp"
#include "server.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        boost::asio::io_context     io_context_;

        // load 憑證與私鑰
        boost::asio::ssl::context   ssl_ctx{boost::asio::ssl::context::tlsv12_server};
        ssl_ctx.use_certificate_chain_file("server.crt");
        ssl_ctx.use_private_key_file("server.key", boost::asio::ssl::context::pem);

        // 把 ssl_ctx 傳給 Server
        redsafe::network::APIServer server(io_context_, ssl_ctx, SERVER_PORT);
        server.start();
        io_context_.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Server error: " << e.what() << "\n";
    }
    std::cin.get();
    return 0;
}