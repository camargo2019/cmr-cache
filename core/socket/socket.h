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

#ifndef CORE_SOCKET_H
    #define CORE_SOCKET_H

    #include <thread>
    #include <string>
    #include <atomic>
    #include <memory>
    #include <iostream>
    #include <boost/asio.hpp>
    #include "../manager/manager.cpp"


    class CoreSocket: public std::enable_shared_from_this<CoreSocket> {
        public:
            CoreSocket(boost::asio::io_context& io_context, std::string, short port, Cache& cache_);
            
        private:
            void accept() noexcept;
            void ping() noexcept;

            Cache& cache_;
            std::atomic<int> client_;
            boost::asio::ip::tcp::socket socket_;
            boost::asio::ip::tcp::acceptor acceptor_;
            ConfigConnect ConfigConn = Config::getConfigConnect();
    };

#endif