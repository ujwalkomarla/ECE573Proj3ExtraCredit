CC=gcc
CFLAGS=-c -Wall -g 
LDFLAGS=-pthread
SOURCES=distVec.c customDefinitions.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=distance_vector


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)
.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o $(EXECUTABLE)
