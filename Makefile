CC = gcc
CFLAGS = -Iinclude -Wall
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

milestone1: dijkstra

# linking
dijkstra: build/main.o build/graph.o build/dijkstra.o
	$(CC) build/main.o build/graph.o build/dijkstra.o -o dijkstra $(LIBS)

# compile each C file into an object file individually
build/graph.o: src/graph.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/graph.c -o build/graph.o

build/dijkstra.o: src/dijkstra.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/dijkstra.c -o build/dijkstra.o

build/main.o: src/main.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

clean:
	rm -rf build dijkstra
