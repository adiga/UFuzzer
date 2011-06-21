# Make file for ufuzzer project
CC=gcc
CFLAGS=-Wall
SOURCES=src/*.c src/lib/*.c
EXECUTABLE=ufuzzer

all: clean $(EXECUTABLE)

$(EXECUTABLE): 	
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

clean:
	rm $(EXECUTABLE)


