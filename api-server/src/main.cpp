// server.cpp
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main() {
    try {
        // 建立 io_context
        boost::asio::io_context io_context;

        // 在本機所有 IPv4 介面、12345 埠上監聽
        tcp::acceptor acceptor{io_context, tcp::endpoint(tcp::v4(), 12345)};
        std::cout << "伺服器啟動，監聽埠 12345 ..." << std::endl;

        for (;;) {
            // 接受連線
            tcp::socket socket{io_context};
            acceptor.accept(socket);
            std::cout << "新連線：" << socket.remote_endpoint() << std::endl;

            // 讀取並回傳資料
            for (;;) {
                char data[1024];
                boost::system::error_code ec;

                // 同步讀
                std::size_t length = socket.read_some(boost::asio::buffer(data), ec);
                if (ec == boost::asio::error::eof) {
                    // 客戶端關閉連線
                    std::cout << "客戶端關閉連線\n";
                    break;
                } else if (ec) {
                    throw boost::system::system_error{ec};
                }

                // 同步寫（Echo）
                boost::asio::write(socket, boost::asio::buffer(data, length));
            }
        }
    } catch (std::exception& e) {
        std::cerr << "例外：" << e.what() << std::endl;
    }

    return 0;
}
