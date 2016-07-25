#SRC := emu.c emu_test.c emu_opengl.c
SRC := emu.c emu_test.c emu_ncurses.c emu_ncurses_debug.c
OBJ := $(SRC:.c=.o)
BIN := emu
SYMBOLS := -DDEBUG
CFLAGS := -std=c89 -g -Wall -Wextra -pedantic -Wconversion $(SYMBOLS)

#LDFLAGS := -lGLEW -lglut
LDFLAGS := -lncurses

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LDFLAGS += -lrt
endif

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

run: $(BIN)
	./$(BIN) rom/PONG

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean :
	$(RM) $(BIN)
	$(RM) *.o

# debug
print-%  : ; @echo $* := $($*)
