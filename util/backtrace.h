#ifndef STORAGE_LEVELDB_UTIL_BACKTRACE_H_
#define STORAGE_LEVELDB_UTIL_BACKTRACE_H_

#include <libunwind.h>
#include <cxxabi.h>
#include <iostream>

#define BACK_TRACE

namespace leveldb {
    void BackTrace();
    void BackTracePlus();
}  // namespace leveldb

#endif