# Code Review — bugs, leaks, input-validation gaps

Severity legend: **C**ritical (memory corruption / crash / deadlock), **H**igh (silent wrong result or resource leak), **M**edium (robustness), **L**ow (cleanup / dead code).

---

## C1 — `loadGraph`: no validation of `N` (nodes) or `M` (edges)
`src/graph.c:30-40`, `src/graph.c:11`

`createGraph(N)` does `for (int i = 0; i < numNodes; i++) graph->adj[i] = NULL;`. But `Graph::adj` is a fixed `Node *adj[MAX_NODES]` (MAX_NODES=15). If the file specifies `N > 15` or `N < 0`, this writes out of bounds. The extra `sizeof(Node*) * numNodes` added to the malloc does not help — the array is at a fixed offset inside the struct.

Also `initGuiSetup` declares `Vector2 positions[MAX_NODES]` and indexes `[0..g->num_nodes)` — same OOB if N > MAX_NODES.

Fix: reject `N < 1 || N > MAX_NODES`; reject `M < 0`.

## C2 — `loadGraph`: negative / zero edge weight accepted
`src/graph.c:50-58`

Dijkstra is undefined on negative weights. Zero weight makes `car->t += car->speed / (float)w * dt;` (gui.c UpdateCar) divide by zero. `GetEdgeWeight` returns 1 only on miss, not on bad weight.

Fix: reject `weight <= 0`.

## C3 — `loadGraph`: traveler `src`/`dst` not bounds-checked
`src/graph.c:84-90`

Edges check `0 <= src,dst < N` but travelers don't. A bad traveler line drives:
- `solveDijkstra(g, src, dst)` → `dst[start]=0` writes OOB on `int dst[num_nodes]`.
- GUI `ctx->positions[current_node]` OOB read.

Fix: validate each traveler `src`/`dst` against `N`.

## C4 — `loadGraph`: `count` not validated before `malloc(count * sizeof(...))`
`src/graph.c:75`, `src/graph.c:79`

`count` is read as `int`. If negative, `count * sizeof(TravelerQuery)` underflows to a huge `size_t` → either huge alloc or returns NULL, producing a misleading "allocation failed" message. If `count == 0`, `malloc(0)` is implementation-defined and the subsequent `pid_t pids[travelers.count]` VLA in main.c (size 0) is **undefined behavior** in C.

Fix: require `count >= 1` (and an upper bound, see C5).

## C5 — `main.c`: VLA `pid_t pids[travelers.count]` with no upper bound
`src/main.c:30`

`MAX_TRAVELERS` is defined in `ipc.c` (=8) but unused. Nothing caps the file's traveler count → unbounded stack allocation. Combine with C4: a malicious or corrupt file can blow the stack.

Fix: enforce `count <= MAX_TRAVELERS`; promote `MAX_TRAVELERS` to a header.

## C6 — Deadlock on early window close
`src/main.c:52-60`, `src/ipc.c` (writeTravelerPathToSharedMemory)

Children block in `sem_wait(&shared_mem[i].sem_ready_to_write)` between hops. If the user closes the window before they finish, `WindowShouldClose()` returns true, the loop exits, and main reaches `waitpid(pids[i], NULL, 0)` — but the children are still waiting on a semaphore that nobody will post. **`waitpid` blocks forever.**

Same blockage if the GUI is not yet `running` (initial state): children that need >1 hop will lock the start node, post `sem_ready_to_read`, wait for `sem_ready_to_write`, and stall until the user clicks Play.

Fix on shutdown: either `kill(pids[i], SIGTERM)` before `waitpid`, or sem_post the write semaphores N times to unblock children and let them detect a "shutdown" flag.

## C7 — Signal handler is not async-signal-safe
`src/ipc.c:33-37`, `src/main.c:21-22`

`cleanup(int sig)` calls `detachShm()` which calls `sem_destroy`, `shmdt`, `shmctl`, `printf` (none async-signal-safe) and then `exit(0)` (also unsafe — runs atexit + stdio flush). On glibc `signal()` does not block re-entry; a second SIGINT during cleanup can re-enter.

Also: each forked child inherits the handler. The `getpid() == main_pid` guard prevents the child from `IPC_RMID`-ing, but the child still does `sem_destroy` on shared semaphores it doesn't own. Wait — re-read: the destroy loop is *inside* the `main_pid` branch. Good. Still unsafe due to the calls above.

Fix: set a `volatile sig_atomic_t stop = 1` flag, check it in the GUI loop, do the real cleanup in `main` after the loop. Use `sigaction` with `SA_RESTART` cleared and a one-shot mask.

---

## H1 — `ftok` key reuse leaks SHM across crashed runs
`src/ipc.c:51`

