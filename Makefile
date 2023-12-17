
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
		obj/json_reader.o \
		obj/json_writer.o \
		obj/json_value.o 

OBJS= obj/db.o  $(JSON) obj/networkinterface.o obj/options_json.o 

TESTOBJS= obj/test.o $(JSON) 

TESTGET= obj/testget.o $(JSON)
obj/%.o: src/%.cc 
	$(CC) $(CFLAGS) -I./include -fPIC -c -o $@ $<

$T:	$(OBJS)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@  $(LIB)  $^ -lrocksdb  -lstdc++

testget: $(TESTGET)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@   $(TESTGET)    -lstdc++
$U: $(TESTOBJS)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@   $(TESTOBJS)    -lstdc++

clean:
	rm -f $T $(OBJS)  test testget obj/test.o obj/testget.o




