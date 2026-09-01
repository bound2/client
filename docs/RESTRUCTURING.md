# Client Restructuring Plan

Living, trackable plan for moving the OpenDarkEden client's game code out of the
`DarkEden` executable and into testable static libraries, while the outstanding
findings of `docs/code-health-review-2026-08-29.md` keep getting fixed. The end
state: **a fix that touches library code ships with a unit test in the same
commit; code that cannot be tested is a shrinking, named exception rather than
the default.**

This mirrors the server repo's `docs/RESTRUCTURING.md` (same parent directory,
`../server`), which has already executed the exact refactoring this client
needs on its half of the shared codebase: `execute()` stripped off the packet
classes, a dispatch table at the composition root, the whole wire layer
compiled once into a standalone `de-kernel` library, membership held by a file
and enforced by an include-graph checker, and legacy debt tracked by
shrink-only ratchets. Where a task here has a proven server twin, the task
cites it — read the server's status notes before starting, because its
adversarial reviews recorded the traps.

## How to use this document

- Every task has a checkbox and a `> **Status:**` line. Update the status line
  **in the same commit** as the work it describes. Allowed values:
  `not started` | `in progress (<what remains>)` | `done (<commit>)` |
  `dropped (<why>)`.
- A task is only `done` when its **Owner** exists — the test or mechanism that
  keeps the rule true from then on. Landing the change without the owner is
  `in progress (owner missing)`.
- Ratchet numbers only go **down**. Re-measure with the given command before
  and after a change that claims progress; commit the updated number with the
  change.
- Substantial tasks get an **adversarial review** before merge (standing
  practice in this repo — the last one found three real defects in the fixes
  themselves).
- Verification for every extraction: both Debug trees build
  (`build/vs2022` with /RTC1, `build/vs2022-asan` with ASan), the test suite
  is green in a plain tree **and** an ASan tree, `wire_inventory_fresh`
  passes, and the user runtime-verifies the client against a live server
  before the branch merges (do not launch `DarkEden.exe` yourself).

### Working agreements for refactoring agents

- **New library targets use explicit source lists, never `file(GLOB)`.** The
  existing globs don't use `CONFIGURE_DEPENDS`, so a moved file silently stays
  in the old target until someone reconfigures; explicit lists make membership
  reviewable and are what the ratchet script parses. After moving files,
  reconfigure both trees.
- **Moving a file between targets must not change its bytes** in the same
  commit. Move first, prove the build, then fix — separate commits, so the
  diff that changes behavior is readable.
- Sources are CRLF; use the Edit tool, not `sed -i`/`awk`, to modify them.
- Wire layout is pinned: any change under `Client/Packet` that touches
  `read()`/`write()`/`getPacketMaxSize()` must keep
  `tests/wire-layout.txt` in sync (regenerate via
  `tests/tools/gen_wire_inventory.pl`, re-record with `UPDATE_GOLDENS=1`) and
  must be diffed against the server with
  `server/tests/tools/wire_inventory_diff.sh`. A layout change is a protocol
  change and needs the identical change in the server repo.

## Goals / non-goals

**Goals**

1. **Every new fix is unit-tested.** The code a fix touches gets moved into
   (or already lives in) a static library linked by `unit_tests`, and the fix
   is written test-first. Exceptions are named in the exemption list below,
   not decided ad hoc.
2. **The wire layer becomes a standalone library** (`packetwire`): streams,
   encrypter, info classes, and eventually all packet classes — so the #1
   open risk (unvalidated server input in `Client/Packet/Gpackets/`,
   code-health review priority 1) becomes directly testable with hostile
   inputs instead of only greppable.
3. **Game-model logic leaves the executable** piecewise (tables, inventories,
   price/trade logic), cutting `g_p*` global seams as it goes.
4. Architecture rules owned by tests (membership files, include-graph
   checker, ratchets), never by memory.

**Non-goals**

- No behavior change on the wire or in game rules unless a task says so
  explicitly. Layout stays pinned by the wire inventory + the server diff.
