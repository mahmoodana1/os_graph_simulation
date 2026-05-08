CC = gcc
CFLAGS = -Iinclude -Wall
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

milestone1: dijkstra

milestone2: sim

# linking
dijkstra: build/main.o build/graph.o build/dijkstra.o build/gui.o build/utils.o
	$(CC) build/main.o build/graph.o build/dijkstra.o build/gui.o build/utils.o -o dijkstra $(LIBS)

sim: build/main.o build/graph.o build/dijkstra.o build/gui.o build/utils.o
	$(CC) build/main.o build/graph.o build/dijkstra.o build/gui.o build/utils.o -o sim $(LIBS)

# compile each C file into an object file individually
build/graph.o: src/graph.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/graph.c -o build/graph.o

build/utils.o: src/utils.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/utils.c -o build/utils.o	

build/dijkstra.o: src/dijkstra.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/dijkstra.c -o build/dijkstra.o
  
build/gui.o: src/gui.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/gui.c -o build/gui.o

build/main.o: src/main.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

clean:
	rm -rf build dijkstra sim

