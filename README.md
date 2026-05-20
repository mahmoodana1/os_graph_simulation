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

## Input File Format

```txt
6 8          # num_nodes  num_edges
0 1 7        # src  dst  weight
0 2 2
1 3 3
2 1 4
2 3 8
3 4 2
4 5 1
1 5 20
3            # number of travelers
0 5          # traveler 1: src  dst
1 4          # traveler 2: src  dst
2 3          # traveler 3: src  dst
```

- First line: number of nodes and edges
- Next N lines: one edge per line `src dst weight`
- Then: number of travelers
- Last lines: one traveler per line `src dst`

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

# Compile the Milestone 3 target
make milestone3

# Compile the Milestone 4 target
make milestone4

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

### milestone 3 execution
```
./sim data/input.txt
```

### milestone 4 execution
```
./sim data/input.txt
```

## Milestone 4

### Multi-Process Simulation
The simulation now forks one child process per traveler. Each child prints its PID to the terminal (`[PID] started`) and then calls `pause()`, keeping it alive for the duration of its journey.

### Parent Manages Everything
The parent process:
- Reads the graph and traveler list from the input file
- Computes the shortest path (Dijkstra) for every traveler
- Forks all child processes
- Runs the raylib GUI showing all travelers animating simultaneously, each in a distinct color

### Signal Based Lifecycle
The `startGui` function was extended to accept the child PIDs. As each car reaches its destination (`CAR_ARRIVED`), the parent sends `SIGTERM` to the corresponding child, terminating it cleanly. After the GUI window closes, any remaining children are also reaped via `waitpid` — no zombie processes.

