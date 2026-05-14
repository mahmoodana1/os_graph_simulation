CC = gcc
CFLAGS = -Iinclude -Wall
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
BUILD_DIR = build

# the default target
.PHONY: all
all: milestone1 milestone2 milestone3 milestone4

# mark milestones as phony targets to avoid conflicts with files of the same name
.PHONY: milestone1 milestone2 milestone3 milestone4

# create the build directory once
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Linking rules
milestone1: $(BUILD_DIR)/milestone1.o $(BUILD_DIR)/graph.o $(BUILD_DIR)/dijkstra.o $(BUILD_DIR)/utils.o | $(BUILD_DIR)
	$(CC) $^ -o dijkstra $(LIBS)

milestone2: $(BUILD_DIR)/milestone2.o $(BUILD_DIR)/graph.o $(BUILD_DIR)/dijkstra.o $(BUILD_DIR)/gui.o $(BUILD_DIR)/utils.o | $(BUILD_DIR)
	$(CC) $^ -o sim $(LIBS)

milestone3: milestone2

milestone4: $(BUILD_DIR)/milestone4.o $(BUILD_DIR)/graph.o $(BUILD_DIR)/dijkstra.o $(BUILD_DIR)/gui.o $(BUILD_DIR)/utils.o | $(BUILD_DIR)
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

$(BUILD_DIR)/milestone1.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/milestones/milestone1.c -o $(BUILD_DIR)/milestone1.o

$(BUILD_DIR)/milestone2.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/milestones/milestone2.c -o $(BUILD_DIR)/milestone2.o

$(BUILD_DIR)/milestone4.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/milestones/milestone4.c -o $(BUILD_DIR)/milestone4.o

clean:
	rm -rf $(BUILD_DIR) dijkstra sim

