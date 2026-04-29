# OS Graph Simulation

A C-based traffic simulation on a directed weighted graph, built for Linux using `raylib`. 

## 👥 Team & Responsibilities
* **מוחמד אשהב:** Data Architect - Handles File I/O, parsing the graph data, and managing memory structures.
* **לואי נמר:** Algorithmic Lead - Responsible for implementing Dijkstra's algorithm and handling edge cases.
* **אחמד סלמן:** UI Developer - Manages the `raylib` visual implementation and node coordinate layout.
* **מחמוד אבו רמילה:** Integration & Build Master - Manages GitHub, the `Makefile`, and the integration between the backend logic and frontend UI.

## 🛠️ Prerequisites
To compile and run this project, you must be on a Linux environment and have the following installed:
* A C Compiler (e.g., `gcc`)
* `make`
* `raylib`

## 🚀 Build & Run

To ensure a clean environment and consistent results, follow these steps to compile and execute the simulation:

# **1. Prepare and Compile**
Clean any previous build artifacts and compile the project using the Milestone 1 configuration:
```bash
# Clean previous builds
make clean

# Compile the Milestone 1 target
make milestone1

# Compile the Milestone 2 target
make milestone2

```
# **2. Execute the Program**
Run the compiled binary by passing your input file as a command-line argument:
Bash

### milestone 1 execution
```
./dijkstra data/input.txt
```

### milestone 2 execution
```
./sim data/input.txt
```
