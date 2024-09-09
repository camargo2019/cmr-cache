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

Manager::Manager(boost::asio::ip::tcp::socket socket, Cache& cache, std::function<void()> onDisconnect)
    : socket_(std::move(socket)), cache_(cache), onDisconnect_(onDisconnect) {}

Manager::~Manager() {
    onDisconnect_();
}

void Manager::run(){
    read();
}

void Manager::read(){
    auto self(shared_from_this());
    std::string remoteIp = socket_.remote_endpoint().address().to_string();

    auto findIP = std::find(ConfigConn.auth.allow_ip.begin(), ConfigConn.auth.allow_ip.end(), remoteIp);
    if (findIP != ConfigConn.auth.allow_ip.end() || ConfigConn.auth.allow_ip.size() == 0) {
        boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(data_), "\n",
            [this, self](boost::system::error_code ec, std::size_t length){
                if (!ec) {
                    invokeAction();
                    read();
                }
            }
        );
    }else{
        result(std::string("ERROR: IP is not on the released list"));
        socket_.close();
    }
}

void Manager::result(std::string value){
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(value),
        [this, self](boost::system::error_code ec, std::size_t){
            if (ec) socket_.close();
        }
    );
}

void Manager::invokeAction(){
    std::istringstream request(data_);
    std::string command;
    std::string value;
    std::vector<std::string> args;
    request >> command;

    while (request >> value) {
        args.push_back(value);
    }

    if (std::find(commands.auth.begin(), commands.auth.end(), command) != commands.auth.end()) {
        invokeAuth(args);
    }

    if (std::find(commands.use.begin(), commands.use.end(), command) != commands.use.end()) {
        invokeUse(args);
    }

    if (std::find(commands.get.begin(), commands.get.end(), command) != commands.get.end()) {
        invokeGet(args);
    }

    if (std::find(commands.set.begin(), commands.set.end(), command) != commands.set.end()) {
        invokeSet(args);
    }

    if (std::find(commands.del.begin(), commands.del.end(), command) != commands.del.end()) {
        invokeDel(args);
    }

    data_.clear();
}

void Manager::invokeDel(std::vector<std::string> args){
    if (user.user.empty() && user.db.empty()) return result(std::string("ERROR: you need to authenticate"));
    if (user.db == "*") return result(std::string("ERROR: use the USE DB command to choose the name of the database instance"));
    if (args[0].empty()) return result(std::string("ERROR: use DEL KEY"));

    if (!cache_.del(user.db, args[0])){
        return result(std::string("ERROR: there is no record for this key"));
    }

    scheduleSave();

    result(std::string("SUCCESS: data deleted successfully"));
}

void Manager::invokeSet(std::vector<std::string> args){
    if (user.user.empty() && user.db.empty()) return result(std::string("ERROR: you need to authenticate"));
    if (user.db == "*") return result(std::string("ERROR: use the USE DB command to choose the name of the database instance"));
    if (args[0].empty() || args[1].empty()) return result(std::string("ERROR: use SET KEY VALUE (EXPIRE)"));

    int expire = -1;
    if (args.size() > 2 && !args[2].empty()){
        try {
            expire = std::stoi(args[2]);
        }catch (std::exception& e){
            std::cout << "Failed: " << args[0] << " " << args[1] << " " << args[2] << " | " << e.what() << std::endl;
        }
    }

    if (!cache_.set(user.db, args[0], args[1], expire)){
        return result(std::string("ERROR: failed to enter data"));
    }

    scheduleSave();

    result(std::string("SUCCESS: data saved successfully"));
}

void Manager::invokeGet(std::vector<std::string> args){
    if (user.user.empty() && user.db.empty()) return result(std::string("ERROR: you need to authenticate"));
    if (user.db == "*") return result(std::string("ERROR: use the `USE DB` command to choose the name of the database instance"));
    if (args[0].empty()) return result(std::string("ERROR: use GET KEY"));

    std::string value = cache_.get(user.db, args[0]);
    if (value.empty()){
        return result(std::string("ERROR: there is no record for this key"));
    }

    result(std::string("SUCCESS: " + value));
}

void Manager::invokeUse(std::vector<std::string> args){
    if (user.user.empty() && user.db.empty()) return result(std::string("ERROR: you need to authenticate"));
    if (args[0].empty()) return result(std::string("ERROR: use the `USE DB` command to choose the name of the database instance"));

    for (const auto& auth : ConfigConn.auth.basic) {
        if (auth.user == user.user && auth.db == "*"){
            user.db = args[0];

            return result(std::string("SUCCESS: you have successfully changed the database"));
        }
    }

    return result(std::string("ERROR: you are not allowed to do this"));
}

void Manager::invokeAuth(std::vector<std::string> args){
    if (!user.user.empty() && !user.db.empty()) return result(std::string("ERROR: you are not authenticated"));
    if (args[0].empty() || args[1].empty()) return result(std::string("ERROR: use AUTH USER PASS"));

    for (const auto& auth : ConfigConn.auth.basic) {
        if (auth.user == args[0] && auth.pass == args[1]){
            user.user = auth.user;
            user.db = auth.db;

            return result(std::string("SUCCESS: authenticated"));
        }
    }

    result(std::string("ERROR: failed to authenticate"));
}

void Manager::scheduleSave(){
    if (!saveRunning_){
        saveRunning_ = true;
         std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            cache_.save();
            saveRunning_ = false;
        }).detach();
    }
}