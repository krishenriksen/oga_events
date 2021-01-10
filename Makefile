CC = gcc
CCFLAGS = -W -Wall

BINARY = oga_events
LIBRARIES = -levdev -lpthread
SOURCES = "main.c"

all:
	$(CC) $(CCFLAGS) $(INCLUDES) $(SOURCES) -o $(BINARY) $(LIBRARIES)

clean:
	rm -f $(BINARY)

