CC=gcc
CFLAGS=-g -std=c11 -Wall -Werror
TARGET= $(DIR)nbody.c $(DIR)nbodycli.c $(DIR)nbodygui.c
DIR=src/
TEST=test/
.PHONY: clean

all: $(TARGET)
	$(CC) $(CFLAGS) $(DIR)nbody.c $(DIR)nbodycli.c -o nbody -lpthread -lm
	$(CC) $(CFLAGS) $(DIR)nbody.c $(DIR)nbodygui.c -o nbody-gui -lpthread -lm -lSDL2 -lSDL2_gfx

nbody: $(DIR)nbody.c $(DIR)nbodycli.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lm

nbody-gui: $(DIR)nbodygui.c $(DIR)nbody.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lSDL2 -lSDL2_gfx -lm

nbody-test: $(TEST)nbodytest.c $(DIR)nbody.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lcmocka -lm
	
nbody-asan: $(DIR)nbody.c $(DIR)nbodycli.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lm -fsanitize=address

benchmark: 
	cd benchmarking; bash ./bench.sh

clean:
	rm -f *.o
	rm -f nbody-gui
	rm -f nbody
	rm -f nbody-test
	rm -f nbody-asan
