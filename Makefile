CC=g++
CFLAGS=-c -Wall 
TAILFLAGS=-lpthread
DEFINES=-D_FILE_OFFSET_BITS=64
LDFLAGS=
SOURCES=main.cpp checkThread.h GlobalData.h readPcapFile.h flowManager.h sendUdpPacket.h
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=detect

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(DEFINES) $(OBJECTS) -o $@ $(TAILFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $(DEFINES) $< -o $@ $(TAILFLAGS)
clean:
	rm -rf *o detect

