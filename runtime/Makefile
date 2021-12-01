CC=gcc
DEPS = src/main.c src/vm.c src/object.c src/gc.c src/natives.c
OBJ = src/main.o src/vm.o src/object.o src/gc.o src/natives.o

%.o: %.c $(DEPS)
	$(CC) -Wall -std=c99 -g -c -o $@ $< $(CFLAGS)

all: make

make: $(OBJ)
	$(CC) -g -o crow $^ $(CFLAGS)

clean:
	rm -rf src/*.o crow