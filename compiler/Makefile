CC=gcc
DEPS = src/main.c src/compiler.c src/lexer.c src/parser.c src/expressions.c src/decompiler.c
OBJ = src/compiler.o src/lexer.o src/parser.o src/expressions.o
MAINP = src/main.o
MAINS = src/decompiler.o

%.o: %.c $(DEPS)
	$(CC) -Wall -std=c99 -g -c -o $@ $< $(CFLAGS)

all: make decompiler

make: $(OBJ) $(MAINP)
	$(CC) -g -o croc $^ $(CFLAGS)

decompiler: $(OBJ) $(MAINS)
	$(CC) -g -o crod $^ $(CFLAGS)

clean:
	rm -rf src/*.o croc crod