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
    std::ifstream file("data/databases.dat");
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

std::string Cache::get(std::string db, std::string key){
    if (cache_.find(db) != cache_.end()) {
        if (cache_[db].find(key) != cache_[db].end()){
            return cache_[db][key].value;
        }
    }

    return "";
}

bool Cache::set(std::string db, std::string key, std::string value, int expire){

    CacheStruct data;
    data.value = value;
    data.expire = expire;

    cache_[db][key] = data;

    return true;
}

bool Cache::del(std::string db, std::string key){
    if (cache_.find(db) != cache_.end()) {
        if (cache_[db].find(key) != cache_[db].end()){
            cache_[db].erase(key);

            return true;
        }
    }

    return false;
}

void Cache::save(){
    std::ofstream file("data/databases.dat");
    
    size_t cacheSize = cache_.size();
    file.write(reinterpret_cast<const char*>(&cacheSize), sizeof(cacheSize));

    for (const auto& row: cache_) {
        const std::string& key = row.first;
        const std::unordered_map<std::string, CacheStruct>& map = row.second;

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
    }
}