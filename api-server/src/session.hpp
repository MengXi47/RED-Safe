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

#include "util.hpp"

#include <boost/asio.hpp>
#include <array>
#include <memory>

namespace redsafe::network
{
    /**
    * @class Session
    * @brief 管理與單一客戶端的非同步 I/O 會話
    */
    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        /**
         * @brief 建構函式
         * @param socket 已接受的 TCP 連線 socket
         */
        explicit Session(boost::asio::ip::tcp::socket socket);
    
        /**
         * @brief 啟動會話，開始非同步讀取資料
         */
        void start();
    
    private:
        /**
         * @brief 發起一次非同步讀取請求
         */
        void do_read();
    
        /**
         * @brief 讀取完成回呼
         * @param ec    錯誤代碼，ec == eof 表示對端已關閉
         * @param length 讀取到的位元組數
         */
        void on_read(boost::system::error_code ec, std::size_t length);
    
        boost::asio::ip::tcp::socket    socket_;        /// 底層 TCP socket
        std::array<char, 1024>          buffer_;        /// 接收緩衝區
    };
}