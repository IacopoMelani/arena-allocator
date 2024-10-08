
# makefile for main.c arena.c and memdump.c

CC = gcc

CFLAGS = -std=c17 -Wall -Wextra -Wpedantic

DBGFLAGS = $(CFLAGS) -g -D DEBUG

help:
	@echo "make init: create directories for object files"
	@echo "make comp_test_arena: compile test_arena"
	@echo "make test_arena: run test_arena"
	@echo "make comp_test_linked_list: compile test_linked_list"
	@echo "make test_linked_list: run test_linked_list"
	@echo "make clean: remove object files and executables"

init:
	mkdir -p target/test/obj
	mkdir -p target/release/obj
	mkdir -p target/test/output

install_lib: release/arena.o
	ar rcs target/release/libarena.a target/release/obj/arena.o
	mkdir -p target/release/include
	cp src/arena.h target/release/include/arena.h
	cp src/alloc.h target/release/include/alloc.h
	tar -czf target/release/arena.tar.gz -C $(PWD)/target/release libarena.a include

comp_test_arena: test/test_arena.o test/arena.o test/memdump.o
	$(CC) $(DBGFLAGS) -o target/test/test_arena target/test/obj/test_arena.o target/test/obj/arena.o target/test/obj/memdump.o

comp_test_linked_list: test/test_linked_list.o test/arena.o test/memdump.o
	$(CC) $(DBGFLAGS) -o target/test/test_linked_list target/test/obj/test_linked_list.o target/test/obj/arena.o target/test/obj/memdump.o

comp_test_binary_tree: test/test_binary_tree.o test/arena.o test/memdump.o
	$(CC) $(DBGFLAGS) -o target/test/test_binary_tree target/test/obj/test_binary_tree.o target/test/obj/arena.o target/test/obj/memdump.o

test_all: test_arena test_linked_list test_binary_tree
test_arena: comp_test_arena
	./target/test/test_arena > target/test/output/test_arena.txt
test_linked_list: comp_test_linked_list
	./target/test/test_linked_list > target/test/output/test_linked_list.txt
test_binary_tree: comp_test_binary_tree
	./target/test/test_binary_tree > target/test/output/test_binary_tree.txt

test/test_arena.o: test/test_arena.c
	$(CC) $(DBGFLAGS) -c test/test_arena.c -o target/test/obj/test_arena.o
test/test_linked_list.o: test/test_linked_list.c
	$(CC) $(DBGFLAGS) -c test/test_linked_list.c -o target/test/obj/test_linked_list.o
test/test_binary_tree.o: test/test_binary_tree.c
	$(CC) $(DBGFLAGS) -c test/test_binary_tree.c -o target/test/obj/test_binary_tree.o
test/arena.o: src/arena.c
	$(CC) $(DBGFLAGS) -c src/arena.c -o target/test/obj/arena.o
test/memdump.o: src/memdump.c
	$(CC) $(DBGFLAGS) -c src/memdump.c -o target/test/obj/memdump.o

release/arena.o: src/arena.c
	$(CC) $(CFLAGS) -c src/arena.c -o target/release/obj/arena.o

clean:
	rm -rf target/*

.PHONY: help \
	init \
	comp_test_arena \
	comp_test_linked_list \
	comp_test_binary_tree \
	test_all \
	test_arena \
	test_linked_list \
	test_binary_tree \
	test/test_arena.o \
	test/test_linked_list.o \
	test/test_binary_tree.o \
	test/arena.o \
	test/memdump.o \
	release/arena.o \
	clean \
	install_lib