- No rewrite of the render/game loop (`GameMain.cpp`, `MZone` drawing,
  `GameUI.cpp`) or the VS_UI widget tree. These stay executable-or-UI-side
  and keep their live-server verification path.
- No new test framework. `tests/framework/test_framework.h` stays; tests are
  files in `tests/unit/` (globbed — reconfigure after adding one).
- Not a port of the server's kernel wholesale: the two repos stay separate
  codebases pinned to one wire contract by the shared inventory.

## Exemption list (tangled, verified at runtime instead)

Code on this list is fixed executable-side with live-server verification and
a **regression guard** note in the commit message, per existing practice.
Shrink it when a task extracts a seam, and record the removal here.

| Code | Why exempt |
|---|---|
| `GameMain.cpp`, `GameInit.cpp`, `Client.cpp`, `SDLMain.cpp` | process lifecycle, DLL whitelist, render loop |
| `MZone` rendering / `TileRenderer` draw paths | draws through live surfaces; viewer tools cover some of it |
| `VS_UI/src/**` widgets and dialogs | deep two-way coupling with game globals; UI verified visually |
| `Gpackets/*Handler.cpp` bodies | mutate `g_pZone`/creature state; the *parsers* they consume are in scope, the mutations are not (yet) |
| `PacketFunction.cpp`, `ClientCommunicationManager` connect paths | Winsock + connection state machine |

Everything else under `Client/*.cpp` and `Client/Packet/**` is presumed
movable until a task proves otherwise and adds it here with a reason.

## Ratchets (shrink-only)

Baselines measured 2026-09-01, commands run from the repo root (Git Bash).
Once task 0.2 lands, these live in `tests/ratchet/ratchets.sh` (run by ctest)
and the checked-in numbers fail the suite when a count **rises**, and fail on
an unrecorded drop, so tightening lands in the same commit as the progress.

| # | Metric | Baseline | Command |
|---|--------|---------:|---------|
| R1 | Translation units compiled directly into the DarkEden target | 993 (was 1,044 before task 1.1's `packetwire` move) | `grep -c "<ClCompile Include" build/vs2022/DarkEden.vcxproj` (machine-local tree; the ratchet script recomputes it from the CMake lists when the vcxproj is absent) |
| R2 | Packet `.cpp` files still defining `::execute(` (non-Handler) | 448 | `grep -rlE '::execute\s*\(' Client/Packet/Gpackets Client/Packet/Cpackets Client/Packet/Lpackets Client/Packet/Rpackets Client/Packet/Upackets --include='*.cpp' \| grep -v Handler \| wc -l` |
| R3 | `sprintf`/`strcpy`/`strcat` call sites under `Client/Packet` | 70 | `grep -rE '\b(sprintf\|strcpy\|strcat)\s*\(' Client/Packet --include='*.cpp' \| wc -l` |
| R4 | Files in static libraries referencing `g_p*` client globals | measure in 0.2 | `ratchets.sh` computes it over the union of the libraries' explicit source lists |

R1 is the headline number: it counts what still cannot be unit-tested. R2 is
the client twin of the server's R4 (which it drove to 0). R3 tracks
code-health priority 2 mechanically.

---

## Phase 0 — Scaffolding

- [x] **0.1 This document.**
  > **Status:** done (2026-09-01).
  - Owner: the status-line discipline itself; CLAUDE.md points here.

- [ ] **0.2 Ratchet script.** `tests/ratchet/ratchets.sh`, registered in
  `tests/CMakeLists.txt` as a ctest, ports the server's script shape: fails
  on increase and on unrecorded decrease; generates into scratch space, never
  overwrites tracked files in place. Measure and record the R4 baseline.
  Server twin: task 1.5 there, plus its "defects found by adversarial
  review" list (CRLF on committed test data, `UPDATE_GOLDENS` accepting any
  value) — apply those lessons on day one: `.gitattributes` `eol=lf` for any
  new committed test-data files.
  > **Status:** not started.
  - Owner: the ratchet test.

