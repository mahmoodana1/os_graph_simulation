CC = gcc
CFLAGS = -Iinclude -Wall
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

all: build/traffic_sim

# link all the object files ".o" together to create the final program
build/traffic_sim: build/main.o build/graph.o build/dijkstra.o build/gui.o
	$(CC) build/main.o build/graph.o build/dijkstra.o build/gui.o -o build/traffic_sim $(LIBS)

# compile each C file into an object file individually
build/graph.o: src/graph.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/graph.c -o build/graph.o

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
	rm -rf build
