
EXTRACFLAGS= -fpic

INC=   -I./include
LIB=  -L/home/chakra/.local/lib
WARN= -Wall
CFLAGS= -O2 -std=c++17 $(WARN) $(INC)  -fvisibility=hidden -DTESTING_NWC
all: nwcrocks test testget
MYNAME= nwcrocks
MYTEST= test
MYLIB= 
T= $(MYNAME)
U=$(MYTEST)

JSON= \
		src/json_reader.o \
		src/json_writer.o \
		src/json_value.o 

OBJS= src/db.o  $(JSON) src/networkinterface.o src/options_json.o 

TESTOBJS= src/test.o $(JSON) 

TESTGET= src/testget.o $(JSON)
%.o: %.cc 
	$(CC) $(CFLAGS) -I./include -fPIC -c -o $@ $<

$T:	$(OBJS)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@  $(LIB)  $^ -lrocksdb  -lstdc++

testget: $(TESTGET)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@   $(TESTGET)    -lstdc++
$U: $(TESTOBJS)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@   $(TESTOBJS)    -lstdc++

clean:
	rm -f $T $(OBJS)  test testget src/test.o src/testget.o




