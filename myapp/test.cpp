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
#include <thread>
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "db/version_edit.h"
#include "db/log_reader.h"

#define gettid() syscall(__NR_gettid)

// for c
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

// for c++
void 
BackTracePlus() 
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
    // return;
    // sleep(2);
    // std::printf("child %d started!\n", getpid());
    // leveldb::DB *db = nullptr;
    // leveldb::Options options; // 这玩意相当于是配置

    // // 如果数据库不存在就创建
    // options.create_if_missing = true;
    // leveldb::Status status = leveldb::DB::Open(options, "./testdb", &db);
    // std::cout << status.ToString() << std::endl;
    // // assert(status.ok());

    // // writebatch batch;
    // // batch.push_back(a,b);

    // std::string key = "A";
    // std::string value = "a";
    // std::string get_value;
    
    // // 写入 key1 -> value1
    // leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);

    // // 写入成功，就读取 key:people 对应的 value
    // if (s.ok())
    //     s = db->Get(leveldb::ReadOptions(), "A", &get_value);

    // // 读取成功就输出
    // if (s.ok())
    //     std::cout << get_value << std::endl;
    // else
    //     std::cout << s.ToString() << std::endl;

    // // 关闭数据库连接
    // // // 保持数据库打开测试一下其它进程能否进入
    // // delete db;

    // return;
}

int 
Put(leveldb::DB* db_, const std::string& k, const std::string& v) 
{
    // leveldb::WriteOptions write_options;
    // write_options.sync = true;
    // db_->Put(leveldb::WriteOptions(), k, v);
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
    leveldb::Status status = leveldb::Env::Default()->NewSequentialFile("/home/doubled/double_D/DB/leveldb/myapp/testdb/MANIFEST-000013", &file);
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
    // // sleep(2);
    // // std::printf("father %d started!\n", getpid());
    // // 另一个进程也需要连接数据库。有file级别的锁
    // leveldb::DB *db = nullptr;
    // leveldb::Options options;

    // options.create_if_missing = true;
    // // Small write buffer
    // options.write_buffer_size = 100000;
    // // 打开一个已经存在的数据库，打开数据库文件，打开的对象是目录
    // leveldb::Status status = leveldb::DB::Open(options, "./testdb", &db);
    // if(!status.ok()){
    //     std::cout << status.ToString() << std::endl;
    //     exit(-1);
    // }

    // std::string get_value;
    // leveldb::Status s = db->Get(leveldb::ReadOptions(), "k1", &get_value);
    // std::cout<< get_value << std::endl;
    // // assert(status.ok());
    // // Put(db, "k1", "double_D!");  // Fill memtable.
    // // Put(db, "k5", std::string(100000, 'y'));  // Trigger compaction.
    // // Put(db, "k6", std::string(100000, 'z'));
    // // Put(db, "k8", std::string(8000000, 'a'));

    // // 关闭数据库的连接
    // delete db;

    // // ManifestToString();

    // return;
}

void FatherAndSon()
{
    pid_t pid = fork();
    if(pid<0){
        perror("fork error!");
        exit(EXIT_FAILURE);
    }

    if( pid==0 ) { 
        // RunChild();
    } else { 
        // RunFather();
        waitpid(pid, NULL, 0); 
    }
    return;
}

void 
SimpleOpen()
{
    leveldb::DB *db = nullptr;
    leveldb::Options options;

    options.create_if_missing = true;
    options.write_buffer_size = 100000;
    leveldb::Status status = leveldb::DB::Open(options, "./testdb", &db);
    if(!status.ok()){
        std::cout << status.ToString() << std::endl;
        exit(EXIT_FAILURE);
    }

    // leveldb::Status s;
    // // std::string get_value;
    // // s = db->Get(leveldb::ReadOptions(), "k1", &get_value);
    // // std::cout<< get_value << std::endl;
    // // assert(status.ok());
    // leveldb::WriteOptions write_options;
    // write_options.sync = true;
    // s = db->Put(leveldb::WriteOptions(), "k10", "k10-doubel_D");
    // s = db->Put(leveldb::WriteOptions(), "k1", std::string(100000, 'a'));
    // s = db->Put(leveldb::WriteOptions(), "k2", std::string(100000, 'b'));
    // s = db->Put(leveldb::WriteOptions(), "k3", std::string(100000, 'c'));
    // s = db->Put(leveldb::WriteOptions(), "k4", std::string(1000000, 'd'));
    // assert(status.ok());

    delete db;
    return;
}

// 分割线

