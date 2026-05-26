CC = gcc
CFLAGS = -Iinclude -Wall
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
BUILD_DIR = build

# the default target
all: milestone5

# create the build directory once
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Linking rules
milestone5: $(BUILD_DIR)/main.o $(BUILD_DIR)/graph.o $(BUILD_DIR)/dijkstra.o $(BUILD_DIR)/gui.o $(BUILD_DIR)/utils.o | $(BUILD_DIR)
	$(CC) $^ -o sim $(LIBS)

# creating the object files
$(BUILD_DIR)/graph.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/graph.c -o $(BUILD_DIR)/graph.o

$(BUILD_DIR)/utils.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/utils.c -o $(BUILD_DIR)/utils.o

$(BUILD_DIR)/dijkstra.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/dijkstra.c -o $(BUILD_DIR)/dijkstra.o

$(BUILD_DIR)/gui.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/gui.c -o $(BUILD_DIR)/gui.o

$(BUILD_DIR)/main.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/main.c -o $(BUILD_DIR)/main.o

clean:
	rm -rf $(BUILD_DIR) dijkstra sim

