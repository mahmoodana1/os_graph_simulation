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

In Milestone 5 the shared memory only carried the per-traveler mailboxes, and there was nothing stopping two travelers from sitting on the same intersection at the same moment. Milestone 6 turns every node on the graph into a resource that has to be checked out before a car is allowed to stand on it, and the way travelers talk to the parent through shared memory was rearranged to make that work.

### Shared Memory Layout

The shared memory segment now holds two things instead of one. The first part of the segment is an array of node locks, one semaphore for every node in the graph, all initialised to one so every intersection starts as free. Right after that array sits the same per-traveler mailbox table from before. The parent calculates the total size from both parts, calls shmget and shmat once, and then just takes the address of the first byte for the node lock array and the address right after it for the traveler messages. Children that fork later inherit the same attached segment, so the same locks and the same mailboxes are seen by everyone without any extra setup.

The TravelerMsg struct also picked up a new field called queued at node. It is just an integer that a child sets to whichever node it is currently waiting on, or minus one when it is not waiting on anything. The GUI watches this field to know when to draw the small queued indicator on a passenger that is stuck waiting for its turn.

### Locking Mechanism

The rule is simple. Before any car is allowed to be parked on a node, the traveler that owns it has to hold the semaphore for that node. As soon as the car leaves the node it gives the semaphore back so somebody else can take it. So a node lock at value one means the intersection is empty and up for grabs, and a node lock at value zero means somebody is sitting on it right now.

When a child first starts moving its traveler, it does not just blindly take its starting node. It sets its queued at node field to that node, calls sem wait on the lock, and only clears the queued field once the wait returns. If the starting node happens to be free the wait returns immediately and the car appears at the start. If another traveler is already parked there the child blocks inside sem wait, and during that whole time the GUI sees the queued field is set and draws a little blinking marker on the waiting passenger so the user can tell something is happening.

The interesting part is how the lock is handed off between hops. The child does not wait until the car visually arrives at the next node to grab its lock. Instead the moment the child decides to leave the current node it releases the current lock and publishes the move into the mailbox. On the GUI side, when the car gets close to the next node, about an approach distance away, the GUI does the sem wait on the next node lock on behalf of the traveler. If the next node is free the wait returns and the car keeps rolling in like nothing happened. If the next node is taken by somebody else the GUI freezes the car just outside the intersection until the lock becomes free, which gives you that natural looking traffic jam where a car is literally waiting at the edge of a junction for the car in front to leave. This split, where the child handles the release on the way out and the GUI handles the acquire on the way in, is what keeps the visual timing of the simulation in sync with the locking.

When the traveler finally reaches its destination the child publishes its last message with next node set to minus one, sleeps for a second to give the car its parked moment at the end, and then releases the destination lock and exits. The detach routine in the parent walks the node lock array and destroys every semaphore the same way it destroys the per-traveler read and write semaphores, so the segment comes down cleanly when the program shuts.

### IPC Side of Things

The mailbox dance from Milestone 5 is still the same producer consumer pattern with the two semaphores per traveler, but the consumer side moved out of the IPC polling loop. In Milestone 5 the GUI was reading from shared memory and applying car state in the same place, and that made the queued at node updates and the lock acquires fight with the message polling. In Milestone 6 the queued node handling lives in the GUI update path next to the lock acquire, while the read from shared memory function only consumes a hop when the car is actually parked, either idle or in node wait. The write side from the child is still the same loop, where for each hop it sleeps for the dwell, releases the current lock, writes current and next into the mailbox, posts ready to read, and then waits on ready to write until the GUI says the car has finished moving.

The end result is that the IPC is doing two jobs at once. The mailbox semaphores keep the parent and the child agreeing on what hop the car is on, and the node semaphores keep all the travelers agreeing on who owns which intersection, all living in the same shared memory segment created at startup.

