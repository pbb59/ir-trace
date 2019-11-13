EXEC ?= nbody
TOP_BUILD=./build
BUILD_DIR=$(TOP_BUILD)/$(EXEC)
EMBENCH_PATH=../embench-iot

# FIND EMBENCH DIRECTORIES WITH SRC
#EMBENCH_SRC=$(EMBENCH_PATH)/src/$(EXEC)
EMBENCH_SRC=.

#C_SRCS = $(wildcard *.c) $(wildcard $(EMBENCH_SRC)/*.c)
C_SRCS=$(EXEC).c

C_SRCS_NAMES = $(foreach x, $(C_SRCS), $(notdir $x))
# OPTIMIZED LLVM IR
LL_DEPS:= $(C_SRCS_NAMES:.c=.ll)
#$(info ${LL_DEPS})
LL_DEPS:= $(patsubst %, $(BUILD_DIR)/%, $(LL_DEPS))
# UNOPTIMIZED LLVM IR
LLN_DEPS:= $(C_SRCS:.c=.lln)
LLN_DEPS:= $(patsubst %, $(BUILD_DIR)/%, $(LLN_DEPS))
# OBJECT FILES FROM LLVM IR
LLO_DEPS:= $(C_SRCS_NAMES:.c=.llo)
LLO_DEPS:= $(patsubst %, $(BUILD_DIR)/%, $(LLO_DEPS))
# OBJECT FILES
OBJ_DEPS:= $(C_SRCS:.c=.o)
OBJ_DEPS:= $(patsubst %, $(BUILD_DIR)/%, $(OBJ_DEPS))

# OBJS COMPILED DIRECTLY FROM CLANG
OBJS:= $(notdir $(C_SRCS:.c=.o))
OBJS:= $(patsubst %, $(BUILD_DIR)/%, $(OBJS))
# OBJS COMPILED VIA LLVM OPT
LL_OBJS:= $(notdir $(C_SRCS:.c=.llo))
LL_OBJS:= $(patsubst %, $(BUILD_DIR)/%, $(LL_OBJS))



# installed cross compiler gcc for riscv
CC=clang-9
OPT=opt-9

COMMON_FLAGS= -DCPU_MHZ=1000 -DWARMUP_HEAT=0
LLN_FLAGS= -S -emit-llvm -Xclang -disable-O0-optnone $(COMMON_FLAGS)
OPT_FLAGS= -S -O3
C_FLAGS = -O3 $(COMMON_FLAGS)
COPT_FLAGS = $(COMMON_FLAGS)
LINKER_FLAGS = -lm
C_INCLUDES = -I$(EMBENCH_SRC) -I.

default: build-opt

emit-llvm: $(LLN_DEPS)
	
opt-llvm: $(LLN_DEPS) $(LL_DEPS)
	
build: $(OBJ_DEPS)
	$(CC) $(OBJS) $(LINKER_FLAGS) -o $(BUILD_DIR)/$(EXEC)-orig

build-opt: $(LLN_DEPS) $(LL_DEPS) $(LLO_DEPS)
	$(CC) $(LL_OBJS) $(LINKER_FLAGS) -o $(BUILD_DIR)/$(EXEC)-opt

$(BUILD_DIR)/%.o:%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(C_FLAGS) $(C_INCLUDES) -c -o $(BUILD_DIR)/$(notdir $@) $<

%.llo:%.ll
	$(CC) $(COPT_FLAGS) -c -o $(BUILD_DIR)/$(notdir $@) $<

$(BUILD_DIR)/%.lln:%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(LLN_FLAGS) $(C_INCLUDES) -c -o $(BUILD_DIR)/$(notdir $@) $<

%.ll:%.lln
	$(OPT) $(OPT_FLAGS) -o $(BUILD_DIR)/$(notdir $@) $<

clean:
	rm -rf $(TOP_BUILD)
