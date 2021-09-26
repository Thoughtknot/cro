CC=gcc
DEPS = src/main.c src/compiler.c src/lexer.c src/parser.c src/expressions.c
OBJ = src/main.o src/compiler.o src/lexer.o src/parser.o src/expressions.o

%.o: %.c $(DEPS)
	$(CC) -Wall -g -c -o $@ $< $(CFLAGS)

make: $(OBJ)
	$(CC) -g -o croc $^ $(CFLAGS)

clean:
	rm -rf src/*.o croc