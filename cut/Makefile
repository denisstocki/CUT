SRC_DIR := ./src
INC_DIR := ./inc
APP_DIR := ./app
TESTS_DIR := ./tests

MODE := app
SRC := $(wildcard $(SRC_DIR)/*.c) 

APP_SRC := $(SRC) $(wildcard $(APP_DIR)/*.c)
APP_TARGET := main.out

TEST_SRC := $(SRC) $(wildcard $(TESTS_DIR)/*.c)
TEST_TARGET := $(TESTS_DIR)/test.out

APP_OBJ := $(APP_SRC:%.c=%.o) 
TEST_OBJ := $(TEST_SRC:%.c=%.o) 

APP_DEPS := $(APP_OBJ:%.o=%.d) 
TEST_DEPS := $(TEST_OBJ:%.o=%.d) 

LIBS := pthread

CC ?= gcc 
C_FLAGS := -Wall -Wextra -Werror
DEP_FLAGS := -MMD -MP 

INCS_INC := $(foreach i, $(INC_DIR), -I$i) 
LIBS_INC := $(foreach l, $(LIBS), -l$l)

ifeq ($(CC), clang)
	C_FLAGS += -Weverything -Wno-vla -Wno-disabled-macro-expansion -Wno-deprecated-pragma
endif

ifeq ("$(origin O)", "command line")
	OPT := -O$(O)
else
	OPT := -O3
endif

ifeq ("$(origin G)", "command line")
	GGDB := -ggdb$(G)
else
	GGDB :=
endif

C_FLAGS += $(OPT) $(GGDB) $(DEP_FLAGS)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true

all: logs $(APP_TARGET)

test: $(TEST_TARGET)

$(APP_TARGET): $(APP_OBJ)
	$(CC) $(C_FLAGS) $(INCS_INC) $(APP_OBJ) -o $@ $(LIBS_INC)

$(TEST_TARGET): $(TEST_OBJ)
	$(CC) $(C_FLAGS) $(INCS_INC) $(TEST_OBJ) -o $@ $(LIBS_INC)

%.o:%.c %.d
	$(CC) $(C_FLAGS) $(INCS_INC) -c $< -o $@

clean:
	rm -rf $(APP_TARGET)
	rm -rf $(TEST_TARGET)
	rm -rf $(APP_OBJ)
	rm -rf $(TEST_OBJ)
	rm -rf $(APP_DEPS)
	rm -rf $(TEST_DEPS)
	rm -rf logs/*

$(APP_DEPS):
include $(wildcard $(APP_DEPS))

$(TEST_DEPS):
include $(wildcard $(TEST_DEPS))

logs:
	mkdir -p logs