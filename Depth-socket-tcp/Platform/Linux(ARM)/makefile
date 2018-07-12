CC = g++

INCPATH = -I../../ThirdParty/OpenNI2/Include \
        	    -I../../ThirdParty/LibUSB/Include \
				-I../../ThirdParty/GL/Include
				

LIBPATH = -L../../ThirdParty/OpenNI2/Lib \
                       -L../../ThirdParty/OpenNI2/Redist/Linux/ARM \
	  	  
LDLIBS += -lOpenNI2 -lusb-1.0


CFLAGS=-O0 -DLINUX -g -Wall -std=c++11 -fpermissive
LDFLAGS += -Wl,-rpath=.

CSOURCE = ../../Src/client.cpp

CTARGET = client

SSOURCE = ../../Src/server.cpp

STARGET = server

client:
	$(CC) $(CFLAGS)  $(INCPATH) $(LIBPATH)  $(CSOURCE) $(LDFLAGS) $(LDLIBS) -lpthread  -o $(CTARGET)
	cp -r ../../ThirdParty/OpenNI2/Redist/Linux/ARM/*  ./
server:
	$(CC) $(CFLAGS) $(SSOURCE) $(LDFLAGS) -lpthread -o $(STARGET)
all:
	$(CC) $(CFLAGS)  $(INCPATH) $(LIBPATH)  $(CSOURCE) $(LDFLAGS) $(LDLIBS) -lpthread  -o $(CTARGET)
	cp -r ../../ThirdParty/OpenNI2/Redist/Linux/ARM/*  ./
	$(CC) $(CFLAGS) $(SSOURCE) $(LDFLAGS) -lpthread -o $(STARGET)
clean:
	rm -f *.o $(CTARGET) $(STARGET) *~

