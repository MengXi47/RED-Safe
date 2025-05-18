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

#pragma once

#ifndef SERVER_PORT
#define SERVER_PORT 30678
#endif

#ifndef SERVER_LOGFILE_PATH
#define SERVER_LOGFILE_PATH "server.log"
#endif

#ifndef ACCESS_LOGFILE_PATH
#define ACCESS_LOGFILE_PATH "access.log"
#endif

#ifndef SQL_CONNECTION_STR
#define SQL_CONNECTION_STR "host=127.0.0.1 port=5432 dbname=redsafedb user=redsafedb_user password=redsafedb_1204"
#endif

#ifndef SERVER_THREAD_TYPE
#define SERVER_THREAD_TYPE 1 // 0 -> single_thread,  1 -> std::jthread, 2 -> boost.asio.thread_pool
#endif
