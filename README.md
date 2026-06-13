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

# Compile the Milestone 5 target
make milestone5

# Compile the Milestone 6 target
make milestone6

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

### milestone 6 execution
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

### milestone 5 execution
```
./sim data/input.txt
```

## Milestone 5

### What Changed from Milestone 4

#### Dijkstra Moved into Child Processes
In Milestone 4, the **parent** computed every traveler's shortest path before forking. In Milestone 5, that work was moved **inside each child process** — every forked child independently calls `solveDijkstra()` for its own traveler and owns the result entirely.

#### Shared Memory Channel (IPC)
Instead of the parent holding all path data, each child now communicates its path back to the parent/GUI **one hop at a time** through a shared memory segment:
- `shmget` / `shmat` allocate a segment of `N × sizeof(TravelerMsg)` (one slot per traveler).
- Each `TravelerMsg` carries `pid`, `current_node`, `next_node`, and `total_hops`.
- The GUI reads these fields and calls `ApplyTravelerUpdate()` to advance the matching car on screen.

#### Producer–Consumer Synchronisation with Semaphores
Two **unnamed POSIX semaphores** (`pshared = 1`) per traveler slot enforce a strict one-message-at-a-time handoff:

Each traveler slot has two semaphores. `sem_ready_to_write` starts at 1, meaning the slot is free and the child can write immediately. `sem_ready_to_read` starts at 0, meaning there's nothing to read yet. Once the child writes a hop and signals `sem_ready_to_read`, the GUI picks it up and signals `sem_ready_to_write` back, so the child can write the next one.

The child loops over every node in the path, calling `sem_wait(write) → write → sem_post(read)`. The GUI calls `sem_trywait(read) → consume → sem_post(write)` each frame, only when the car is `CAR_IDLE` or `CAR_NODE_WAIT`.

#### Children Exit Cleanly on Their Own
Milestone 4 children called `pause()` and were killed by the parent via `SIGTERM`. Milestone 5 children exit naturally after posting their last hop (`next_node = -1`), no signals needed for lifecycle management.

## Milestone 6

### What Changed from Milestone 5

Milestone 5 had per-traveler mailboxes in shared memory with no restriction on multiple travelers occupying the same node. Milestone 6 adds a node ownership system: every intersection is a semaphore-guarded resource that must be acquired before a car can stand on it.

### Shared Memory Layout

The segment now holds two contiguous regions: an array of node semaphores (one per graph node, all initialized to 1) followed by the same per-traveler mailbox table from Milestone 5. The parent computes the combined size, calls shmget/shmat once, and children inherit both regions automatically. TravelerMsg gained one new field, queued_at_node (the node a child is blocked waiting on, or -1 if not waiting), used by the GUI to render the queued indicator.

### Locking Mechanism

A traveler must hold a node's semaphore while its car is parked there; it releases the semaphore the moment it leaves. Semaphore value 1 = free, 0 = occupied.
Startup: the child sets queued_at_node, calls sem_wait on the starting node, then clears the field once the lock is acquired. If the node is already taken, the child blocks and the GUI shows the waiting indicator.
Mid-route handoff: the child releases the current node lock and posts the next hop to the mailbox immediately on departure. The GUI then does the sem_wait on the destination node as the car approaches it. If the next node is free, the car rolls in smoothly; if not, the GUI freezes the car just outside until the lock is released, producing a natural-looking traffic jam. This split — child releases on exit, GUI acquires on approach — keeps visual timing in sync with locking.
Arrival: the child posts a final message with next_node = -1, sleeps one second, releases the destination lock, and exits. The parent's teardown destroys all node semaphores alongside the per-traveler semaphores.

### IPC Side of Things

The producer-consumer mailbox pattern from Milestone 5 is unchanged. The only structural change is that queued_at_node handling and lock acquisition now live in the GUI update path rather than the IPC polling loop, which previously caused them to interfere with message consumption. The mailbox semaphores synchronize parent and child on hop progress; the node semaphores synchronize all travelers on intersection ownership — both living in the single shared memory segment created at startup.

