CXX = g++ -std=c++11

DIR_CUR = $(shell pwd)
# path of libleveldb
DIR_LEVELDB_LIB = $(DIR_CUR)/../debug
DIR_LEVELDB_LIB_HEADER = $(DIR_CUR)/../include/leveldb
DIR_LEVELDB_LIB_HEADER_ = $(DIR_CUR)/../include
DIR_LEVELDB_LIB_EXTRA_HEADER_ = $(DIR_CUR)/..

LEVELDB_LIB = -lleveldb

CFLAGS := -g -Wall -O0 -Wno-unused-variable

INCLUDEFLAGS :=

INCLUDEFLAGS += -I$(DIR_LEVELDB_LIB_HEADER) -I$(DIR_LEVELDB_LIB_HEADER_) -I$(DIR_LEVELDB_LIB_EXTRA_HEADER_)
# 还真遇到 lib 连接顺序的问题啊，pthread 与 libleveldb.a 的 link 顺序反了之后会有问题
LDFLAGS := -L$(DIR_LEVELDB_LIB) $(LEVELDB_LIB) -Wl,-rpath=$(DIR_LEVELDB_LIB)
LDFLAGS += -lpthread -lunwind -lunwind-x86_64 -ldl

BIN = kv
SRCS = $(wildcard *.cpp)
DEPS = $(SRCS:.cpp=.d)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

RM = rm -rf

.PHONY:
all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o:%.cpp
	$(CXX) -c $< -o $@ $(CFLAGS) $(INCLUDEFLAGS)

%.d:%.cpp
	@set -e; \
	$(RM) $@; \
	$(CXX) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@: ,g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$
-include $(DEPS)

.PHONY:test
test:
	@echo do check

.PHONY:clean
clean:
	@$(RM) *.d *.o $(BIN)
	@echo clean done