`ftok("/tmp", 'y')` returns the same key every run. If a previous run was killed (e.g. with SIGKILL, or via C7 deadlock), the SHM segment stays in the kernel. Next run, `shmget(IPC_CREAT|IPC_EXCL)` fails with `EEXIST` and exits — opaque from the user's view.

Fix: on `EEXIST` either `shmctl(..., IPC_RMID, ...)` and retry, or print a clear "leftover shm — run `ipcrm -M`" message.

## H2 — `loadGraph`: trailing-data check misses lines with whitespace
`src/graph.c:97-103`

`extra[0] != '\n' && extra[0] != '\0'` rejects `" \n"` (good) but a line that's all spaces or a comment will be reported as "more data than expected" — and a blank trailing line is silently accepted, fine. Bigger issue: the error path frees `travelers->travelers` but does **not** set `travelers->travelers = NULL` or `travelers->count = 0`. `main` never reaches the bottom-of-main `free` in that path (returns NULL → EXIT_FAILURE), so no UAF — but the convention is fragile. Same pattern in the loops above.

Fix: after `free(travelers->travelers)`, set the field to NULL and count to 0.

## H3 — `solveDijkstra` / `BuildDijkstraPath` don't validate `start`/`end`
`src/dijkstra.c` (solveDijkstra) and `src/dijkstra.c` (BuildDijkstraPath)

`dst[start] = 0;` and `parent[end]` indexing happen with no bounds check. C3 fix at the loader is the right place; this is defense in depth.

## H4 — `ApplyTravelerUpdate` doesn't bounds-check `current_node` / `next_node`
`src/gui.c` (ApplyTravelerUpdate)

`ctx->positions[current_node]` / `[next_node]` indexed without checking against `ctx->node_count`. If shared memory is ever corrupted or a child sends garbage, OOB read.

## H5 — `BuildDijkstraPath` looks like dead code with a different node cap
`src/dijkstra.c` (BuildDijkstraPath)

Hardcoded `n > 64` cap, separate buffers, duplicates `solveDijkstra`. Nothing in `main.c` calls it. Either delete or wire it back in; right now it's drift waiting to happen.

---

## M1 — `LoadTexture` failures not checked
`src/gui.c` (InitRenderer)

If `data/station0.png` is missing, raylib returns a zero-id texture and logs a warning. `UnloadTexture` on a zero-id texture is a no-op in raylib so no crash, but the cars will draw on blank tiles. Worth at least `TraceLog`-ing or checking `texture.id != 0`.

## M2 — `printf` error messages without `\n`
`src/graph.c:21` ("Could not open the file"), `src/graph.c:34` ("Error creating the graph"), `src/graph.c:36` ("Invalid file format on first line...") — missing newlines so the next shell prompt collides.

## M3 — `createGraph` over-allocates and is misleading
`src/graph.c:5`

`malloc(sizeof(Graph) + numNodes * sizeof(Node *))` — the `Graph` struct already contains `Node *adj[MAX_NODES]` at fixed offset. The extra bytes are unused. Looks like a leftover from a flexible-array attempt. Drop the addend.

## M4 — `addEdge` allocation failure swallowed
`src/graph.c:14-19`

On `malloc` failure it prints and returns; the loader never knows. The graph silently becomes inconsistent. At MAX_NODES=15 this is academic but worth fixing: return int and propagate.

## M5 — `ApplyTravelerUpdate` mallocs a 2-int array every hop
`src/gui.c` (ApplyTravelerUpdate)

`if (c->path) free(c->path); c->path = malloc(2 * sizeof(int));` — repeated alloc/free per hop. Not a leak, just churn. Either keep a fixed `int path[2]` inline on `Car`, or hold a single allocation across hops.

---

## L1 — `MAX_TRAVELERS` defined in ipc.c but never referenced
`src/ipc.c:11`. Either enforce it (see C5) or delete.

## L2 — `utils.c` redefines `#define INF 1000000` and never uses it.

## L3 — `signal(SIGINT, cleanup)` instead of `sigaction`
Older glibc resets handler after first delivery on some platforms; sigaction gives portable, documented semantics.

## L4 — `printf` from the SIGCHLD-ish loops floods stdout
`src/ipc.c` (writeTravelerPathToSharedMemory) and `src/gui.c` (UpdateCar) print `[LOCK] ...` on every event. Useful while debugging, but consider an env-flag toggle.

## L5 — `freeRenderer` calls `UnloadTexture` after `CloseWindow`
`src/main.c:62-64` order: `CloseWindow()` then `freeRenderer(ctx)`. raylib's `UnloadTexture` after `CloseWindow` is technically undefined — the GL context is gone. Swap the order: `freeRenderer(ctx); CloseWindow();`.
