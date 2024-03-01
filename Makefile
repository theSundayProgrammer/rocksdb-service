
EXTRACFLAGS= -fpic

INC=   -I./include
LIB=  -L/home/chakra/.local/lib
WARN= -Wall
CFLAGS= -O2 -std=c++17 $(WARN) $(INC)  -fvisibility=hidden -DTESTING_NWC
all: server \
	 catchtest

obj/json.a: \
		obj/json_reader.o \
		obj/json_writer.o \
		obj/json_value.o 
	ar rs $@ $^


SERVEROBJS= obj/server.o  obj/json.a obj/networkinterface.o obj/options_json.o 

CATCHTEST = obj/catchtest.o obj/json.a 
obj/%.o: src/%.cc 
	$(CC) $(CFLAGS) -I./include -fPIC -c -o $@ $<

server:	$(SERVEROBJS)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@  $(LIB)  $^ -lrocksdb  -lstdc++

catchtest: $(CATCHTEST)
	$(CC) $(CFLAGS)  $(EXTRACFLAGS) -o $@   $(CATCHTEST)    -lstdc++ $(LIB) -lCatch2Main -lCatch2

clean:
	rm -f $(SERVEROBJS)  testput testget testcfput obj/test.o obj/testget.o




