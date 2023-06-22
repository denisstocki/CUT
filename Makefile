CC = clang
CFLAGS = -Weverything -std=c99 -pthread
VFLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes

SRC_DIR = cut/src
OBJ_DIR = cut/out

SOURCES = $(wildcard $(SRC_DIR)/**/*.c)
EXECUTABLE = main

.PHONY: all clean run

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(OBJ_DIR)/$(EXECUTABLE)

run:
	./$(OBJ_DIR)/$(EXECUTABLE)

valgrind:
	valgrind $(VFLAGS) ./$(EXECUTABLE)

clean:
	rm -rf $(OBJ_DIR)/$(EXECUTABLE)
