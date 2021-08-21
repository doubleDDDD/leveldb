#include <iostream>
#include <cassert>
#include <cstdlib>
#include <string>

// 包含必要的头文件
#include "leveldb/db.h"

int main(void)
{
    leveldb::DB *db = nullptr;
    leveldb::Options options;

    // 如果数据库不存在就创建
    options.create_if_missing = true;

    // 创建的数据库在 /tmp/testdb
    leveldb::Status status = leveldb::DB::Open(options, "./testdb", &db);
    assert(status.ok());

    std::string key = "A";
    std::string value = "a";
    std::string get_value;
    
    // 写入 key1 -> value1
    leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);

    // 写入成功，就读取 key:people 对应的 value
    if (s.ok())
        s = db->Get(leveldb::ReadOptions(), "A", &get_value);

    // 读取成功就输出
    if (s.ok())
        std::cout << get_value << std::endl;
    else
        std::cout << s.ToString() << std::endl;

    delete db;

    return 0;
}