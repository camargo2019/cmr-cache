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

#include "manager.h"

Manager::Manager(boost::asio::ip::tcp::socket socket, Cache& cache, std::function<void()> onDisconnect, ConfigConnect& ConfigConn)
    : socket_(std::move(socket)), cache_(cache), onDisconnect_(onDisconnect), ConfigConn_(ConfigConn) {
        command_map[Command.auth] = &Manager::invokeAuth;
        command_map[Command.use] = &Manager::invokeUse;
        command_map[Command.get] = &Manager::invokeGet;
        command_map[Command.set] = &Manager::invokeSet;
        command_map[Command.del] = &Manager::invokeDel;
        command_map[Command.keys] = &Manager::invokeKeys;
}

Manager::~Manager() {
    onDisconnect_();
}

void Manager::run() noexcept {
    read();
}

void Manager::read() noexcept {
    auto self(shared_from_this());

    socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [this, self](const boost::system::error_code& ec, std::size_t length) {
                if (ec) {
                    std::cerr << "Erro ao ler do socket: " << ec.message() << std::endl;
                    return;
                };
                data_.append(buffer_.data(), length);

                std::size_t start = 0;
                std::size_t pos;

                while ((pos = data_.find("\r\n", start)) != std::string::npos) {
                    std::string_view line(data_.data() + start, pos - start);
                    invokeAction(line);
                    start = pos + 2;
                }

                data_.erase(0, start);
                read();
            }
    );
}

void Manager::result(const std::string& value) noexcept {
    boost::asio::async_write(socket_, boost::asio::buffer(value), [this](const boost::system::error_code& ec, std::size_t _) {});
}

void Manager::invokeAction(const std::string_view& line) noexcept {
    std::vector<std::string> args;
    char useCommand[5] = {0};

    const char* ptr = line.data();
    const char* end = ptr + line.size();

    int i = 0;
    while (ptr != end && !std::isspace(*ptr) && i < 4) {
        useCommand[i++] += std::toupper(*ptr);
        ++ptr;
    }

    auto action = command_map.find(useCommand);
    if (action == command_map.end()) return result("ERROR: incorrect command");

    if (user.user.empty() && user.db.empty() && Command.authenticated.find(useCommand) != Command.authenticated.end()) {
        return result("ERROR: you are not authenticated");
    }

    if (user.db == "*" && Command.selected_database.find(useCommand) != Command.selected_database.end()) {
        return result("ERROR: use the USE DB command to choose the name of the database instance");
    }

    while (ptr != end && std::isspace(*ptr)) ++ptr;

    std::string value;
    char quote = '\0';
    bool escape = false;

    while (ptr != end) {
        char c = *ptr++;

        if (quote) {
            if (escape) {
                value += c;
                escape = false;
            } else if (c == '\\') {
                escape = true;
            } else if (c == quote) {
                quote = '\0';
                args.push_back(std::move(value));
                value.clear();
            } else {
                value += c;
            }
        } else if (c == '"' || c == '\'') {
            quote = c;
        } else if (std::isspace(c)) {
            if (!value.empty()) {
                args.push_back(std::move(value));
                value.clear();
            }
        } else {
            value += c;
        }
    }

    if (!value.empty()) args.push_back(std::move(value));
    (this->*(action->second))(args);
}

void Manager::invokeDel(const std::vector<std::string>& args) noexcept {
    if (args[0].empty()) return result("ERROR: use DEL KEY");

    if (!cache_.del(user.db, args[0])){
        return result("ERROR: there is no record for this key");
    }

    result("SUCCESS: data deleted successfully");
}

void Manager::invokeSet(const std::vector<std::string>& args) noexcept {
    if (args.size() < 2 || args[0].empty() || args[1].empty()) return result("ERROR: use SET KEY VALUE (EXPIRE)");

    int expire = -1;
    if (args.size() > 2 && !args[2].empty()) {
        const char* begin = args[2].c_str();
        const char* end = begin + args[2].size();
        auto [ptr, ec] = std::from_chars(begin, end, expire);

        if (ec != std::errc() || ptr != end) {
            return result("ERROR: invalid expire time format");
        }
    }

    if (!cache_.set(user.db, args[0], args[1], expire)) {
        return result("ERROR: failed to enter data");
    }

    return result("SUCCESS: data saved successfully");

}

void Manager::invokeGet(const std::vector<std::string>& args) noexcept {
    if (args[0].empty()) return result("ERROR: use GET KEY");

    std::string value = cache_.get(user.db, args[0]);
    if (value.empty()){
        return result("ERROR: there is no record for this key");
    }

    result("SUCCESS: $"+std::to_string(value.size())+"\r\n" + value);
}

void Manager::invokeUse(const std::vector<std::string>& args) noexcept {
    if (args[0].empty()) return result("ERROR: use the `USE DB` command to choose the name of the database instance");

    for (const auto& auth : ConfigConn_.auth.basic) {
        if (auth.user == user.user && auth.db == "*"){
            user.db = args[0];

            return result("SUCCESS: you have successfully changed the database");
        }
    }

    result("ERROR: you are not allowed to do this");
}

void Manager::invokeAuth(const std::vector<std::string>& args) noexcept {
    if (args[0].empty() || args[1].empty()) return result("ERROR: use AUTH USER PASS");

    for (const auto& auth : ConfigConn_.auth.basic) {
        if (auth.user == args[0] && auth.pass == args[1]){
            user.user = auth.user;
            user.db = auth.db;

            return result("SUCCESS: authenticated");
        }
    }

    result("ERROR: failed to authenticate");
}

void Manager::invokeKeys(const std::vector<std::string>& args) noexcept {
    std::string messageKeys;
    std::vector<std::string> keys = cache_.keys(user.db);
    for (const std::string& key : keys) {
        messageKeys += key + "\n";
    }

    return result("SUCCESS: $"+std::to_string(messageKeys.size())+"\r\n" + messageKeys);
}