class Writebatch;
class Memtable;
class LinuxEnv;
class Version;
class VersionEdit;
class VersionSet;
class SimpleLevelDB;

/**
 * @brief 写操作kv的聚合对象
 * kv的分界线不做具体处理了，这里涉及到各种数据与元数据（分界线是不应该被使用的，正确的处理方案是长度）的问题
 * 此外，rep_这个字符串本身最前面也有一个元数据，用来表明当前batch中kv的数量以及一些其它元数据等
 */
class Writebatch
{
public:
    Writebatch():rep_(""){}
    ~Writebatch(){}

    void 
    Put(std::string& userkey, std::string& value){
        rep_ += userkey + "|" + value;
    }

    std::string ToString() const {
        return rep_;
    }

    uint64_t Size() const{
        return rep_.size();
    }
private:
    std::string rep_;
};
/**
 * @brief 对应内存数据结构
 */
class Memtable
{
public:
    Memtable(){}
    ~Memtable(){}

    Memtable(Memtable const &) = delete;
    Memtable &operator=(Memtable const &) = delete;
    void InsertKV(Writebatch* batch){return;};
    void ToImmTable(){};
    // 判断当期啊memtable是否足够写下面的内容
    bool IsEnouhgh(uint64_t bytes){return true;};
private:
};

/**
 * @brief linux os的环境
 */
class LinuxEnv
{
public:
    LinuxEnv(){}
    ~LinuxEnv(){}

    LinuxEnv(LinuxEnv const &) = delete;
    LinuxEnv &operator=(LinuxEnv const &) = delete;

    bool DirExist(std::string& dirname){ return true; }
    int DeleteDir(){ return 0; }
    int CreateDir(std::string& dbname_){ return 0; }
    int LockDBDir(){ /* may be wait for lock */ return 0; };
    int CreateFile(std::string& filename, uint64_t seq){ return 0; }
    int WriteFile(std::string& filename, std::string contents){ return 0; }
    int ReadFile(std::string& filename, std::vector<std::string>){ return 0; }
    int OpenFile(std::string& filename){ return 0;}
    void Sync(){}
    void Schedule(void (*f)(void*)){}
private:
    // 
};

/**
 * @brief version用于多版本的并发控制
 */
class Version
{
public:
    Version(){}
    ~Version(){}

    void AdpplyEdit(VersionEdit& delta){}
private:
};

class VersionEdit
{
public:
    VersionEdit(){}
    ~VersionEdit(){}
    void SetFrom0to1(){return;}
    void SetFrom1to2(){return;};
    std::string ToString(){
        std::string tmp;
        return tmp;
    }
    void Encode(){}
private:
};

class VersionSet
{
public:
    VersionSet() : current_(new Version())
    {
        // levelDB启动，current_是一个空白的version，就像一个新创建的levelDB一样
        AppendVersion(current_);
    }
    ~VersionSet(){}
    /**
     * @brief 可以借助这个函数来理解一下version
     * 一个version最直观的就是拥有哪些SSTable
     * SSTable中并未保存自己所在的level以及其它的一些元信息
     * 假设一个数据库已经run很久了，version edit上将保留有很多的变化，在重新启动这个数据库的时候是需要创建一个version变量的
     * 新建的这个version变量是空白的
     *      1. 首先根据这个current_创建一个builder的辅助对象，Builder builder(this, current_); builder也是一个空的
     *      2. 然后根据version edit中的entry可以把版本重建出来了
     * 核心原因是这样的
     *      一个levelDB第一次启动的时候显然是空白的，然后经历了MANIFEST文件中的操作之后变成了当前的版本
     *      一个levelDB重启后，初始化的version显然也是空白的，这不就对上了么，所以一个空白的version+version edit确实是能够恢复到当前的版本的
     * 当一个levelDB被重启后，所有的versionEdit会被整合为一个version edit，所以空白的version施加该影响也是完全没有问题的
     *      我丢，终于理解了，一直没有想明白之前
     * 通过 versione edits 能够把一个空白的levelDB版本在内存中恢复成当前的版本的内存的描述
     *      这些描述包括当前的SSTables的序列号，以及下一个序列号是什么等等
     * levelDB重启后大都要将version edit进行一定程度的整合，如果levelDB的启动时间足够长，那么manifest文件也可以足够大
     *      levelDB重启之后一般都会整合当前的version edits 为一个version edit.
     *      这个时候manifest文件中一般会有两个项，第一个是一个空白项，代表levelDB第一次启动时的情形。而第二个version edit将之前的所有edits的描述全部整合到一起称为一个version edit
     * 这样做的好处就是每次levelDB重启即意味着manifest文件大小的缓解，但是可能会丢一些信息，但是这个权衡是没有办法的，必须要的
     */
    void Recovery(LinuxEnv* env_){
        // 读取MANIFEST文件，文件名随便写一个先
        std::string manifestfile("./manifest-x");
        std::vector<std::string> contents;
        // int ReadFile(std::string& filename, std::string& contents){ return 0; }
        env_->ReadFile(manifestfile, contents);
        ApplyEdit(contents);
        return; 
    }
    uint64_t NewFileNumber(){return 0;}  // 当前版本控制（多版本并发控制）下一个文件号是多少
    void LogAndApply(VersionEdit* delta, std::mutex* mu_){}
    void SetLastSequence(uint64_t seq){};
private:
    // version代表的是当前的版本
    void AppendVersion(Version* _version){
        if(!_version){return;}
    }

