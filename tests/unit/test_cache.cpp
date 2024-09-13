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

#include <gtest/gtest.h>
#include "../../core/cache/cache.cpp"


TEST(CacheTest, SetCommand) {
    Cache cache;
    EXPECT_TRUE(cache.set("tests", "key1", "value1"));
    EXPECT_TRUE(cache.set("tests", "key2", "value2"));
}


TEST(CacheTest, GetCommand) {
    Cache cache;
    cache.set("tests", "key1", "value1");
    cache.set("tests", "key2", "value2");

    EXPECT_EQ(cache.get("tests", "key1"), "value1");
    EXPECT_EQ(cache.get("tests", "key2"), "value2");
    EXPECT_EQ(cache.get("tests", "key3"), "");
}

TEST(CacheTest, DelCommand) {
    Cache cache;
    cache.set("tests", "key1", "value1");
    cache.set("tests", "key2", "value2");

    EXPECT_TRUE(cache.del("tests", "key1"));
    EXPECT_EQ(cache.get("tests", "key1"), "");
    EXPECT_TRUE(cache.del("tests", "key3"));
}
