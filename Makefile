CFLAGS+=-std=c11
CFLAGS+=-Wall -Werror -Wextra -Wpedantic -Wno-unused-variable -Wno-unused-parameter
SRC=./src
BIN=./bin
TESTS=./tests
C_INCLUDE_PATH=$(SRC)/Classes
OUTFILE=Server
UNIT=$1

all:
	gcc $(CFLAGS) -lm -pthread -I$(C_INCLUDE_PATH) $(C_INCLUDE_PATH)/*.c $(SRC)/main.c -o $(BIN)/$(OUTFILE)

test:
	gcc $(CFLAGS) -lm -pthread -I$(C_INCLUDE_PATH) $(C_INCLUDE_PATH)/*.c $(TESTS)/test_$(unit).c -o $(BIN)/test_$(unit)

run:
	$(BIN)/$(OUTFILE)

debug: CFLAGS+=-g
debug: all

$(shell   mkdir -p $(BIN))