    void 
    ApplyEdit(std::vector<std::string>& records)
    {
        if(records.empty()){return;}
        Version* newversion = new Version();
        for(auto i = records.begin(); i != records.end(); ++i){
            VersionEdit delta;
            delta.Encode();
            newversion->AdpplyEdit(delta);
        }

        current_ = newversion;
    }

    // 表示当前的版本
    Version* current_;
};

// 全局变量
LinuxEnv env;
std::string dbname("./testsimpledb");
VersionSet globalVersionSet;

/**
 * @brief 一个简单的levelDB的梳理过程
 * 最简单最简单，完全不去care任何需要类化的东西
 * 仅支持linux，没有跨平台那一套东西
 * 扮演了数据库连接的实例
 */
class SimpleLevelDB
{
public:
    SimpleLevelDB(
        LinuxEnv* env, 
        VersionSet* globalVersionSet) : 
        env_(env), 
        globalVersionSet_(globalVersionSet),
        seq_(0),
        memtable_(nullptr),
        logfd_(-1)
    {}
    ~SimpleLevelDB(){}

    SimpleLevelDB(SimpleLevelDB const &) = delete;
    SimpleLevelDB &operator=(SimpleLevelDB const &) = delete;

    void
    Open() {
        // 1. Open levelDB
        int ret = OpenDB();
        if(!ret){
            perror("simple levelDB open error!\n");
            exit(EXIT_FAILURE);
        }
        return;
    }

    void 
    Put(std::string& userkey, std::string& value) {
        Writebatch batch;
        batch.Put(userkey, value);
        return Write(&batch);
    }

    /**
     * @brief 保证有足够的的空间来写memtable
     */
    void 
    GuaranteeWriteRoom(uint64_t size)
    {
        if(memtable_->IsEnouhgh(size)){ return; }
        memtable_->ToImmTable();
        memtable_ = new Memtable();
        // 尝试调度compact线程，即使当前线程被调度走，也不涉及是否释放锁的操作
        MaybeScheduleCompaction();
    }

    /**
     * @brief 这就是写操作的全部，不会涉及到compact的操作貌似是，只有一个memtable到l0的过程
     * @param  batch            desc
     */
    void 
    Write(Writebatch* batch)
    {
        // TODO 
        // 这里有一个比较巧妙的机制来保证并发的性能，前面的线程可能帮后面排队的准备好的线程把数据消化掉
        // ...
        // 这里来执行最后的写入操作
        mu_.lock();
        seq_ += 1; //insert一对kv，则自增1
        if(!memtable_){return;}
        // 先检查当前memtable的容量是否足够，如果不够的话需要创建新的memtable，并且可能调度compact线程
        GuaranteeWriteRoom(batch->Size());
        // 先写log
        std::string logfilename(std::to_string(logfd_));  // 模拟一下吧，根据句柄知己获得文件名
        env_->WriteFile(logfilename, batch->ToString());
        // 是否持久化取决于是否设置了sync
        // 再写memtable
        memtable_->InsertKV(batch);
        globalVersionSet_->SetLastSequence(seq_);

        // 是否触发memtable的持久化与重新分配，假设被触发了
        bool triger=true;
        if(!triger){return;}
        // 书写的位置理解错了，不是事后处理的，事前要完成
        // // memtable -> imm and new a memtable
        // memtable_->ToImmTable();
        // memtable_ = new Memtable();
        mu_.unlock();
        return;
    }

