# Makefile for mod_random unit tests

CC = gcc
CFLAGS = -Wall -Wextra -g -I../../src -I/usr/include/apache2 -I/usr/include/apr-1.0
LDFLAGS = -lapr-1 -laprutil-1 -lssl -lcrypto

# Source files from main module
SRC_DIR = ../../src
SOURCES = $(SRC_DIR)/mod_random_encode.c \
          $(SRC_DIR)/mod_random_crypto.c

# Test executable
TEST_EXEC = test_mod_random

# Object files
OBJS = $(SOURCES:.c=.o) test_mod_random.o

.PHONY: all clean test

all: $(TEST_EXEC)

$(TEST_EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_EXEC)
	@echo "Running test suite..."
	@./$(TEST_EXEC)

clean:
	rm -f $(TEST_EXEC) $(OBJS) $(SRC_DIR)/*.o test_mod_random.o

run: test
