C_SRCS = $(wildcard *.c)
# OPTIMIZED LLVM IR
LL_DEPS = $(C_SRCS:.c=.ll)
# UNOPTIMIZED LLVM IR
LLN_DEPS = $(C_SRCS:.c=.lln)
# OBJECT FILES FROM LLVM IR
LLO_DEPS = $(C_SRCS:.c=.llo)
# OBJECT FILES
OBJ_DEPS = $(C_SRCS:.c=.o)
OBJS = $(notdir $(C_SRCS:.c=.o))
LL_OBJS = $(notdir $(C_SRCS:.c=.llo))


EXEC = nbody

# installed cross compiler gcc for riscv
CC=clang-9
OPT=opt-9

COMMON_FLAGS= -DCPU_MHZ=1000 -DWARMUP_HEAT=0
LLN_FLAGS= -S -emit-llvm -Xclang -disable-O0-optnone $(COMMON_FLAGS)
OPT_FLAGS= -S -O3
C_FLAGS = -O3 $(COMMON_FLAGS)
COPT_FLAGS = $(COMMON_FLAGS)
LINKER_FLAGS = -lm

default: build

emit-llvm: $(LLN_DEPS)
	
opt-llvm: $(LLN_DEPS) $(LL_DEPS)
	
build: $(OBJ_DEPS)
	$(CC) $(OBJS) $(LINKER_FLAGS) -o $(EXEC)-orig

build-opt: $(LLN_DEPS) $(LL_DEPS) $(LLO_DEPS)
	$(CC) $(LL_OBJS) $(LINKER_FLAGS) -o $(EXEC)-opt

%.o:%.c
	$(CC) $(C_FLAGS) -c -o $(notdir $@) $<

%.llo:%.ll
	$(CC) $(COPT_FLAGS) -c -o $(notdir $@) $<

%.lln:%.c
	$(CC) $(LLN_FLAGS) -c -o $(notdir $@) $<

%.ll:%.lln
	$(OPT) $(OPT_FLAGS) -o $(notdir $@) $<

clean:
	rm -rf *.o *.ll *.lln *.llo $(EXEC)-orig $(EXEC)-opt