    /** 读写操作是可能去标记需要compact并尝试调度compact线程
     * @brief 
     * @param  userkey          desc
     * @param  ret              desc
     */
    void 
    Get(std::string& userkey, std::string* ret)
    {
        return;
    }

    void 
    Read(){
        ;
    }

private:
    friend LinuxEnv;
    
    // 打开数据库
    int OpenDB() 
    {
        int ret=0;
        mu_.lock();
        if(!env_->DirExist(dbname)) {
            // 从无到有创建数据库 
            if(!NewDB()){ exit(EXIT_FAILURE); }
        }
        // 一般的锁都是阻塞，但是levelDB明确不允许多进程共同打开，所以这里如果无法得到锁，直接报错返回
        // 如果有其它进程在并发，直接全部GG
        if(!env_->LockDBDir()){
            // 如果有其它进程正在打开数据库，则主动GG
            exit(EXIT_FAILURE);
        }

        // 根据version edit 去恢复内存中的 version
        // recovery执行完毕之后，version版本在内存中恢复正常，即依据manifest文件恢复
        // 所以到此为止，代表当前版本的current是能够准确描述当前的levelDB的状态的
        //      包括当前版本所拥有的SSTables文件，在静态条件下，就是当前SSTable的全部，不确定在奔溃恢复中是否有意义
        globalVersionSet_->Recovery(env_);

        // 一般在这个地方，log本来是与memtable对应的，但是recovery中我们假设log一定会被compact到SSTable
        uint64_t new_log_number = globalVersionSet_->NewFileNumber();  // 版本控制器来决定下一个文件的序列号是多少
        std::string logfilename("logfilename+new_log_number");
        logfd_ = env_->OpenFile(logfilename); // 打开日志文件，返回文件操作句柄
        assert(!memtable_);
        // 创建于log文件对应的memtable
        memtable_ = new Memtable();

        // 一顿操作只有先变一个版本
        VersionEdit delta;
        delta.SetFrom1to2();
        // 意味着新版本的出现
        globalVersionSet_->LogAndApply(&delta, &mu_);
        mu_.unlock();
        return ret;
    }

    int NewDB()
    {
        int ret=0;
        std::string Manifestname("MANIFEST");
        env_->CreateDir(dbname);

        // VersionEdit，这个变化是从无到有
        // 第一次产生一个MANIFEST，这是整个数据库的描述文件
        VersionEdit newdb;
        newdb.SetFrom0to1();
        env_->CreateFile(Manifestname, 1);
        env_->WriteFile(Manifestname, newdb.ToString());
        env_->Sync();

        return ret;
    }

    // 类的非静态成员函数不允许地址的直接转换
    static void 
    BGWork(void *db){
        SimpleLevelDB* db_ = reinterpret_cast<SimpleLevelDB*>(db);
    }

    void 
    MaybeScheduleCompaction(){
        // 基本就是一个模拟过程
        // void Schedule(void (*f)(void*)){} 我草还真是简单粗暴的解决方案
        env_->Schedule((void (*)(void *))(&BGWork));
    }

    LinuxEnv* env_;
    VersionSet* globalVersionSet_;
    uint64_t seq_;
    std::mutex mu_;
    Memtable* memtable_;
    uint16_t logfd_; // log文件的文件描述符
};

void 
Write(SimpleLevelDB* db_){
    // 多线程并发的写操作
    std::string usrkey(std::to_string(gettid()));
    std::string value("double's simple DB");
    for(int i=0;i<100000;++i){
        std::string _key;
        _key += usrkey;
        std::string _value;
        _value += value;
        db_->Put(_key, _value);
    }
    return;
}

void
Read(SimpleLevelDB* db_){
    // 多线程并发的读操作
    std::string usrkey(std::to_string(gettid()));
    std::string get_value;
    for(int i=0;i<100000;++i){
        std::string _key;
        _key += usrkey;
        db_->Get(_key, &get_value);
    }
    return;
}

void
RunSimpleLevelDB()
{
    SimpleLevelDB db_(&env, &globalVersionSet);
    db_.Open();

    std::thread writethreads[10];
    for(int i=0;i<10;++i){
        writethreads[i] = std::thread(Write, &db_);
    }

    std::thread readthreads[10];
    for(int i=0;i<10;++i){
        readthreads[i] = std::thread(Read, &db_);
    }

    for (auto& t: writethreads) {t.join();}
    for (auto& t: readthreads) {t.join();}
    return;
}

int main(void)
{
    // FatherAndSon();
    // ManifestToString();
    // SimpleOpen();
    RunSimpleLevelDB();
    return 0;
}