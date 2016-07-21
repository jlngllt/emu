SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
BIN := emu
SYMBOLS :=
CFLAGS := -std=c89 -g -Wall -Wextra -pedantic -Wconversion $(SYMBOLS)
LDFLAGS := -lncurses -lrt

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SYMBOLS) $^ -o $@

debug: $(OBJ)
	@echo $^
	$(CC) $(CFLAGS) $(LDFLAGS) -DDEBUG $(SYMBOLS) $^ -o $(BIN)

run: ; ./$(BIN) rom/PONG

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean :
	$(RM) $(BIN)
	$(RM) $(OBJ)

# debug
print-%  : ; @echo $* := $($*)
