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
    #include <vector>
    #include <memory>
    #include <chrono>
    #include <charconv>
    #include <iostream>
    #include <stdexcept>
    #include <functional>
    #include <string_view>
    #include <system_error>
    #include <unordered_set>
    #include <unordered_map>
    #include <boost/asio.hpp>
    #include <boost/algorithm/string.hpp>
    #include "../entities/user.h"
    #include "../entities/config.h"

    struct ManagerCommand {
        std::string set = "SET";
        std::string get = "GET";
        std::string use = "USE";
        std::string del = "DEL";
        std::string auth = "AUTH";
        std::string keys = "KEYS";
        
        std::unordered_set<std::string> authenticated = {
            "GET", "SET", "USE", "DEL", "KEYS"
        };

        std::unordered_set<std::string> selected_database = {
            "GET", "SET", "DEL", "KEYS"
        };
    };

    class Manager: public std::enable_shared_from_this<Manager> {
        public:
            Manager(boost::asio::ip::tcp::socket socket, Cache& cache, std::function<void()> onDisconnect, ConfigConnect& ConfigConn);
            void run() noexcept;

            ~Manager();

        private:
            void read() noexcept;
            void result(const std::string& value) noexcept;
            void invokeAction(const std::string_view& line) noexcept;
            void invokeDel(const std::vector<std::string>& args) noexcept;
            void invokeSet(const std::vector<std::string>& args) noexcept;
            void invokeGet(const std::vector<std::string>& args) noexcept;
            void invokeAuth(const std::vector<std::string>& args) noexcept;
            void invokeUse(const std::vector<std::string>& args) noexcept;
            void invokeKeys(const std::vector<std::string>& args) noexcept;

            Cache& cache_;
            std::string data_;
            UserEntities user;
            bool saveRunning_ = false;
            std::function<void()> onDisconnect_;
            boost::asio::ip::tcp::socket socket_;
            ConfigConnect& ConfigConn_;
            ManagerCommand Command;
            std::array<char, 65536> buffer_;
            std::unordered_map<std::string, void (Manager::*)(const std::vector<std::string>& args) noexcept> command_map;
    };
#endif