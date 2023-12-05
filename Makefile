
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

MYNAME= nwcrocks
MYLIB= 
T= $(MYNAME)
OBJS= src/db.o \
		src/json_reader.o \
		src/json_writer.o \
		src/json_value.o \
		src/networkinterface.o \
		src/options_json.o 

%.o: %.cc 
	$(CC) $(CFLAGS) -I./include -fPIC -c -o $@ $<

$T:	$(OBJS)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@  $(LIB)  $(OBJS) -lrocksdb  -lstdc++

clean:
	rm -f $T $(OBJS)