- [ ] **0.3 Include-graph checker.** `tests/arch/check_includes.pl` (perl —
  Git for Windows ships it, and the wire-inventory generator is already
  perl), run by ctest. Rules, extended per phase:
  - **W1**: a `packetwire` member (membership = the target's explicit source
    list) quote-includes only `packetwire` members, `basic/`, and system
    headers. No `Client/` game headers, no SDL, no VS_UI.
  - **W2**: no `MinTr.h` / `DebugInfo.h` / `DebugKit.h` includes in
    `packetwire` members (debug facilities whose definitions live in the
    exe/VS_UI; the commented-out references in the streams may not come
    back live via a lib file).
  Grandfathered violations go in a frozen shrink-only baseline file, exactly
  like the server's `tests/arch/baseline.txt`.
  > **Status:** not started.
  - Owner: the `arch_includes` ctest.

---

## Phase 1 — `packetwire`: the wire-support library (pilot)

The first extraction, deliberately chosen so that **no source file needs to
change to move** (bytes-identical relocation into a new target) and the
result immediately covers the top-risk area's foundations. This is the
"first candidate" — task 1.1 below names the exact membership.

- [ ] **1.1 Create the `packetwire` static library** from the game-free
  subset of the `Client/Packet` root (verified 2026-09-01: every file below
  includes only `Client_PCH.h`, packet-internal headers, and system headers;
  the debug-trace references in the streams are commented out).

  Membership (explicit list in `CMakeLists.txt`, ~57 files):
  - **Streams / framing / crypto:** `SocketInputStream.cpp`,
    `SocketOutputStream.cpp`, `SocketEncryptInputStream.cpp`,
    `SocketEncryptOutputStream.cpp`, `Encrypter.cpp`, `StringStream.cpp`,
    `Exception.cpp`, `PacketAssert.cpp`, `Datagram.cpp`
  - **Sockets:** `Socket.cpp`, `SocketImpl.cpp`, `ServerSocket.cpp`
    (`SocketAPI.cpp` and `DatagramSocket.cpp` include `DebugInfo.h` — they
    stay in the exe until their debug calls are sorted; W2 enforces this)
  - **Info classes (wire sub-structures, the parsers the GC packets
    delegate to):** `EffectInfo.cpp`, `ExtraInfo.cpp`, `ExtraSlotInfo.cpp`,
    `GameTime.cpp`, `GearInfo.cpp`, `GearSlotInfo.cpp`, `GuildWarInfo.cpp`,
    `InventoryInfo.cpp`, `InventorySlotInfo.cpp`, `LevelWarInfo.cpp`,
    `ModifyInfo.cpp`, `PCItemInfo.cpp`, `PCOustersInfo.cpp`,
    `PCOustersInfo2.cpp`, `PCOustersInfo3.cpp`, `PCSlayerInfo.cpp`,
    `PCSlayerInfo2.cpp`, `PCSlayerInfo3.cpp`, `PCVampireInfo.cpp`,
    `PCVampireInfo2.cpp`, `PCVampireInfo3.cpp`, `PetInfo.cpp`,
    `QuestStatusInfo.cpp`, `RaceWarInfo.cpp`, `RideMotorcycleInfo.cpp`,
    `RideMotorcycleSlotInfo.cpp`, `ScriptParameter.cpp`,
    `ServerGroupInfo.cpp`, `SlayerSkillInfo.cpp`, `SubItemInfo.cpp`,
    `SubOustersSkillInfo.cpp`, `SubServerInfo.cpp`,
    `SubSlayerSkillInfo.cpp`, `SubVampireSkillInfo.cpp`, `TextInfo.cpp`,
    `VampireSkillInfo.cpp`, `WarInfo.cpp`, `WorldInfo.cpp`
  - **Misc utility:** `Properties.cpp`

  Excluded, with reasons (sort when touched, never silently):
  - `ClientPlayer.cpp`, `ClientCommunicationManager.cpp`,
    `RequestClientPlayer*.cpp`, `RequestServerPlayer*.cpp`, `Player.cpp` —
    receive loops and connection state; include `MZone.h`/`MPlayer.h`/
    `UserInformation.h`/`ClientConfig.h`/`DebugInfo.h`. They become
    dispatch-only in Phase 2 and move then if their game includes go.
  - `PacketFactoryManager.cpp`, `PacketIDSet.cpp`, `PacketValidator.cpp` —
    reference every packet factory, which today drags every packet's
    `execute()` and therefore every handler; they move in 2.4.
  - `NPCInfo.cpp` (includes `MZone.h`/`MCreatureTable.h`),
    `OustersSkillInfo.cpp` (includes `ClientDef.h`), `SocketAPI.cpp`,
    `DatagramSocket.cpp`, `PacketFileAPI.cpp` — one vestigial include each
    away from eligibility; each is its own small follow-up commit.

  Build wiring: `packetwire` gets the same defines the exe gives these files
  today (`__GAME_CLIENT__` comes from `Client_PCH.h`; add `__WIN32__` and
  `__WINDOWS__` on WIN32 — `SocketAPI.cpp`-style `#if __WINDOWS__` branches
  exist in this family), include dirs `Client/Packet`, `Client`, `basic`.
  `DarkEden` and `VS_UI` link it; the files leave `CLIENT_MAIN_SOURCES` via
  `list(REMOVE_ITEM)`. Zero source-file edits in the move commit.
  > **Status:** in progress (owner missing — the explicit source list
  > exists, but the W1/W2 include rules (0.3) and the R1 ratchet test
  > (0.2) do not yet enforce it). The move itself landed: 51 files (the
  > membership above, minus a few counted twice in the "~57" estimate)
  > compile as `packetwire`, DarkEden dropped 1,044 → 993 TUs with zero
  > source edits and zero double-compiled members (verified against the
  > generated .vcxproj), both Debug trees build with 0 errors, and the
  > existing test suite is green. Runtime verification against a live
  > server pending (user-run).
  - Owner: the explicit source list + W1/W2 in the include checker + R1
    dropping by the member count.

