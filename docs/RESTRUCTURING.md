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
| `Client/PacketHandler/*Handler.cpp` bodies (moved out of `Gpackets/` etc. by task 2.4) | mutate `g_pZone`/creature state; the *parsers* they consume are in `packetwire` and testable, the mutations are not (yet) |
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
| R1 | Translation units compiled directly into the DarkEden target | **517** (518 before 4.2 moved `MMoneyManager.cpp`, another double-compiled VS_UI entry; 528 before task 4.1's `gamemodel` took its ten members out — the four support sources, and the six tables that the relative `VS_UI_CLIENT_SOURCES` list had never actually removed from the exe glob, so they compiled into both VS_UI and the executable — as the 36 files still on that list do; 529 before task 2.5 deleted the dead `CRRequest2Handler.cpp`; 992 before task 2.4 moved the 465 packet/table/info sources into `packetwire`, +1 for the split-out `GCExchangeBuyHandler.cpp`; 1,044 before task 1.1; the task-2.2 composition root `PacketHandlerRegistry.cpp` was a recorded +1, offset when finishing the migration deleted `CGHandlersStub.cpp`) | `grep -c "<ClCompile Include" build/vs2022/DarkEden.vcxproj` — `ratchets.sh` reads the generated vcxproj, preferring the ctest run's own build dir; on generators with no vcxproj it reports SKIP, not PASS |
| R2 | Packet `.cpp` files still defining a packet-style `::execute(Player` | **0** (448 → 432 in slice 1 → 0 when 2.2/2.3 finished; regex refined at 0 to stop matching comments and the in-file handler body in `GCExchangeBuy.cpp`) | `grep -rlE '^void\s+\w+::execute\s*\(\s*Player' Client/Packet/{Gpackets,Cpackets,Lpackets,Rpackets,Upackets} --include='*.cpp' \| grep -v Handler \| wc -l` |
| R3 | Live `sprintf`/`strcpy`/`strcat` lines under `Client/Packet` **and `Client/PacketHandler`** | 46 (unchanged by task 2.4, which widened the scope to follow the handlers out of `Client/Packet`; 61 at first measurement — the 2026-09-01 adversarial review showed a quarter of that was commented-out code, so the measurement now excludes `//` matches) | see `ratchets.sh` — the grep excludes comment-prefixed matches |
| R4 | Library-compiled `.cpp` files referencing `g_p*` client globals **they do not define themselves** | **35** (59 before task 4.0 — a reclassification, not seam-cutting: the 36 `VS_UI_CLIENT_SOURCES` files stopped being library-compiled, so the 24 of them that reach globals are executable debt now, counted by R1 and outside this ratchet; 61 before task 4.1 cut the two `g_pFileDef` seams in `MGameStringTable` and `SystemAvailabilities` and added the `gamemodel` membership file, whose four new members reference no game global; 81 before task 2.4 grew the membership from 52 to 518 files; the number fell because the measurement stopped counting a file's references to globals it defines — the packet tables own `g_pPacketFactoryManager`/`g_pPacketValidator` — and the two dead server-only bodies that reached game globals were deleted; 83 at first measurement, before two never-compiled files were filtered) | `ratchets.sh` computes it over the library dirs (minus CMake-excluded files) plus the `VS_UI_CLIENT_SOURCES` list parsed from `CMakeLists.txt` and the `packetwire` membership file |
| R5 | Direct packet `execute()` call sites outside `Client/Packet` (handlers under `Client/PacketHandler` are in scope) | 1 (a commented-out block in `CGameUpdate.cpp`; added 2026-09-01 after the review found live local-echo callers the receive-loop enumeration had missed; task 2.4 found two more inside handlers — `GCReconnectLoginHandler`/`LCReconnectHandler` fabricating a `CGConnectSetKey` — invisible while handlers lived under the excluded `Client/Packet`, caught by the compiler once `Packet::execute` was deleted, and routed through the dispatcher; a live caller is now a compile error before it is a ratchet failure) | see `ratchets.sh` |

R1 is the headline number: it counts what still cannot be unit-tested. R2 is
the client twin of the server's R4 (which it drove to 0). R3 tracks
code-health priority 2 mechanically.

---

## Phase 0 — Scaffolding

- [x] **0.1 This document.**
  > **Status:** done (2026-09-01).
  - Owner: the status-line discipline itself; CLAUDE.md points here.

- [x] **0.2 Ratchet script.** `tests/ratchet/ratchets.sh`, registered in
  `tests/CMakeLists.txt` as a ctest, ports the server's script shape: fails
  on increase and on unrecorded decrease; generates into scratch space, never
  overwrites tracked files in place. Measure and record the R4 baseline.
  Server twin: task 1.5 there, plus its "defects found by adversarial
  review" list (CRLF on committed test data, `UPDATE_GOLDENS` accepting any
  value) — apply those lessons on day one: `.gitattributes` `eol=lf` for any
  new committed test-data files.
  > **Status:** done (2026-09-01, `restructuring/enforcement`) — the
  > `ratchets` ctest runs R1–R4 with baselines inline in the script;
  > measurements are grep-only (nothing generated, nothing overwritten).
  > R1 reads the generated `DarkEden.vcxproj`, preferring the ctest
  > run's own build dir so the number always reflects the tree just
  > configured, and SKIPs (never passes) where no vcxproj exists. R4
  > measured at 83. `.gitattributes` got `eol=lf` for the new committed
  > test data and scripts.
  - Owner: the ratchet test.

- [x] **0.3 Include-graph checker.** `tests/arch/check_includes.pl` (perl —
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
  > **Status:** done (2026-09-01, `restructuring/enforcement`) — the
  > `arch_includes` ctest walks the full quote-include closure of the
  > membership parsed from `CMakeLists.txt` (133 files) with the
  > compiler's own resolution order; unresolvable includes are
  > violations too, and an unmatched baseline entry fails the run
  > (shrink-only). Getting to a clean walk took three source changes,
  > landed with it: the two vestigial debug includes came out of the
  > streams (`MinTr.h` in `SocketInputStream.cpp`, `DebugInfo.h` in
  > `SocketOutputStream.cpp` — both referenced only from comments), and
  > W1's one real finding, `Types/ItemTypes.h` → `Client/RaceType.h`,
  > was fixed the way the server fixed `SkillTypes`: `RaceType.h` is
  > pure wire vocabulary (the `Race` enum, `Race_t`, `szRace`) and
  > moved to `Client/Packet/RaceType.h`; every includer resolves it via
  > the `Client/Packet` include dir, which `effect_viewer` — the one
  > target compiling game sources without it — now also carries.
  > `tests/arch/baseline.txt` is empty by design.
  - Owner: the `arch_includes` ctest.

---

## Phase 1 — `packetwire`: the wire-support library (pilot)

The first extraction, deliberately chosen so that **no source file needs to
change to move** (bytes-identical relocation into a new target) and the
result immediately covers the top-risk area's foundations. This is the
"first candidate" — task 1.1 below names the exact membership.

- [x] **1.1 Create the `packetwire` static library** from the game-free
  subset of the `Client/Packet` root (verified 2026-09-01: every file below
  includes only `Client_PCH.h`, packet-internal headers, and system headers;
  the debug-trace references in the streams are commented out).

  Membership (explicit list in `CMakeLists.txt`, ~57 files):
  - **Streams / framing / crypto:** `SocketInputStream.cpp`,
    `SocketOutputStream.cpp`, `SocketEncryptInputStream.cpp`,
    `SocketEncryptOutputStream.cpp`, `Encrypter.cpp`, `StringStream.cpp`,
    `Exception.cpp`, `PacketAssert.cpp`, `Datagram.cpp`
  - **Sockets:** `Socket.cpp`, `SocketImpl.cpp`, `ServerSocket.cpp`,
    `SocketAPI.cpp` (its `DebugInfo.h` include turned out to be commented
    out — it joined in 1.2 because `~SocketImpl` calls
    `SocketAPI::closesocket_ex`, so test binaries constructing a `Socket`
    need it to link; `DatagramSocket.cpp` calls `DEBUG_ADD` for real and
    stays out until 5.1)
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
  - `NPCInfo.cpp` (includes `MZone.h`/`MCreatureTable.h`) and
    `OustersSkillInfo.cpp` (includes `ClientDef.h`) — one vestigial
    include each away from eligibility; `DatagramSocket.cpp` (live
    `DEBUG_ADD` calls, task 5.1); `PacketFileAPI.cpp` (include-clean but
    unneeded until the Rpackets file-transfer work).

  Build wiring: `packetwire` gets the same defines the exe gives these files
  today (`__GAME_CLIENT__` comes from `Client_PCH.h`; add `__WIN32__` and
  `__WINDOWS__` on WIN32 — `SocketAPI.cpp`-style `#if __WINDOWS__` branches
  exist in this family), include dirs `Client/Packet`, `Client`, `basic`.
  `DarkEden` and `VS_UI` link it; the files leave `CLIENT_MAIN_SOURCES` via
  `list(REMOVE_ITEM)`. Zero source-file edits in the move commit.
  > **Status:** done (2026-09-01 — the owners landed with 0.2/0.3: the
  > R1 ratchet and the W1/W2 include rules now enforce the boundary;
  > runtime-verified against a live server and merged in PR #34).
  > History: the move landed first with the owners missing: 51 files (the
  > membership above, minus a few counted twice in the "~57" estimate)
  > compile as `packetwire`, DarkEden dropped 1,044 → 993 TUs with zero
  > source edits and zero double-compiled members (verified against the
  > generated .vcxproj), both Debug trees build with 0 errors, and the
  > existing test suite stayed green.
  - Owner: the explicit source list + W1/W2 in the include checker + R1
    dropping by the member count.

- [x] **1.2 Link `packetwire` into `unit_tests`** and land the first parser
  tests: hostile-input tests for `ModifyInfo::read` and
  `InventoryInfo::read` (oversized counts, zero/overlong string lengths —
  assert the rejected input / thrown `InvalidProtocolException`, then run
  the same suite under ASan per house rules), and a `SocketInputStream`
  framing test (a length that exceeds the buffer must not read past it).
  Note for stream construction in tests: the streams read from a real
  socket handle in production and `fill()` is the only production writer
  of the ring, so the seam is a one-line `friend class
  SocketInputStreamTestAccess;` in the stream header — access-only, no
  layout or behavior change, declared unconditionally so the class
  definition is identical in every TU. The helper itself lives in the
  test file. Test TUs must compile with the library's own defines
  (`__GAME_CLIENT__=1`, Windows wire macros): `Packet`'s virtual set
  changes under them, so a mismatch is a real vtable/ODR break.
  > **Status:** done (2026-09-01, `restructuring/packetwire`) —
  > `unit_tests` links `packetwire`; 7 parser/stream tests landed:
  > stream bounds (zero-length rejected, over-read throws
  > `InsufficientDataException` and consumes nothing, wrap-around
  > reassembly), `ModifyInfo` (happy-path parse, hostile count hits the
  > underflow guard), `InventoryInfo` (zero count, hostile count).
  > `SocketAPI.cpp` joined the library (see 1.1 membership note). Suite
  > green in the plain and ASan test trees.
  > **Review round (2026-09-01, PR #34):** the review found the task's
  > promised string-length coverage missing and a leak on the very
  > guard path the hostile-count tests pin. Both fixed on the branch:
  > three `read(std::string&, len)` tests added (zero length rejected,
  > over-read throws and consumes nothing, and the
  > truncate-at-embedded-NUL-but-consume-full-length asymmetry that a
  > parser framing on `str.size()` would desync on), and the five
  > allocate → read → push_back parser sites (`InventoryInfo`,
  > `GearInfo`, `ExtraInfo`, `RideMotorcycleInfo`, `PCItemInfo`) now
  > push_back before read so the destructor frees the in-flight slot
  > when a truncated payload makes the nested read throw. The leak fix
  > is a **regression guard** — MSVC's Windows ASan does no leak
  > detection, so no test observes it.
  - Owner: the tests themselves; `unit_tests` link line.

- [x] **1.3 First test-first fix: `StringStream` stack overflow.**
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
  > **Status:** done (2026-09-01, `restructuring/packetwire`) — the
  > tests ran first and proved the defect exactly as predicted: /RTC1
  > reported "Stack around the variable 'buf' was corrupted" on the
  > float ≥ 10,000 and double ≥ 1e15 cases, and `%f` of 1e300 (308
  > characters into `char[22]`) killed the process with a stack-buffer
  > overrun fast-fail (0xc0000409). Fix: the whole nine-operator numeric
  > family switched to `snprintf` with range-sized buffers (`short` 8,
  > 32/64-bit integers 24, `float` 64, `double` 352 — `%f` of `-DBL_MAX`
  > is 317 characters). 7 tests pin the boundaries, including exact
  > formatting at each old overflow threshold and `INT_MIN`/
  > `ULLONG_MAX`. Suite green in the plain tree AND the ASan tree; both
  > full client trees rebuild clean.
  > **Review round (2026-09-01, PR #34):** with 300+ character entries
  > now real instead of a crash, the review caught that `m_Size` was
  > `ushort` and wrapped at 64 KiB — `isEmpty()` reported an empty
  > stream over real content and `toString()`'s `reserve()`
  > under-reserved. Fixed test-first (`size_t` now): the new test's 64 ×
  > 1024-character insertions ran red (`isEmpty()` true at exactly
  > 65,536) before the one-line header fix, green after. Also clears
  > the file's C4267 warnings.
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

- [x] **2.1 `PacketDispatcher` in `packetwire`.** Table of packet id →
  `void(*)(Packet*, Player*)`, written only at startup, consulted before the
  legacy virtual. Port the server's `PacketDispatcher.h` shape (including
  the `DE_REGISTER_PACKET_HANDLER` macros) rather than inventing a new one.
  The client's five receive loops all go dispatch-first:
  `ClientPlayer.cpp:293`, `Player.cpp:201`, `RequestClientPlayer.cpp:235`,
  `RequestServerPlayer.cpp:215`, `ClientCommunicationManager.cpp:212`
  (datagram, `NULL` player).
  > **Status:** done (2026-09-01, `restructuring/packet-dispatcher`) —
  > `Client/Packet/PacketDispatcher.{h,cpp}` is the server's design
  > verbatim (fixed `PACKET_MAX` table, startup-only writes, assert on
  > double registration, `InvalidProtocolException` from `dispatch` on
  > an unregistered id, both `DE_REGISTER_PACKET_HANDLER` macros) plus
  > one client-only transitional entry: `tryDispatch`, which reports an
  > unregistered id instead of throwing so the five receive loops —
  > all converted, `DatagramPacket` is a `Packet` so the datagram loop
  > uses the same table — fall back to the legacy virtual until R2
  > reaches 0, at which point the loops switch to `dispatch()` and
  > `tryDispatch` is deleted. Five unit tests pin the contract
  > (registered handler runs with the given packet/player, tryDispatch
  > true/false split, dispatch throws on unregistered, double
  > registration refused via `AssertionError`). With no handlers
  > registered yet the change is behaviorally inert — every packet
  > still takes the fallback — so the runtime risk of this landing
  > alone is nil; each 2.2 slice carries its own live verification.
  > Suite 119/450/0 in plain and ASan trees; both client trees 0 errors.
  - Owner: R2 ratchet + a dispatcher unit test (unknown id → protocol
    exception).

- [x] **2.2 Migrate the GC direction** (the risk direction: 251 handlers).
  Per batch: register the handler function at the composition root
  (`GamePacketDispatch.cpp` analog compiled into the exe), delete
  `execute()` from the packet class, leave the handler `.cpp` where it is.
  Wire layout untouched — `wire_inventory_fresh` and the server diff prove
  it per batch. R2 448 → tracks progress; do it in mergeable slices
  (~25 packets each), highest-risk parsers first (shop/stash/chat/guild —
  the fixed findings already name them).
  > **Status:** in progress (slice 1 landed 2026-09-01,
  > `restructuring/gc-slice-1`; 235 GC packets remain). Slice 1 = the 16
  > shop/stash/say/guild packets (`GCSay`, `GCGuildChat`,
  > `GCGuildMemberList`, `GCGuildResponse`, the 10 `GCShop*`,
  > `GCStashList`, `GCStashSell`). `Packet::execute` is no longer pure:
  > the default throws `InvalidProtocolException`, so a migrated id
  > received with no registration disconnects instead of silently
  > no-opping — a mis-migration surfaces in the first live run.
  > `Client/PacketHandlerRegistry.cpp` is the composition root, called
  > from `InitGame()` beside the `PacketFactoryManager` creation,
  > idempotent against a second init pass; the two stash packets keep
  > their `__BEGIN_DEBUG` wrapper via explicit thunks (no-op on
  > `__WIN32__`, preserved for the cout-branch platforms — the server's
  > `CGStashList` precedent). Every deleted `execute()` was verified
  > pure delegation before deletion. R2 448 → 432; live verification of
  > chat/shop/stash/guild flows gates the merge.
  > **Completed 2026-09-01** (`restructuring/packet-migration-2`, with
  > 2.3 below): every `execute()` body in every packet direction was
  > mechanically classified (pure delegation / delegation-with-debug /
  > packet-only-handler / empty / fully-preprocessor-guarded / other)
  > and all 432 remaining were stripped. The GC/LC/CR-RC/U directions
  > are registered in `PacketHandlerRegistry.cpp`: 271 standard
  > delegations (including `GCUseSkillCardOK`, a second packet class
  > hiding in `GCUseOK.cpp` that filename-based tooling missed), 7
  > packet-only handlers (the datagram connection family), 6
  > `__BEGIN_DEBUG` thunks, `GLIncomingConnectionError`'s cout trace
  > preserved verbatim, and `GCExchangeList` as an explicit no-op (its
  > execute was empty on purpose — the exchange UI consumes the parsed
  > packet elsewhere). R2 = 0, held by the ratchet with a refined regex.
  > **Adversarial review round (2026-09-01, 8 Opus reviewers — 2 per
  > stack branch, behavior + claims lenses; 3 of 8 NO-SHIP):** the
  > review earned its keep. Fixed on the branch, most severe first:
  > 1. **The registry registered all 271 standard handlers twice** (a
  >    script assembly fault duplicated the block verbatim) — the
  >    client would have thrown out of startup on the first run.
  > 2. **The receive loops were never the whole story**: the client
  >    fabricates packets locally and calls `execute()` on them
  >    directly — 12 live sites (skill echoes in `CGameUpdate.cpp`, GM
  >    system messages in `Client.cpp`, and `CGConnectSetKey` on the
  >    login paths in `UIMessageManager.cpp`). All would have hit the
  >    new throwing default; login and skill feedback were broken. All
  >    12 now route through `PacketDispatcher::dispatch`,
  >    `CGConnectSetKey` is registered as the explicit no-op its
  >    linked stub always was, and new ratchet **R5** owns the rule.
  > 3. **`CRRequest2` collided with `CRRequest`** — both claim
  >    `PACKET_CR_REQUEST`; the orphan duplicate (no factory, no
  >    callers) is no longer registered and is flagged for 5.2.
  > 4. The claimed CG-deletion safety rationale was **false as
  >    stated**: `PacketValidator`'s `CPS_NORMAL` accepts any id and
  >    `Player::processCommand` has no validator. The honest basis
  >    (documented in the registry header now): the server never sends
  >    CG/CL ids and the deleted bodies were no-ops, so the only
  >    change is that a protocol-violating peer now disconnects — the
  >    same trade the server repo made. Also: 16 of the 163 deleted
  >    "CG" files are actually CL packets.
  > 5. `registerHandler`'s guards were `Assert` (gone under NDEBUG) —
  >    now unconditional throws; the registry's idempotency flag is
  >    set after success, not before; the `DE_REGISTER` thunks carry
  >    `__BEGIN_TRY`/`__END_CATCH` so the per-packet stack-annotation
  >    frame the deleted `execute()`s had is preserved; the
  >    composition root's caller is correctly documented as
  >    `InitSocket()` (per login attempt), not `InitGame()`.
  > 6. Ratchet hygiene: R2 asserts its directories exist (a rename
  >    would have fail-opened at baseline 0), R3 stops counting
  >    commented-out code (61 → 46), R4 stops counting two files CMake
  >    never compiles (83 → 81), and the dispatcher tests gained the
  >    coverage the review showed missing (non-null player
  >    pass-through, out-of-range everywhere, the base-default throw).
  - Owner: R2 ratchet.

- [x] **2.3 Migrate CG / LC / CR-RC / U directions.** CG handlers
  (35 files) are already excluded from the build — on the client, CG
  `execute()` bodies are server-side vestiges; delete them with the same
  layout proof. The server deleted its no-op halves outright (its 2.4 step
  1) — mirror that: what the client never executes gets deleted, not
  migrated.
  > **Status:** done (2026-09-01, `restructuring/packet-migration-2`,
  > landed together with the 2.2 completion) — LC (17 + 1 packet-only)
  > and CR/RC (12) and U (2) are registered like GC; all 163 CG
  > `execute()` bodies are deleted unregistered, which is
  > behavior-preserving because `PacketValidator` rejects CG ids on
  > every receive path before dispatch, and the bodies were empty on
  > the client anyway — 138 fully `#ifndef __GAME_CLIENT__`-guarded, 5
  > empty, 6 guarded-with-debug-wrapper, and 14 whose unguarded
  > delegation went to the no-op stubs in `CGHandlersStub.cpp`, which
  > is now **deleted** (that stub file existed only to satisfy those
  > calls). `DatagramPacket` lost its pure `execute` redeclaration —
  > every subclass migrated, so the `Packet` base's throwing default is
  > correct. The receive loops keep the `tryDispatch` fallback for now:
  > with R2 = 0 the fallback reaches only the base default, which
  > throws exactly like `dispatch()` — flipping the loops and deleting
  > `tryDispatch` is a cosmetic follow-up once live verification passes.
  > **Follow-up done with 2.4** (PR #39 was the live verification): the
  > five receive loops call `PacketDispatcher::dispatch` unconditionally,
  > `tryDispatch` is deleted, and so is `Packet::execute` itself — the
  > base class carries no handler entry point at all, as on the server.
  > R2 stays as the guard against an override coming back.
  - Owner: R2 ratchet reaching ~0.

- [x] **2.4 Move the packet classes into `packetwire`** (now that nothing in
  them references handlers), plus `PacketFactoryManager` /
  `PacketValidator` / `PacketIDSet`, and the held-back root files whose
  vestigial includes get removed on the way (`NPCInfo.cpp` etc.). Handlers
  move to a `Client/PacketHandler/` directory compiled into the exe, so the
  `Client/Packet` tree is wire-only — the include checker then locks the
  whole directory. `unit_tests` can now construct real packets through the
  real factories: add golden-fixture round-trips for the highest-risk GC
  packets (server task 1.2 has the recipe and the canonical-value rules —
  field values ≥ 128, distinct per field).
  > **Status:** done (2026-09-01,
  > `restructuring/packet-classes-to-packetwire`; live verification of a
  > play session gates the merge). `Client/Packet` is wire-only and
  > compiled once: `packetwire` = 518 files, membership held by
  > `tests/arch/packetwire_files.txt` (read by CMake, the include checker
  > and the ratchet script — one file, three readers, as the server's
  > `kernel_files.txt`), with the eight receive-loop sources named in
  > `packetwire_holdouts.txt`. The 284 live handlers moved to
  > `Client/PacketHandler/` by pure `git mv` after one prep commit
  > qualified their includes (`"Gpackets/GCSay.h"`); the 35 never-compiled
  > CG handlers were deleted (the server's step 1); `GCExchangeBuy.cpp`'s
  > in-file handler was split out. The last game reaches came out
  > test-first or by seam: `CLLogin`'s `g_pUserInformation` read became a
  > packet member the sender sets, `WHISPER_MESSAGE` moved beside
  > `CRWhisper`, five root info classes (`NicknameInfo`, `StoreInfo`,
  > `GuildInfo`, `GuildMemberInfo`, `BloodBibleSignInfo`) moved under
  > `Client/Packet`, seven vestigial includes went, and two dead
  > `__GAME_SERVER__` bodies that built wire fields from live game
  > objects were deleted (`GCSelectQuestID(PlayerCreature*)`,
  > `GCStashList::setStashItem`). The include checker gained **W0**
  > (every `.cpp` under `Client/Packet` is in exactly one of the two
  > files) and evaluates `#if`/`#ifdef` on the one-meaning macros
  > (`__GAME_CLIENT__` defined, server macros never) so the 58 server
  > headers behind dead branches are skipped as the compiler skips them;
  > 1,068 files walk clean. R1 992 → 529.
  > **The first standalone link found the last seam**: `Datagram::read`
  > called `SendBugReport` (exe-defined, declared ad hoc so no include
  > rule saw it). `PacketDiagnostics` is the hook — the library formats,
  > the exe installs `SendBugReport` at the composition root.
  > **Goldens (the owner):** `test_packet_goldens.cpp` writes through the
  > real streams (a `SocketOutputStreamTestAccess` friend, the twin of
  > 1.2's input seam) and pins 60 `.hex` files, **54 byte-identical copies
  > of the server's** with its fixture values (all 19 encrypter packets
  > at codes 0–5, GCMoveOK framed, CGSay, CGWhisper; the 15 beyond the
  > first cut were added in the review round) —
  > `diff -r` of the two golden directories is the cross-repo check — plus
  > six client-authored (GCSay, GCGuildChat ×2, GCSystemMessage, CLLogin
  > ×2). 38 of 39 matched on the first run; **the 39th was a real wire
  > defect**: `CGMove` at encrypt code 0 wrote x,y,dir where the server
  > reads dir,x,y (codes 1–5 agreed), and code 0 is reachable — the
  > session code `((zone>>8)^zone)^((server+1)<<4)` cancels for some
  > zones. Fixed test-first on the client (the reading side is the
  > authority); the server needs no change. `test_packet_factories.cpp`
  > is the link proof (one `PacketFactoryManager::init()` pulls every
  > packet object) and pins that the manager refuses CG and out-of-range
  > ids. Two asymmetries stated as fact-tests, server-style: this repo's
  > `CLLogin::read` is one byte short of the login server's (the client
  > never reads one), and `GCDropItemToZone` round-trips here while the
  > server pins only its `write()`. Suite: 164 tests (1,718 checks) green in the plain
  > and ASan trees; all four ctests green.
  > **Adversarial review round (2026-09-02, 8 Opus reviewers by angle;
  > 1 NO-SHIP, on the tooling):** fixed on the branch —
  > 1. `check_includes.pl` ignored `#include <...>`, and the library's
  >    include path carries `Client/`, so `#include <MZone.h>` would have
  >    compiled unseen: angle includes that resolve inside the tree are
  >    now checked like quoted ones. Its search order was also the
  >    inverse of the compiler's (a `Client/` shadow of a `Client/Packet`
  >    header would have passed the checker and bound in the build);
  >    it now mirrors `target_include_directories(packetwire)`. W0 keys
  >    are case-folded (Windows), indented membership lines are refused
  >    (CMake anchored at column 1 and would have dropped them), CMake
  >    strips before matching, and both floors are 400.
  > 2. Ratchets: R3 excluded any line whose `//` tail mentioned the
  >    keyword, so a live `sprintf` with a trailing comment slipped —
  >    the tail is stripped first now; R1 fails on a `.vcxproj` older
  >    than the lists it was generated from instead of measuring a
  >    stale tree; `check()` fails on an unmeasurable value instead of
  >    passing it.
  > 3. Tests: a recording run (`UPDATE_GOLDENS=1`) is no longer green;
  >    `EncrypterFree` checks `read()` as well as `write()`;
  >    `CheckItemBaseEqual` no longer pops an empty list (UB inside the
  >    expected failure); the framework catches a throwing test body
  >    instead of losing the rest of the suite; the remaining 15
  >    encrypter packets are pinned with the server's goldens (54 of 60
  >    files now byte-identical copies); the chat parsers' length bounds
  >    have hostile-input tests; comment claims trimmed to what the code
  >    proves (the factory link test covers the received directions
  >    only; shared fixtures keep the server's sub-128 values).
  > 4. Facts corrected: the reachable code-0 zone in the shipped data is
  >    1301 (the fix commit's "zone 16" is arithmetic, not a real zone);
  >    the four RC datagram handlers a reviewer reported unregistered
  >    are registered through the `_NOPLAYER` macro form.
  > 5. Review-surfaced follow-ups, not done here: the server's
  >    `docs/RESTRUCTURING.md` 1.2 note still says the client has no
  >    goldens and its `FIXES.md` has no entry for the CGMove finding
  >    (server repo); a normalised read/write sweep across all 500
  >    shared `.cpp` files flagged 8 GC/LC packets whose field sequence
  >    may differ between repos (`GCAddItemToInventory`,
  >    `GCAddItemToItemVerify`, `GCShopBought`, `GCShopBuyOK`,
  >    `GCUpdateInfo`, `GCGQuestInventory`, `GCPartySay`,
  >    `BloodBibleBonusInfo`) — triaged under 2.5: one real desync
  >    (`GCAddItemToItemVerify`), seven false positives.
  > **Deliberately not done here:** the 186 remaining
  > `__GAME_SERVER__`/`__GAME_CLIENT__` conditionals in 157 packet
  > sources (the server's K2 rule bans them; here they have one meaning
  > in every target, so the checker evaluates them instead — sweep the
  > dead server halves under 5.2), and goldens for the ~500 unpinned
  > packets (2.5 builds on the real factories now).
  - Owner: W0/W1 over the whole `Client/Packet` tree (membership file);
    the goldens; the factory link test.

- [x] **2.5 Retire the wire-inventory workaround.** With packet `.cpp`s
  linkable, `test_wire_layout.cpp` can call the real factories instead of
  the perl-lifted `WireInventory.inc`. Keep `tests/wire-layout.txt` and its
  format byte-compatible — the server's `wire_inventory_diff.sh` reads it.
  > **Status:** done (2026-09-02,
  > `restructuring/wire-inventory-real-factories`). `test_wire_layout.cpp`
  > now constructs every factory under `Client/Packet` through
  > `tests/generated/WireInventory.inc`, which took the server's registry
  > shape (one include and one registration per factory class;
  > `gen_wire_inventory.pl` emits it, `wire_inventory_fresh` still pins
  > it) instead of perl-lifted method bodies. `tests/wire-layout.txt` is
  > byte-identical before and after. Two client-only divergences had to
  > go first: **112 factory classes were compiled out of every client
  > build** — wrapped in `#ifdef __DEBUG_OUTPUT__` (108) or `#ifndef
  > __GAME_CLIENT__` (4), where the server compiles all of its
  > unconditionally — so the guards came out (224 deleted lines, nothing
  > else touched, CRLF intact); and `CRRequest2`, the dead duplicate that
  > claimed `PACKET_CR_REQUEST` beside `CRRequest`, was deleted with its
  > handler and membership line (the registry comment had deferred it to
  > 5.2; the uniqueness test it broke made it due now; R1 529 → 528).
  > Rpackets are registered but flagged out of the rendered file (the
  > server deleted its copies; the file stays diffable line for line).
  > The test gained the server's "every factory creates a packet with its
  > own id" — the link proof for the written CG/CL directions, which
  > `test_packet_factories.cpp`'s manager never constructs — and a manager
  > check: every id `PacketFactoryManager::init()` serves is a listed
  > factory at the factory's own max size (the server pins the same
  > relation from its ratchet script).
  > **The first all-factory construct/destroy found a real defect:**
  > `GCUpdateInfo`'s constructor never initialised `m_pBloodBibleSign`
  > while the client-side destructor deletes it, so a packet destroyed
  > before a complete `read()` — a truncated body or a mid-parse
  > exception, exactly the receive loop's failure path — freed a garbage
  > pointer. Fixed test-first in `test_packetwire_parsers.cpp` (a fresh
  > packet holds no sub-objects; a packet whose read threw is safe to
  > destroy): the contract check fails and the process then dies without
  > the one-token fix; green plain and ASan with it. The server's copy has
  > the same constructor but never deletes the member, so it needs no
  > change. Suite: 168 tests (3,199 checks) green in both test trees.
  > **Triage of the 8 packets task 2.4's sweep flagged** (field sequence
  > possibly differing between the repos), `read()`/`write()` bodies
  > diffed against the server's: six are false positives with identical
  > layouts (`GCAddItemToInventory`, `GCShopBought`, `GCShopBuyOK`,
  > `GCGQuestInventory`, `BloodBibleBonusInfo`, and `GCPartySay`, where
  > the server merely reuses its name-length local for the message
  > length and this repo adds the bounds checks); `GCUpdateInfo`'s
  > `read()` is identical and only its never-used client `write()` lacks
  > the trailing power-point field; **`GCAddItemToItemVerify` was a real
  > desync**: the server writes ONE parameter (the new grade) for
  > `UP_GRADE_OK` — `CGAddItemToItemHandler` calls only
  > `setParameter(pItem->getGrade())`, and the server's `write()` and
  > `getPacketSize()` agree — while this repo's `read()` and
  > `getPacketSize()` took two, so every successful item-grade upgrade
  > over-read four bytes into the next packet. This repo's `write()` also
  > dropped both parameters of `THREE_ENCHANT_OK`. Fixed test-first with
  > three client-authored goldens built from the server's `write()`
  > semantics (`GCAddItemToItemVerify.{upgrade,threeenchant,fail}`),
  > read-exactly and round-trip checks, and a fresh-packet check
  > (`m_Parameter2` was uninitialised, and the handler reads it). The
  > handler's `UP_GRADE_OK` case, which set the item TYPE from the
  > phantom second parameter, now sets the item's grade from the one
  > the server sends — executable code, runtime-verified (exempt path).
  > The client-only `ADD_ITEM_TO_ITEM_REMOVE_OPTION_OK` code (one past
  > the server's enum, where the server's `ADD_ITEM_TO_ITEM_VERIFY_MAX`
  > sits) is left as it was; the server never sends it.
  > **Adversarial review round (2026-09-02, 3 reviewers by angle, all
  > SHIP with findings), fixed on the branch:** the un-guard had left a
  > doubled blank line wherever one of 59 `#endif`s sat between blanks —
  > swept, and the count of blank runs is back to the pre-strip figure;
  > two of the four `__GAME_CLIENT__` guards (`CGFailQuest`,
  > `CGAddItemToCodeSheet`) had also wrapped the server-side handler
  > declaration, now exposed as 41 other CG headers already expose theirs
  > (harmless; the first commit's "factory classes" wording
  > under-described it); the orphaned `Client/Client.vcxproj.filters` and
  > `.user` (no tracked `.vcxproj`, hundreds of dead entries, three of
  > them `CRRequest2`) are deleted; the freshness gate's failure message
  > still spoke of ids and max sizes the registry no longer carries —
  > reworded; the generator's `getPacketName()` check now refuses a
  > spelling it cannot check instead of skipping it. Suite: 172 tests
  > (3,232 checks) green in both test trees; both executable trees 0
  > errors; live verification of an item upgrade (and a play session)
  > gates the merge.
  - Owner: `wire_inventory_fresh` staying green through the swap; the
    all-factory construction in `test_wire_layout.cpp`; the
    `GCAddItemToItemVerify` goldens.

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
  > **Status:** adopted (recorded 2026-09-02 with task 4.1). The rule
  > has been followed in substance since 1.2 — every fix was written
  > test-first in a library or called out as executable-side — but not
  > in wording: of the nine `fix:` commits since 1.2, `GCUpdateInfo`,
  > `GCAddItemToItemVerify` and the item-option bound say "Test path:
  > lib + test" / "exempt" outright, the two `StringStream` fixes and
  > the slot-info leak say "test-first against packetwire" instead, and
  > the two review-round repair commits (`9b69cff`, `e5db5f9`) name no
  > path at all while mixing library and handler edits. From here on the
  > three words are mandatory in every fix commit, review-round repairs
  > included, and the 4.1 review round is the miss that graduates this
  > to a commit-lint hook: add it with the next fix.
  - Owner: review practice + this document; graduate to a commit-lint hook
    if drift is observed.

---

## Phase 4 — Game-model extraction

Background work, one class family per branch, each independently mergeable.
Target library: `gamemodel` (new; links `basic` + `packetwire`, **no**
SDL/dxlib/VS_UI). The catch, as first written: ~45 `Client/*.cpp` model
files sat in the **VS_UI** target's list (`VS_UI_CLIENT_SOURCES`), which is
nominally a library but drags exe globals, so linking it into tests is not
viable. Task 4.1 found they compiled into the executable too (the list was
relative, the exe's glob absolute, so the `REMOVE_ITEM` between them never
matched — every file on it was built twice and the exe's object won the
link, the LNK4217 noise CLAUDE.md used to describe), and 4.0 resolved it
the safe way round: the executable's objects were the ones that linked all
along, so the list is gone and the files compile once, into the
executable. Extraction now means adding a file to
`tests/arch/gamemodel_files.txt` (which removes it from the exe by absolute
path and asserts it) and cutting its `g_p*`/UI seams.

Order of attack (dependency-ranked; re-verify with an include scan when
starting each — the scan is one grep, and the ranking below is from a
2026-09-01 reading of the include graph, not a proof):

- [x] **4.0 Compile the VS_UI client sources once.** Drop
  `VS_UI_CLIENT_SOURCES` from the `VS_UI` target so its 36 `Client/*.cpp`
  files are built only into the executable, where they already linked
  from.
  > **Status:** done (2026-09-02, `restructuring/vsui-single-compile`;
  > live verification gates the merge as always, but this one is also
  > proven offline). Objects handed to the linker are always linked;
  > library members only when they resolve something still undefined,
  > and every symbol the 36 `VS_UI.lib` copies defined was already
  > defined by the executable's own objects — so the copies never
  > linked, and the executable ran on its own objects. The opposite
  > repair (make the `REMOVE_ITEM` work, link the `_LIB`-compiled
  > copies instead) would have changed which objects run and, through
  > the `#ifndef _LIB` members in the VS_UI headers, class layouts.
  > **Proof:** a linker map of `DarkEden.exe` built before and after,
  > reduced to symbol → defining object: 989,469 rows each, 0 differ;
  > no `VS_UI:<client file>.obj` contributed before, and `VS_UI.vcxproj`
  > carries no `Client/` source after. The list, the exe's
  > never-matching `REMOVE_ITEM` entry and the gamemodel assertion
  > against the list are gone. R1 unchanged at 517 (the files were
  > already counted); R4 59 → 35 by reclassification — the 24 of those
  > files that reach `g_p*` globals are executable debt now, which R1
  > counts.
  - Owner: the linker-map comparison recorded above; R4's membership no
    longer parses a CMake list.

- [x] **4.1 Pure tables first:** `ExperienceTable.cpp`,
  `MItemOptionTable.cpp`, `MGameStringTable.cpp`, `MSoundTable.cpp`,
  `SystemAvailabilities.cpp`, `FameInfo.cpp`. Data-in/data-out, load from
  files — the `TArray`-style loader tests already show the test pattern.
  > **Status:** done (2026-09-02, `restructuring/gamemodel-tables`;
  > live verification gates the merge). `gamemodel` exists: a static
  > library over `basic` + `packetwire` (+ iconv for `MString`), no
  > SDL/UI/dxlib, with the packetwire discipline — membership in
  > `tests/arch/gamemodel_files.txt`, read by CMake (`.cpp` lines), the
  > include checker (rules **M0–M2**: every listed file exists under
  > `Client/`; the closure may include only `basic/`,
  > `Client/framelib/`, `Client/Packet/`, `Client_PCH.h` and the `.h`
  > lines of the file; no `MinTr.h`/`DebugInfo.h`/`DebugKit.h`) and
  > the ratchet script (R4). Members: the six tables, `ExpInfo.cpp`,
  > and the support they stand on — `MString.cpp`, `MStringArray.cpp`,
  > `DebugLog.cpp` (the lib-safe logger 5.1 wants finished; its dead
  > `#if 0` reach to `g_pDebugMessage` was deleted). The include scan
  > confirmed the 2026-09-01 ranking: the six were clean apart from two
  > `g_pFileDef` seams, cut so the files reference no executable
  > global: `UseEnglishText` takes the `Properties` table (with a
  > path-taking `UseEnglishTextFrom` underneath, the testable core), and
  > `SystemAvailabilitiesManager::LoadFromFile` — which opened the
  > archive through `CRarFile` and `g_pFileDef` — became
  > `LoadFromStream(std::istream&)`, with `GameInit.cpp` reading the
  > archive and handing over the lines (`MinTr.h` was a vestigial
  > include). `FameInfo` needs only the `SKILLDOMAIN` enum from
  > `SkillDef.h`, which is listed as a header, not compiled.
  > **Found on the way:** the six tables had been compiled into BOTH
  > `VS_UI` and the executable all along — `VS_UI_CLIENT_SOURCES` is
  > relative and the exe's `REMOVE_ITEM` never matched the glob's
  > absolute paths (the LNK4217 trap CLAUDE.md names). The membership
  > removal is absolute and asserted, so **R1 528 → 518**: the ten
  > members left the executable, ten fewer twice-compiled units. And a
  > real defect: `ITEMOPTION_TABLE::LoadFromFile` wrote the part names
  > into two fixed `MAX_PART` arrays for however many the file declared
  > — the test's oversized count crashed the suite before the bound
  > went in (test-first, lib + test).
  > **Tests** (`test_gamemodel_tables.cpp`, 12): each loader fed its
  > own byte layout from a scratch file — experience by level with
  > out-of-range reads yielding the default, item options with part
  > names and the two rejections (oversized and negative counts), the
  > sound table round trip, the nickname layout's index bound, fame by
  > domain and level, the availability script's three record kinds
  > (script filters gated by the system flag, zone lists by open degree
  > with the out-of-range degree dropped, per-degree scripts following
  > their zone), and the language file's English decision. R4 61 → 59.
  > Suite: 184 tests (3,284 checks) green plain and ASan; all four
  > trees 0 errors.
  > **Adversarial review round (2026-09-02, 2 reviewers by angle — the
  > cap in force from here: one SHIP, one NO-SHIP), fixed on the
  > branch:** the new "member still in `VS_UI_CLIENT_SOURCES`" assertion
  > compared an absolute path against the relative list — the very
  > mismatch it was guarding against — and could never fire; it now
  > compares repository-relative. The double-compilation finding was
  > understated: the 37 files still on the list are all compiled into
  > both targets today (recorded in the Phase 4 intro above; fixing the
  > `REMOVE_ITEM` is its own branch). The 3.1 "every fix names its path"
  > claim was false for four earlier commits and is corrected there.
  > The tests' `strcmp` on `GetString()` now goes through a NULL-safe
  > helper, so a regression fails a check instead of crashing the run.
  > Observation kept, not fixed: a refused item-option table leaves the
  > part-name `MString`s NULL and three `VS_UI` call sites `strcpy` them
  > unguarded — pre-existing for any unset part, and the alternative was
  > heap corruption at startup; a 4.3/4.4 seam.
- [ ] **4.2 Money/price/trade logic:** `MMoneyManager.cpp`,
  `MPriceManager.cpp`, `MTradeManager.cpp` (seams to `g_pShop`/UI to cut).
  > **Status:** money done, price/trade re-ranked (2026-09-02,
  > `restructuring/gamemodel-money`; live verification gates the merge).
  > The include scan the plan asks for overturned the 2026-09-01
  > ranking for two of the three: `MPriceManager` prices through
  > `MItem`, `g_pItemTable`, `g_pPlayer` (race, stats, level),
  > `g_pEventManager`, `g_pTimeItemManager`, `g_pSkillAvailable` and
  > `g_pUserInformation` (its `g_pZone` reach is commented out; the
  > player and time-item ones sit under `__GAME_CLIENT__`, live in every
  > client build), and `MTradeManager` is an `MInventory`
  > shuffle over `MItem`s — both stand on the item core and the
  > containers, so they move **after 4.3/4.4**, not before. **`MMoneyManager`
  > is in `gamemodel`**: its one reach, the storage-box help hint
  > `SetMoney` raised past 100,000, is a per-wallet hook the executable
  > installs on the player's wallet at start-up (the trade and storage
  > box wallets carry none — under the old process-wide `static`
  > one-shot, any of them crossing the threshold consumed the hint).
  > **A defect fixed test-first:** `CanAddMoney` compared the amount
  > alone against the limit and ignored the balance, so a wallet near
  > the limit answered yes and the `AddMoney` that followed answered no
  > — the trade manager asks the first before accepting and does the
  > second after, and the other side's money had nowhere to go in
  > between (its `else` branch is an empty comment). Now it answers for
  > the balance the add would leave, without overflowing on a large
  > amount. `test_money_manager.cpp`: 8 tests (limits, add/use,
  > CanAdd/CanUse agreeing with Add/Use around the edge, the hint once
  > per wallet through the hook, no hook no hint). R1 518 → 517 (the
  > file was another of the double-compiled VS_UI entries). Suite: 192
  > tests (3,334 checks).
  > **Adversarial review round (2026-09-02, 2 reviewers, both SHIP with
  > findings), fixed on the branch:** the copy-constructor test copied a
  > wallet that had already hinted, so "state copied" and "hook dropped"
  > were indistinguishable — split into two wallets where only one half
  > can suppress the hint; the compiler's `operator=` would have copied
  > the hook where the copy constructor drops it — an explicit one now
  > copies balance, limit and state and keeps the target's hook, with a
  > test; `g_pZone` was listed as a live price dependency but is only a
  > commented-out condition; the R1 row's "37 files still on that list"
  > was 36 after this move; and the fix commit's "four checks" was
  > counted against a hybrid old body — under the original one the
  > negative-amount check fails too, five. Suite: 194 tests (3,345
  > checks).
- [ ] **4.3 Containers:** `MInventory.cpp`, `MStorage.cpp`,
  `MShopShelf.cpp`, `MQuickSlot.cpp` — the shop/stash index-bounds fixes
  from the review live here and deserve permanent tests.
  > **Status:** not started.
- [ ] **4.4 Item/skill cores:** `MItem.cpp`, `MItemManager.cpp`,
  `MSkillManager.cpp`, `SkillDef.cpp`, gear classes. Likely partial —
  whatever stays coupled goes on the exemption list explicitly.
  > **Status:** not started.
  - Owner (all of 4.x): `gamemodel`'s membership file
    (`tests/arch/gamemodel_files.txt`), the M0–M2 include rules in
    `check_includes.pl` (in force since 4.1), R4 shrinking.

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
