# botball-gamestate

A C++ coordination library for two-robot Botball teams. Replaces the classic time-based model with a shared, real-time **Game State** synchronized over Wi-Fi — both robots know what's on the table and adapt their strategy accordingly.

Built for the Wombat controller (aarch64) as part of ECER research.

---

## Installation

### Option A — apt package (prebuilt, aarch64, DEPERECATED)

A prebuilt `.deb` for the Wombat is included in `deploy/`.

If you have set up the local package repo on your Wombat:

```bash
sudo apt install botball-gamestate
```

Or install the `.deb` directly:

```bash
sudo dpkg -i deploy/botball-gamestate_1.0.0-1_arm64.deb
```

Headers land in `/usr/include/botball/`, the static lib in `/usr/lib/aarch64-linux-gnu/`.

Link against it in your `CMakeLists.txt`:

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(GAMESTATE REQUIRED botball-gamestate)

target_include_directories(your_target PRIVATE ${GAMESTATE_INCLUDE_DIRS})
target_link_libraries(your_target PRIVATE ${GAMESTATE_LIBRARIES})
```

---

### Option B — submodule / add_subdirectory

Clone this repo into your project:

```bash
git submodule add <repo-url> libs/botball-gamestate
```

Then in your `CMakeLists.txt`:

```cmake
add_subdirectory(libs/botball-gamestate)
target_link_libraries(your_target PRIVATE botball-gamestate)
```

---

## Project Structure

```
.
├── CMakeLists.txt
├── include/                  # Public headers
│   ├── GameState.h           # Main entry point — use this in your project
│   ├── Phase.h
│   ├── PhaseState.h
│   ├── TableState.h
│   ├── Client.h
│   ├── Server.h
│   ├── Socket.h
│   └── Util.h
├── impl/                     # Implementation (.cpp files)
├── lib/
│   └── json.hpp              # nlohmann/json (bundled)
├── deploy/
│   └── botball-gamestate_1.0.0-1_arm64.deb
├── debian/                   # Debian packaging metadata
└── gamestate-logs/           # Runtime logs written here
    └── latest.log
```

---

## Usage

### 1. Define your Table State — `table.json`

Describes the initial physical state of the game table. Any value a robot might read or write during a run goes here. Keys in this file are the keys you'll use in phase conditions and in `mutate_shared_state` / `read_shared_state`.

```json
{
  "coal_in_furnace": false,
  "stacking_order": [],
  "waste_cubes_in_buffer": 0
}
```

---

### 2. Declare your Phases — `phases.json`

Each phase maps to a function you register at runtime. The library handles scheduling, precondition checking, scoring, and synchronization.

```json
[
  {
    "id": "DELIVER_COAL",
    "allowed_agent": "bot_b",
    "points": 0,
    "timeout": 15,
    "conditions": {},
    "completion": { "coal_in_furnace": true }
  },
  {
    "id": "DELIVER_WASTE",
    "allowed_agent": "bot_a",
    "points": 120,
    "timeout": 30,
    "conditions": { "coal_in_furnace": true },
    "completion": {}
  }
]
```

| Field | Description |
|---|---|
| `id` | Unique identifier — must match a key in your actions map |
| `allowed_agent` | Which robot runs this phase (`bot_a` or `bot_b`) |
| `points` | Max points this phase earns (used in the scoring algorithm) |
| `timeout` | Estimated seconds — used for timeout checking |
| `conditions` | Key-value pairs that must match the current Table State before this phase can run |
| `completion` | Table State updates applied automatically when the phase finishes |

---

### 3. Wire it up in code

**Primary robot (`bot_a`)** — starts as TCP server:

```cpp
#include <botball/GameState.h>

int main() {
    auto gs = GameState::connect_server(
        "config/game_state.json",
        "config/table.json",
        "config/phases.json"
    );

    gs->run({
        {"INIT_A",        []() { /* positioning, sensor calibration */ }},
        {"DELIVER_WASTE", [&gs]() {
            // your robot logic here
            gs->mutate_shared_state("waste_cubes_in_buffer", 3);
        }},
    });
}
```

**Secondary robot (`bot_b`)** — connects as TCP client:

```cpp
#include <botball/GameState.h>

int main() {
    auto gs = GameState::connect_client("192.168.1.42", 3000);

    gs->run({
        {"INIT_B",       []() { /* positioning, sensor calibration */ }},
        {"DELIVER_COAL", [&gs]() {
            // your robot logic here
            gs->mutate_shared_state("coal_in_furnace", true);
        }},
    });
}
```

Start **`bot_a` first** — it needs to be listening before `bot_b` tries to connect.

---

### Reading and writing the Table State

```cpp
// Write (broadcasts to peer automatically)
gs->mutate_shared_state("coal_in_furnace", true);
gs->mutate_shared_state("waste_cubes_in_buffer", 3);

// Read (thread-safe)
bool coal  = gs->read_shared_state<bool>("coal_in_furnace");
int  cubes = gs->read_shared_state<int>("waste_cubes_in_buffer");
auto order = gs->read_shared_state<std::vector<int>>("stacking_order");
```

Supported types: `bool`, `int`, `double`, `std::string`, and `std::vector` of each.

---

## How It Works

### Startup

1. `bot_a` starts as a TCP server and parses all config files.
2. `bot_b` connects and requests the full Game State.
3. Both robots run their `INIT_A` / `INIT_B` phases (positioning, calibration).
4. Light start triggers the game timer and the main loop begins.

### Main loop

Each robot independently runs a loop:
- Evaluate all open phases — check preconditions against the current Table State, check if there's enough time left.
- Score each eligible phase: `S = Kp * points - Kt * time_to_completion + Kpt * potential`
  - `potential` = average points of phases that would be unlocked by completing this one
- Execute the highest-scoring phase.
- On completion, the phase's `completion` values are written to the Table State and broadcast to the peer.

### Communication

Two message types flow between robots during a run:

| Message | When sent | What it does |
|---|---|---|
| `UPDATE_TABLE` | After `mutate_shared_state()` | Peer updates its local Table State copy |
| `UPDATE_PHASE` | On any phase status change | Peer updates its local phase registry |

Both robots maintain a background listen thread for incoming updates. All shared state access is mutex-protected.

---

## Phase Statuses

| Status | Meaning |
|---|---|
| `OPEN` | All preconditions met — ready to run |
| `BLOCKED` | Preconditions not yet met |
| `RUNNING` | Currently being executed |
| `TIMEOUT` | Not enough time left to complete |
| `DONE` | Completed successfully |

---

## Logs

Runtime logs are written to `gamestate-logs/` with a timestamped filename and a `latest.log` symlink. Useful for post-run debugging — logs include phase selection scores, state updates, and any listen thread errors.

---

## Known Limitations

- **Wi-Fi congestion** — in a busy competition hall, signal interference can cause sync delays. The library logs listen thread errors but has no automatic reconnect.
- **Phase success = code completion** — a phase is marked `DONE` when its action function returns, not when a sensor confirms the physical outcome.
- **Coefficient tuning** — `Kp`, `Kt`, `Kpt` are set in the game state config. There is no auto-tuning; bad weights mean suboptimal phase selection.
- **aarch64 only** (prebuilt) — the `.deb` targets the Wombat's ARM64 architecture. For other platforms, build from source.

---

## Related

This library accompanies the paper:

> *"Improving Robot Coordination Using a Shared Game State"*  
> Elmesiry, Kummerer, Schönthaler, Drakulic — HTBLuVA Wiener Neustadt, 2026  
> ECER (European Conference on Educational Robotics)