- [ ] **1.2 Link `packetwire` into `unit_tests`** and land the first parser
  tests: hostile-input tests for `ModifyInfo::read` and
  `InventoryInfo::read` (oversized counts, zero/overlong string lengths —
  assert the rejected input / thrown `InvalidProtocolException`, then run
  the same suite under ASan per house rules), and a `SocketInputStream`
  framing test (a length that exceeds the buffer must not read past it).
  Note for stream construction in tests: the streams read from a real
  socket handle in production; tests exercise the buffer-manipulation API
  (`read(char*, len)` over a pre-filled ring) — if a seam is missing, add a
  test-only fill helper in the test, not a change to the stream.
  > **Status:** not started.
  - Owner: the tests themselves; `unit_tests` link line.

- [ ] **1.3 First test-first fix: `StringStream` stack overflow.**
  `StringStream.cpp` `operator<<(float)` does `sprintf(buf, "%f", T)` into
  `char buf[12]` — any float ≥ 10,000 writes 12+ chars plus NUL and smashes
  the stack; `operator<<(double)` has the same pattern into `buf[22]`
  (overflows from ~1e15). The server found and fixed this exact family on
  its side during the Exchange reconcile (server RESTRUCTURING 1.4 notes);
  the client copies are still unfixed. `long`/`ulong` with `buf[12]` are
  safe **on this build** only because MSVC `long` is 32-bit — widen them
  anyway when fixing, and switch the family to `snprintf`. Write the
  overflowing test first (plain tree: garbage/RTC; ASan tree: abort), then
  fix, then both trees green.
  > **Status:** not started.
  - Owner: the unit test.

**Phase exit criteria:** `packetwire` builds in both Debug trees; DarkEden
links and the user has runtime-verified a play session; `unit_tests` links
`packetwire` with new parser tests green under /RTC1 and ASan; R1 down by
the membership count; ratchet + include-checker enforce the boundary.

---

## Phase 2 — Strip `execute()`, move the packet classes

The client twin of the server's tasks 2.3/2.4, which are `done` there — the
migration recipe, the dispatcher design, and the traps are all recorded in
the server's status notes. This is what makes the ~509 `Gpackets` parsers
(priority 1 in the code-health review) directly unit-testable.

