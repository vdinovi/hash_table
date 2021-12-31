CC=gcc
CFLAGS=-I./include -Wall -Werror
SRC_DIR=./src
BUILD_DIR=./build
TEST_DIR=./test
VENDOR_DIR=./vendor
TEST_FLAGS=-I./vendor/include
DEBUG_FLAGS=-g
TEST_SUITES=test_hash_table
DEBUGGER=lldb

.PHONY: test clean build debug

test: run_test_hash_table

build: hash_table.so

clean:
	rm -rf $(BUILD_DIR)/*

debug: test_hash_table
	$(DEBUGGER) $(BUILD_DIR)/test_hash_table


hash_table.so: hash_table.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ -shared $(addprefix $(BUILD_DIR)/, $^)

run_test_hash_table: test_hash_table
	$(BUILD_DIR)/test_hash_table

test_hash_table: test_hash_table.o hash_table.o unity.o
	$(CC) $(CFLAGS) $(TEST_FLAGS) -o $(BUILD_DIR)/$@ $(addprefix $(BUILD_DIR)/, $^)

test_hash_table.o: $(TEST_DIR)/test_hash_table.c
	$(CC) $(CFLAGS) $(TEST_FLAGS) $(DEBUG_FLAGS) -c -o $(BUILD_DIR)/$@ $^

unity.o: $(VENDOR_DIR)/src/unity.c
	$(CC) $(CFLAGS) $(TEST_FLAGS) -c -o $(BUILD_DIR)/$@ $^

hash_table.o: $(SRC_DIR)/hash_table.c
	$(CC) $(CFLAGS) -c -fpic -o $(BUILD_DIR)/$@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(BUILD_DIR)/$@ $^


