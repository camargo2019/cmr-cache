/*
* MIT License
*
* Copyright 2024 Gabriel Camargo
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
* documentation files (the “Software”), to deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*
*/

#include "socket.h"

CoreSocket::CoreSocket(boost::asio::io_context& io_context, std::string ip, short port, Cache& cache_)
    : acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(ip), port)),
      cache_(cache_), client_(0), socket_(io_context) {
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
        acceptor_.set_option(boost::asio::ip::tcp::no_delay(true));
        accept();
        ping();
}

void CoreSocket::accept() noexcept {
    acceptor_.async_accept(
            [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
                if (!ec) {
                    if (client_ > ConfigConn.max_clients)  {
                        boost::asio::write(socket, boost::asio::buffer("ERROR: maximum connections reached"));
                        return;
                    }

                    std::string remoteIp = socket.remote_endpoint().address().to_string();
                    auto findIP = std::find(ConfigConn.auth.allow_ip.begin(), ConfigConn.auth.allow_ip.end(), remoteIp);
                    if (findIP == ConfigConn.auth.allow_ip.end() && ConfigConn.auth.allow_ip.size() > 0) {
                        boost::asio::write(socket, boost::asio::buffer("ERROR: IP is not on the released list"));
                        return;
                    }

                    ++client_;
                    std::make_shared<Manager>(std::move(socket), cache_, [this](){
                        --client_;
                    }, ConfigConn)->run();

                    accept();
                }
        }
    );
}

void CoreSocket::ping() noexcept {
    std::thread([this](){
        while (true) {
            if (socket_.is_open()) {
                boost::asio::write(socket_, boost::asio::buffer(std::string("PING\r\n")));
            }

            cache_.save();
            std::this_thread::sleep_for(std::chrono::seconds(15));
        }
    }).detach();
}
