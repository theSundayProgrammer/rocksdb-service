
LIB_PATH= $(HOME)/.local/lib
INC_PATH= $(HOME)/.local/include
LUA_LIB= -L$(LIB_PATH) -lluajit-5.1
LUA_INC= -I$(INC_PATH)
ROCKSDB_LIB= -lrocksdb
EXTRACFLAGS= -fpic

INC= $(LUA_INC) $(ROCKSDB_INC) -I./include
LIB= $(LUA_LIB) $(ROCKSDB_LIB) 
WARN= -Wall
CFLAGS= -O2 -std=c++17 $(WARN) $(INC)  -fvisibility=hidden -DTESTING_NWC
all: nwcrocks test testget
MYNAME= nwcrocks
MYTEST= test
MYLIB= 
T= $(MYNAME)
U=$(MYTEST)
OBJS= src/db.o \
		src/json_reader.o \
		src/json_writer.o \
		src/json_value.o \
		src/networkinterface.o \
		src/options_json.o 

TESTOBJS= src/test.o \
		src/json_reader.o \
		src/json_writer.o \
		src/json_value.o 
TESTGET= src/testget.o \
		src/json_reader.o \
		src/json_writer.o \
		src/json_value.o 
%.o: %.cc 
	$(CC) $(CFLAGS) -I./include -fPIC -c -o $@ $<

$T:	$(OBJS)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@  $(LIB)  $(OBJS) -lrocksdb  -lstdc++

testget: $(TESTGET)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@  $(LIB) $(TESTGET)    -lstdc++
$U: $(TESTOBJS)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@  $(LIB) $(TESTOBJS)    -lstdc++

clean:
	rm -f $T $(OBJS) testget test




