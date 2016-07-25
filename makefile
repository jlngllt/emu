SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
BIN := emu
SYMBOLS := -DDEBUG
CFLAGS := -std=c89 -g -Wall -Wextra -pedantic -Wconversion $(SYMBOLS)
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
	$(RM) $(OBJ)

# debug
print-%  : ; @echo $* := $($*)
