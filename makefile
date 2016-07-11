SRC := emu.c
OBJ := $(SRC:.c=.o)
BIN := emu
SYMBOLS :=
CFLAGS := -std=c89 -g -Wall -Wextra -pedantic -Wconversion $(SYMBOLS)
LDFLAGS := -lncurses

$(BIN) : $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	$(RM) $(BIN)
	$(RM) $(OBJ)

# debug
print-%  : ; @echo $* := $($*)
