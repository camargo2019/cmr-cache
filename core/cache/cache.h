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

#ifndef CORE_CACHE_H
    #define CORE_CACHE_H

    #include <string>
    #include <vector>
    #include <thread>
    #include <chrono>
    #include <fstream>
    #include <filesystem>
    #include <unordered_map>

    struct CacheStruct {
        std::string value;
        int expire;

        void serialize(std::ofstream& stream) const {
            size_t size = value.size();
            stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
            stream.write(value.data(), size);
            stream.write(reinterpret_cast<const char*>(&expire), sizeof(expire));
        }

        void deserialize(std::ifstream& stream) {
            size_t size;
            stream.read(reinterpret_cast<char*>(&size), sizeof(size));
            value.resize(size);
            stream.read(&value[0], size);
            stream.read(reinterpret_cast<char*>(&expire), sizeof(expire));
        }
    };

    class Cache {
        public:
            Cache ();
            std::string get(const std::string& db, const std::string& key) noexcept;
            bool set(const std::string& db, const std::string& key, const std::string& value, int expire = -1) noexcept;
            bool del(const std::string& db, const std::string& key) noexcept;
            std::vector<std::string> keys(const std::string& db) noexcept;
            void save() noexcept;
            void expire() noexcept;
            ~Cache();

        private:
            std::unordered_map<std::string, std::unordered_map<std::string, CacheStruct>> cache_;
            bool isChange = false;
    };

#endif