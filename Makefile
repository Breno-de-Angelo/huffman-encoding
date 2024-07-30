# Variables
CC = gcc
CFLAGS = -g

# Targets and their dependencies
all: encoder decoder

# Rule to build the encoder executable
encoder: encoder.o
	$(CC) $(CFLAGS) -o encoder encoder.o

# Rule to build the decoder executable
decoder: decoder.o
	$(CC) $(CFLAGS) -o decoder decoder.o

# Rule to compile encoder.c
encoder.o: encoder.c
	$(CC) $(CFLAGS) -c encoder.c

# Rule to compile decoder.c
decoder.o: decoder.c
	$(CC) $(CFLAGS) -c decoder.c

# Clean up build files
clean:
	rm -f encoder decoder *.o

# Phony targets (targets that do not correspond to files)
.PHONY: all clean
