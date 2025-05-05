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

#include <iostream>

namespace redsafe::apiserver
{
    class Server
    {
    public:
        struct Options
        {
            uint16_t    port      {443};
            std::string cert_file {"server.crt"};
            std::string key_file  {"server.key"};
        };

        explicit Server(const Options&);
        Server();
        ~Server();

        void start();
        void stop();
    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}