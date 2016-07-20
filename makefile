SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
BIN := emu
SYMBOLS :=
CFLAGS := -std=c89 -g -Wall -Wextra -pedantic -Wconversion $(SYMBOLS)
LDFLAGS := -lncurses -lrt

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

debug: ; make SYMBOLS="-DDEBUG"

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	$(RM) $(BIN)
	$(RM) $(OBJ)

# debug
print-%  : ; @echo $* := $($*)
