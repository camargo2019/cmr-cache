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

#ifndef CORE_CONFIG_H
    #define CORE_CONFIG_H

    #include <vector>
    #include <fstream>
    #include <iostream>
    #include <stdexcept>
    #include <yaml.cpp>

    typedef struct {
        std::string user;
        std::string pass;
        std::string db;
    } ConfigConnectBasicAuth;

    typedef struct {
        std::vector<std::string> allow_ip;
        std::vector<ConfigConnectBasicAuth> basic;
    } ConfigConnectAuth;

    typedef struct {
        int max_clients;
       ConfigConnectAuth auth;
    } ConfigConnect;

    typedef struct {
        std::string address;
        short port;
    } ConfigHosting;

    class Config {
        public:
            static ConfigConnect getConfigConnect() {
                Yaml::Node root;
                Yaml::Parse(root, "config/connect.yaml");
                ConfigConnect config_connect;

                Yaml::Node& connect = root["connect"];
                if (connect.IsMap() == false){
                    throw std::runtime_error("ERROR: We can't find connect in config/connect.yaml");
                }

                config_connect.max_clients = connect["max-clients"].As<int>();

                Yaml::Node& auth = connect["auth"];
                if (auth.IsMap() == false){
                    throw std::runtime_error("ERROR: We can't find auth in config/connect.yaml");
                }

                Yaml::Node& authIPs = auth["allow-ip"];

                if (authIPs.IsSequence() == false){
                    throw std::runtime_error("ERROR: allow-ip is not a sequence in config/connect.yaml");
                }

                for (auto aIP = authIPs.Begin(); aIP != authIPs.End(); aIP++){
                    config_connect.auth.allow_ip.push_back((*aIP).second.As<std::string>());
                }

                Yaml::Node& basicAuth = auth["basic"];

                if (basicAuth.IsSequence() == false){
                    throw std::runtime_error("ERROR: basic-auth is not a sequence in config/connect.yaml");
                }

                for (auto bAuth = basicAuth.Begin(); bAuth != basicAuth.End(); bAuth++){
                    Yaml::Node& dataAuth = (*bAuth).second;

                    ConfigConnectBasicAuth ConnectBasic;
                    ConnectBasic.db = dataAuth["db"].As<std::string>();
                    ConnectBasic.pass = dataAuth["pass"].As<std::string>();
                    ConnectBasic.user = dataAuth["user"].As<std::string>();
                    config_connect.auth.basic.push_back(ConnectBasic);
                }

                return config_connect;
            }

            static ConfigHosting getConfigHost() {
                Yaml::Node root;
                Yaml::Parse(root, "./config/host.yaml");
                ConfigHosting host;

                Yaml::Node& rootHost = root["host"];
                if (rootHost.IsMap() == false){
                    throw std::runtime_error("ERROR: We can't find the host in config/host.yaml");
                }

                host.address = rootHost["address"].As<std::string>();
                host.port = static_cast<short>(rootHost["port"].As<int>());

                return host;
            }
    };
#endif