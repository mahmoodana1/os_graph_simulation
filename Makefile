CC = gcc
CFLAGS = -Iinclude -Wall
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

milestone1: dijkstra

milestone2: sim
milestone3: sim

# linking
dijkstra: build/milestone1.o build/graph.o build/dijkstra.o build/utils.o
	$(CC) build/milestone1.o build/graph.o build/dijkstra.o build/utils.o -o dijkstra $(LIBS)

sim: build/milestone2.o build/graph.o build/dijkstra.o build/gui.o build/utils.o
	$(CC) build/milestone2.o build/graph.o build/dijkstra.o build/gui.o build/utils.o -o sim $(LIBS)

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

build/milestone1.o: src/milestones/milestone1.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/milestones/milestone1.c -o build/milestone1.o

build/milestone2.o: src/milestones/milestone2.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/milestones/milestone2.c -o build/milestone2.o

clean:
	rm -rf build dijkstra sim

