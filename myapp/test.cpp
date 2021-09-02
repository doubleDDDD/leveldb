#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <libunwind.h>
#include <cxxabi.h>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <string>
#include <condition_variable>
#include <mutex>
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "db/version_edit.h"
#include "db/log_reader.h"

void
BackTrace() 
{
    unw_cursor_t cursor;
    unw_context_t context;

    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    // Unwind frames one by one, going up the frame stack.
    while (unw_step(&cursor) > 0) {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {break;}
        std::printf("0x%lx:", pc);

        char sym[256];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
            std::printf(" (%s+0x%lx)\n", sym, offset);
        } else {
            std::printf(" -- error: unable to obtain symbol name for this frame\n");
        }
    }
}

/**
 * @brief for c++
 */
void 
BackTracePlus() {
    unw_cursor_t cursor;
    unw_context_t context;

    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    // Unwind frames one by one, going up the frame stack.
    while (unw_step(&cursor) > 0) {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {break;}
        std::printf("0x%lx:", pc);

        char sym[256];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
            char* nameptr = sym;
            int status;
            char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
            if (status == 0) {
                nameptr = demangled;
            }
            std::printf(" (%s+0x%lx)\n", nameptr, offset);
            std::free(demangled);
        } else {
            std::printf(" -- error: unable to obtain symbol name for this frame\n");
        }
    }
}

void
RunChild()
{
    return;
    sleep(2);
    std::printf("child %d started!\n", getpid());
    leveldb::DB *db = nullptr;
    leveldb::Options options; // 这玩意相当于是配置

    // 如果数据库不存在就创建
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "./testdb", &db);
    std::cout << status.ToString() << std::endl;
    // assert(status.ok());

    // writebatch batch;
    // batch.push_back(a,b);

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

    // 关闭数据库连接
    // // 保持数据库打开测试一下其它进程能否进入
    // delete db;

    return;
}

int 
Put(leveldb::DB* db_, const std::string& k, const std::string& v) 
{
    leveldb::WriteOptions write_options;
    write_options.sync = true;
    db_->Put(leveldb::WriteOptions(), k, v);
    return 0;
}

/**
 * @brief 打印版本控制文件MANIFEST
 * 格式与log是一样的
 * 对应多个versionEdit写入的过程
 */
void
ManifestToString()
{
    leveldb::SequentialFile* file;
    //MANIFEST files
    leveldb::Status status = leveldb::Env::Default()->NewSequentialFile("/home/doubled/double_D/DB/leveldb/myapp/testdb/MANIFEST-000004", &file);
    std::cout << status.ToString() << std::endl;

    leveldb::log::Reader reader(file, NULL, true/*checksum*/, 0/*initial_offset*/);
    // Read all the records and add to a memtable
    std::string scratch;
    leveldb::Slice record;
    while (reader.ReadRecord(&record, &scratch) && status.ok()) {
        leveldb::VersionEdit edit;
        edit.DecodeFrom(record);
        std::cout << edit.DebugString() << std::endl;
    }
    return;
}

void
RunFather()
{
    // sleep(2);
    // std::printf("father %d started!\n", getpid());
    // 另一个进程也需要连接数据库。有file级别的锁
    leveldb::DB *db = nullptr;
    leveldb::Options options;

    options.create_if_missing = true;
    // Small write buffer
    options.write_buffer_size = 100000;
    // 打开一个已经存在的数据库，打开数据库文件，打开的对象是目录
    leveldb::Status status = leveldb::DB::Open(options, "./testdb", &db);
    if(!status.ok()){
        std::cout << status.ToString() << std::endl;
        exit(-1);
    }

    std::string get_value;
    leveldb::Status s = db->Get(leveldb::ReadOptions(), "k1", &get_value);
    std::cout<< get_value << std::endl;
    // assert(status.ok());
    // Put(db, "k1", "double_D!");  // Fill memtable.
    // Put(db, "k5", std::string(100000, 'y'));  // Trigger compaction.
    // Put(db, "k6", std::string(100000, 'z'));
    // Put(db, "k8", std::string(8000000, 'a'));

    // 关闭数据库的连接
    delete db;

    // ManifestToString();

    return;
}

int main(void)
{
    pid_t pid = fork();
    if(pid<0){
        perror("fork error!");
        exit(-1);
    }

    if( pid==0 ) { 
        RunChild();
    } else { 
        RunFather();
        waitpid(pid, NULL, 0); 
    }

    return 0;
}