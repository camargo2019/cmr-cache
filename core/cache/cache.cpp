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

#include "cache.h"

Cache::Cache(){
    std::filesystem::create_directory("data");

    for (const auto& row: std::filesystem::directory_iterator("data")){
        if (row.path().extension() != ".dat") continue;

        std::ifstream file(row.path(), std::ios::binary);
        if (file.is_open()){
            size_t cacheSize;
            file.read(reinterpret_cast<char*>(&cacheSize), sizeof(cacheSize));

            for (size_t i = 0; i < cacheSize; ++i) {
                std::string key;
                size_t keySize;
                file.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
                key.resize(keySize);
                file.read(&key[0], keySize);

                std::unordered_map<std::string, CacheStruct> map;

                size_t mapSize;
                file.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

                for (size_t f = 0; f < mapSize; ++f) {
                    std::string itemKey;
                    size_t itemKeySize;
                    file.read(reinterpret_cast<char*>(&itemKeySize), sizeof(itemKeySize));
                    itemKey.resize(itemKeySize);
                    file.read(&itemKey[0], itemKeySize);

                    CacheStruct cacheStruct;
                    cacheStruct.deserialize(file);

                    map[itemKey] = cacheStruct;
                }

                cache_[key] = map;
            }
        }
    }
}

std::string Cache::get(const std::string& db, const std::string& key) noexcept {
    auto data = cache_.find(db);
    if (data != cache_.end()) {
        auto value = data->second.find(key);
        if (value != data->second.end()){
            return value->second.value;
        }
    }

    return "";
}

bool Cache::set(const std::string& db, const std::string& key, const std::string& value, int expire) noexcept {
    CacheStruct data{std::move(value), expire};
    cache_[db].insert_or_assign(key, std::move(data));
    isChange = true;

    return true;
}

bool Cache::del(const std::string& db, const std::string& key) noexcept {
    cache_[db].erase(key);
    isChange = true;

    return false;
}

std::vector<std::string> Cache::keys(const std::string& db) noexcept {
    std::vector<std::string> keys;

    for (const auto& row: cache_[db]){
        keys.push_back(row.first);
    }

    return keys;
}

void Cache::save() noexcept {
    if (isChange) {
        std::filesystem::create_directory("data");
        
        size_t cacheSize = cache_.size();

        for (const auto& row: cache_) {
            const std::string& key = row.first;
            const std::unordered_map<std::string, CacheStruct>& map = row.second;

            std::string filename = "data/" + key + ".dat";
            std::ofstream file(filename, std::ios::binary);

            file.write(reinterpret_cast<const char*>(&cacheSize), sizeof(cacheSize));

            size_t keySize = key.size();
            file.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
            file.write(key.data(), keySize);

            size_t mapSize = map.size();
            file.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

            for (const auto& rowPair: map) {
                const std::string& pairKey = rowPair.first;
                const CacheStruct& cachedata = rowPair.second;

                size_t pairKeySize = pairKey.size();
                file.write(reinterpret_cast<const char*>(&pairKeySize), sizeof(pairKeySize));
                file.write(pairKey.data(), pairKeySize);

                cachedata.serialize(file);
            }

            file.close();
        }

        isChange = false;
    }
}

void Cache::expire() noexcept {
    std::thread([this]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            for (auto& row: cache_) {
                for (auto& item: row.second) {
                    if (item.second.expire == 0){
                        del(row.first, item.first);
                    }else if(item.second.expire > 0){
                        --item.second.expire;
                    }
                }
            }
        }
    }).detach();
}

Cache::~Cache(){
    isChange = true;
    save();
}