- [ ] **2.1 `PacketDispatcher` in `packetwire`.** Table of packet id →
  `void(*)(Packet*, Player*)`, written only at startup, consulted before the
  legacy virtual. Port the server's `PacketDispatcher.h` shape (including
  the `DE_REGISTER_PACKET_HANDLER` macros) rather than inventing a new one.
  The client's five receive loops all go dispatch-first:
  `ClientPlayer.cpp:293`, `Player.cpp:201`, `RequestClientPlayer.cpp:235`,
  `RequestServerPlayer.cpp:215`, `ClientCommunicationManager.cpp:212`
  (datagram, `NULL` player).
  > **Status:** not started.
  - Owner: R2 ratchet + a dispatcher unit test (unknown id → protocol
    exception).

- [ ] **2.2 Migrate the GC direction** (the risk direction: 251 handlers).
  Per batch: register the handler function at the composition root
  (`GamePacketDispatch.cpp` analog compiled into the exe), delete
  `execute()` from the packet class, leave the handler `.cpp` where it is.
  Wire layout untouched — `wire_inventory_fresh` and the server diff prove
  it per batch. R2 448 → tracks progress; do it in mergeable slices
  (~25 packets each), highest-risk parsers first (shop/stash/chat/guild —
  the fixed findings already name them).
  > **Status:** not started.
  - Owner: R2 ratchet.

- [ ] **2.3 Migrate CG / LC / CR-RC / U directions.** CG handlers
  (35 files) are already excluded from the build — on the client, CG
  `execute()` bodies are server-side vestiges; delete them with the same
  layout proof. The server deleted its no-op halves outright (its 2.4 step
  1) — mirror that: what the client never executes gets deleted, not
  migrated.
  > **Status:** not started.
  - Owner: R2 ratchet reaching ~0.

- [ ] **2.4 Move the packet classes into `packetwire`** (now that nothing in
  them references handlers), plus `PacketFactoryManager` /
  `PacketValidator` / `PacketIDSet`, and the held-back root files whose
  vestigial includes get removed on the way (`NPCInfo.cpp` etc.). Handlers
  move to a `Client/PacketHandler/` directory compiled into the exe, so the
  `Client/Packet` tree is wire-only — the include checker then locks the
  whole directory. `unit_tests` can now construct real packets through the
  real factories: add golden-fixture round-trips for the highest-risk GC
  packets (server task 1.2 has the recipe and the canonical-value rules —
  field values ≥ 128, distinct per field).
  > **Status:** not started.
  - Owner: W1 extended to the whole `Client/Packet` tree; the goldens.

- [ ] **2.5 Retire the wire-inventory workaround.** With packet `.cpp`s
  linkable, `test_wire_layout.cpp` can call the real factories instead of
  the perl-lifted `WireInventory.inc`. Keep `tests/wire-layout.txt` and its
  format byte-compatible — the server's `wire_inventory_diff.sh` reads it.
  > **Status:** not started.
  - Owner: `wire_inventory_fresh` staying green through the swap.

**Phase exit criteria:** R2 = 0; `Client/Packet` contains no handler code;
parser fixes under `Gpackets` are written test-first against real packet
objects; live-server smoke test passed (user-run).

---

## Phase 3 — The fix policy (in force from Phase 1 onward)

Not a code phase — the standing rule this plan exists to enable, stated once:

- [ ] **3.1 Every fix names its test path in the commit message.** One of:
  (a) *lib + test* — the code is in a static library and the fix commit
  contains the test; (b) *moved, then fixed* — the fix's first commit moves
  the unit into a library (Phase 1/2/4 machinery), the second fixes it
  test-first; (c) *exempt* — the code is on the exemption list, the commit
  says so and carries the regression-guard wording. A fix commit that is
  none of the three is wrong.
  > **Status:** not started (activates when 1.2 lands; record adoption here).
  - Owner: review practice + this document; graduate to a commit-lint hook
    if drift is observed.

---

## Phase 4 — Game-model extraction

