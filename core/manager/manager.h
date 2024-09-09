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

#ifndef CORE_MANAGER_H
    #define CORE_MANAGER_H

    #include <string>
    #include <string>
    #include <memory>
    #include <chrono>
    #include <iostream>
    #include <stdexcept>
    #include <functional>
    #include <unordered_map>
    #include <boost/asio.hpp>
    #include "../entities/user.h"
    #include "../entities/config.h"

    typedef struct ManagerCommands {
        std::vector<std::string> set = {"SET", "set"};
        std::vector<std::string> del = {"DEL", "del"};
        std::vector<std::string> get = {"GET", "get"};
        std::vector<std::string> auth = {"AUTH", "auth"};
        std::vector<std::string> use = {"USE", "use"};
        std::vector<std::string> keys = {"KEYS", "keys"};
    };


    class Manager: public std::enable_shared_from_this<Manager> {
        public:
            Manager(boost::asio::ip::tcp::socket socket, Cache& cache, std::function<void()> onDisconnect);
            void run();

            ~Manager();

        private:
            void read();
            void result(std::string value);
            void invokeAction();
            void invokeDel(std::vector<std::string> args);
            void invokeSet(std::vector<std::string> args);
            void invokeGet(std::vector<std::string> args);
            void invokeAuth(std::vector<std::string> args);
            void invokeUse(std::vector<std::string> args);
            void invokeKeys(std::vector<std::string> args);
            void scheduleSave();

            boost::asio::ip::tcp::socket socket_;
            std::string data_;
            Cache& cache_;
            ManagerCommands commands;
            UserEntities user;
            ConfigConnect ConfigConn = Config::getConfigConnect();
            std::function<void()> onDisconnect_;
            bool saveRunning_ = false;
    };
#endif