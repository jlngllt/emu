SRC := emu.c
OBJ := $(SRC:.c=.o)
BIN := emu
CFLAGS := -std=c89 -g -Wall -Wextra -pedantic
LDFLAGS :=

$(BIN) : $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o : %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	$(RM) $(BIN)
	$(RM) $(OBJ)

# debug
print-%  : ; @echo $* := $($*)