Background work, one class family per branch, each independently mergeable.
Target library: `gamemodel` (new; links `basic` + `packetwire`, **no**
SDL/dxlib/VS_UI). The catch: ~45 `Client/*.cpp` model files currently
compile into the **VS_UI** target (`VS_UI_CLIENT_SOURCES`), which is
nominally a library but drags exe globals, so linking it into tests is not
viable — extraction means moving files *out of that list* into `gamemodel`
and cutting their `g_p*`/UI seams.

Order of attack (dependency-ranked; re-verify with an include scan when
starting each — the scan is one grep, and the ranking below is from a
2026-09-01 reading of the include graph, not a proof):

- [ ] **4.1 Pure tables first:** `ExperienceTable.cpp`,
  `MItemOptionTable.cpp`, `MGameStringTable.cpp`, `MSoundTable.cpp`,
  `SystemAvailabilities.cpp`, `FameInfo.cpp`. Data-in/data-out, load from
  files — the `TArray`-style loader tests already show the test pattern.
  > **Status:** not started.
- [ ] **4.2 Money/price/trade logic:** `MMoneyManager.cpp`,
  `MPriceManager.cpp`, `MTradeManager.cpp` (seams to `g_pShop`/UI to cut).
  > **Status:** not started.
- [ ] **4.3 Containers:** `MInventory.cpp`, `MStorage.cpp`,
  `MShopShelf.cpp`, `MQuickSlot.cpp` — the shop/stash index-bounds fixes
  from the review live here and deserve permanent tests.
  > **Status:** not started.
- [ ] **4.4 Item/skill cores:** `MItem.cpp`, `MItemManager.cpp`,
  `MSkillManager.cpp`, `SkillDef.cpp`, gear classes. Likely partial —
  whatever stays coupled goes on the exemption list explicitly.
  > **Status:** not started.
  - Owner (all of 4.x): `gamemodel`'s explicit source list, an M1 include
    rule (no SDL/UI/dxlib headers from `gamemodel`), R4 shrinking.

---

## Phase 5 — Long tail

- [ ] **5.1 Split the debug facilities.** A lib-safe logging header (the
  `DebugLog.h` system already exists — finish the migration for lib code)
  so `DebugInfo.h`/`MinTr.h` stop gating membership (unblocks
  `SocketAPI.cpp`, `DatagramSocket.cpp`, `NPCInfo.cpp`).
  > **Status:** not started.
- [ ] **5.2 Dead/duplicate source removal** (code-health priority 3): the
  `_bak` files are already excluded by the build — delete them; sort the
  `GameHelpers`/`GameFunctions`/`GamePacketFunctions` exclusion graveyard
  into deleted-or-documented. The wrong-file-edited trap dies here.
  > **Status:** not started.
- [ ] **5.3 TextSystem stub retirement.** Split `TextService.cpp`'s pure
  text utilities from its `g_pLast` drawing entry point so
  `tests/stubs/client_globals.cpp` can shrink (the stub file itself
  documents this as the real fix).
  > **Status:** not started.
- [ ] **5.4 Format-string audit** (code-health C19/C20/C22, ~600 sprintf
  sites fed by data files) — becomes tractable per-library as R3-style
  counts once the owning code is in libs.
  > **Status:** not started.

---

## First candidate (decided 2026-09-01)

**Task 1.1: the `packetwire` library.** Chosen over the alternatives
because:

1. **Zero-edit move.** Every member file's includes were scanned; none
   reaches game code, so the move commit is pure CMake — the safest
   possible first step for agents to execute and review.
2. **It sits under the #1 open risk.** The info classes *are* the parsers
   the GC packets delegate server-supplied data to; the streams are where
   every length check ultimately lands. Tests aimed there have the highest
   defect yield per hour — and the first known defect is already queued
   (task 1.3, the `StringStream` float/double stack overflow, confirmed
   present at `StringStream.cpp:192-222` and already proven real on the
   server's identical copy).
3. **It unblocks Phase 2.** The dispatcher, and eventually all 700+ packet
   classes, land in this target; creating it first means every later phase
   is "grow the membership list", not "invent a target".

Runner-up considered and deferred: extracting `ExperienceTable`-class pure
tables (now 4.1) — testable, but it neither touches the top risk nor
unblocks anything else.
