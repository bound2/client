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
  commit when it has no seam to cut. Move first, prove the build, then fix —
  separate commits, so the diff that changes behavior is readable. A file
  that only links once its reaches go through a host moves and cuts in one
  commit (the Phase 4 pattern), and that commit names every change that is
  not the move.
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
| R1 | Translation units compiled directly into the DarkEden target | **492** (493 before task 5.1's third slice took `ClientPlayer.cpp` into packetwire — its last two game-code includes served one function the build never compiles, and they are behind `WireHost` now; 492 before task 5.3 split `TextService::RenderText` out as `Client/TextServiceScreen.cpp` — a recorded growth, the price of a TextSystem that links without a test stub; 493 before task 5.1's second slice moved `ClientCommunicationManager.cpp` into `packetwire`, the three tuning values it read from the executable's config having gone behind `WireHost`; 495 before task 5.1 moved `Player.cpp` and `DatagramSocket.cpp` into `packetwire`, the two holdouts the debug facilities were the only thing keeping out; 497 before 4.4's fourth slice moved `MSkillManager.cpp`, `MSkillInfoTable.cpp` and `SkillDef.cpp` into `gamemodel` and split the player-facing half out as `MSkillAvailable.cpp`, −3 +1; 502 before 4.4's third slice moved `MPlayerGear.cpp`, the three race gears and `MShop.cpp` into `gamemodel`; 503 before 4.2's third slice moved `MPriceManager.cpp` into `gamemodel`; 505 before 4.2's second slice moved `MTradeManager.cpp` and `MSortedItemManager.cpp` into `gamemodel`; 508 before 4.3's second slice moved `MInventory.cpp`, `MStorage.cpp` and `MShopShelf.cpp` into `gamemodel`; 512 before 4.3's first slice moved the three item managers and `MQuickSlot.cpp` into `gamemodel`; 515 before 4.4's second slice moved `MItem.cpp`, `MObject.cpp`, `UserInformation.cpp`, `ClientConfig.cpp` and `MTimeItemManager.cpp` into `gamemodel` and split their executable halves out as `MItemUse.cpp` and `MObjectScreen.cpp`, +2 −5; 516 before 5.2 deleted the dead `MitemTableInit.cpp`; 517 before 4.4's first slice moved `MItemTable.cpp`; 518 before 4.2 moved `MMoneyManager.cpp`, another double-compiled VS_UI entry; 528 before task 4.1's `gamemodel` took its ten members out — the four support sources, and the six tables that the relative `VS_UI_CLIENT_SOURCES` list had never actually removed from the exe glob, so they compiled into both VS_UI and the executable — as the 36 files still on that list do; 529 before task 2.5 deleted the dead `CRRequest2Handler.cpp`; 992 before task 2.4 moved the 465 packet/table/info sources into `packetwire`, +1 for the split-out `GCExchangeBuyHandler.cpp`; 1,044 before task 1.1; the task-2.2 composition root `PacketHandlerRegistry.cpp` was a recorded +1, offset when finishing the migration deleted `CGHandlersStub.cpp`) | `grep -c "<ClCompile Include" build/vs2022/DarkEden.vcxproj` — `ratchets.sh` reads the generated vcxproj, preferring the ctest run's own build dir; on generators with no vcxproj it reports SKIP, not PASS |
| R2 | Packet `.cpp` files still defining a packet-style `::execute(Player` | **0** (448 → 432 in slice 1 → 0 when 2.2/2.3 finished; regex refined at 0 to stop matching comments and the in-file handler body in `GCExchangeBuy.cpp`) | `grep -rlE '^void\s+\w+::execute\s*\(\s*Player' Client/Packet/{Gpackets,Cpackets,Lpackets,Rpackets,Upackets} --include='*.cpp' \| grep -v Handler \| wc -l` |
| R3 | Live `sprintf`/`strcpy`/`strcat` lines under `Client/Packet` **and `Client/PacketHandler`** | **18** (19 before the review round of task 5.4's first slice found a pre-existing overflow one line below a converted site — `GCBloodBibleListHandler` `sprintf`ing `"%3d %s"` into `char[192]` from a `char[192]`, four bytes short — and bounding it converted a 28th `sprintf` line whose format is a literal and was never a C19 site, which is the difference between R3 and R7 in one example; 46 before task 5.4's first slice converted all 31 game-string-table format sites in `Client/PacketHandler` to `SafeFormat::Format`; only 27 of the 31 move this number, because `\b` rejects the `w` in `wsprintf` — R7 counts all 31, and each ratchet is blind to something the other sees; 46 was unchanged by task 2.4, which widened the scope to follow the handlers out of `Client/Packet`; 61 at first measurement — the 2026-09-01 adversarial review showed a quarter of that was commented-out code, so the measurement now excludes `//` matches) | see `ratchets.sh` — the grep excludes comment-prefixed matches |
| R4 | Library-compiled `.cpp` files referencing `g_p*` client globals **no library file defines**, comment lines excluded | **21** (25 before task 5.3, which both cut a real reach and refined the measurement: `TextService.cpp` lost its live `g_pLast` when `RenderText` moved to the executable, and R4 — unlike R3 and R5 — counted comment lines, so the sentence 5.3 wrote *about* that cut kept the file in the number. Stripping comments drops three more that never had a reach: `SocketOutputStream.cpp`, `SpriteLibBackendSDL.cpp`, `VS_UI_WebBrowser.cpp`. All 21 remaining are real and every one is a `VS_UI` file; 27 before 4.4's fourth slice moved the skill core into `gamemodel` — a reclassification again: `VS_UI_SKILL_VIEW.cpp` and `VS_UI_skill_tree.cpp` reached past the libraries only for `g_pSkillInfoTable`, `g_pSkillManager` and `g_pSkillAvailable`, which `MSkillManager.cpp` defines; 28 before 4.4's third slice moved the gear into `gamemodel` — a reclassification: `VS_UI_Game.cpp`'s only reaches past the libraries were `g_pSlayerGear`, `g_pVampireGear` and `g_pOustersGear`, which the gear sources define; 35 before 4.4's second slice — a reclassification again, 35 + 1 − 8: the subtraction became library-wide, so a library file reading a global another library file defines is no longer a seam; `MItem.cpp` joined reading `gamemodel`'s own tables, +1 under the old per-file rule, and the union rule excludes it with seven earlier members — `Datagram.cpp` reading `packetwire`'s factory manager, and six `VS_UI` sources whose only reaches are `gamemodel`'s tables, `packetwire`'s `g_pFileDef` or `VS_UI`'s own globals; 59 before task 4.0 — a reclassification, not seam-cutting: the 36 `VS_UI_CLIENT_SOURCES` files stopped being library-compiled, so the 24 of them that reach globals are executable debt now, counted by R1 and outside this ratchet; 61 before task 4.1 cut the two `g_pFileDef` seams in `MGameStringTable` and `SystemAvailabilities` and added the `gamemodel` membership file, whose four new members reference no game global; 81 before task 2.4 grew the membership from 52 to 518 files; the number fell because the measurement stopped counting a file's references to globals it defines — the packet tables own `g_pPacketFactoryManager`/`g_pPacketValidator` — and the two dead server-only bodies that reached game globals were deleted; 83 at first measurement, before two never-compiled files were filtered) | `ratchets.sh` computes it over the library dirs (minus CMake-excluded files) plus the `packetwire` and `gamemodel` membership files |
| R5 | Direct packet `execute()` call sites outside `Client/Packet` (handlers under `Client/PacketHandler` are in scope) | 1 (a commented-out block in `CGameUpdate.cpp`; added 2026-09-01 after the review found live local-echo callers the receive-loop enumeration had missed; task 2.4 found two more inside handlers — `GCReconnectLoginHandler`/`LCReconnectHandler` fabricating a `CGConnectSetKey` — invisible while handlers lived under the excluded `Client/Packet`, caught by the compiler once `Packet::execute` was deleted, and routed through the dispatcher; a live caller is now a compile error before it is a ratchet failure) | see `ratchets.sh` |
| R6 | *retired* — `packetwire` members calling `SendBugReport`, which the executable used to define | — (lived for one slice, 2026-09-03. Task 5.1's first slice added it to replace the failed-link detector that stubbing the symbol had disabled — a library file calling an executable-side *function* is invisible to W1/W2, which read includes, and to R4, which greps `g_p*`. It fired on the very next thing done to the tree: promoting `ClientCommunicationManager.cpp` took the count to 2, which is what said to move the function rather than grow the seam. `SendBugReport` is in `Client/Packet/WireHost.cpp` now, so the ratchet measures a symbol nothing is on the wrong side of, and the stub that disabled the link detector is gone with it. Note what came back is narrower than what left: a failed link catches an executable-side call only in a library `unit_tests` links, and only in an object some test forces the linker to pull in — which is what the address-taking link proofs in `test_wire_host.cpp` and `test_player_base.cpp` exist to guarantee. R6 grepped every membership file unconditionally. `ratchets.sh` keeps the history where the check was) | — |

| R7 | Call sites handing a game string table entry to `printf` as its **format** argument, **where the lookup is spelled at the call site** (`sprintf` family including `fprintf`, the counted family, `AddFormat`, the offset-append form and `.Format`, across `Client` and `VS_UI`) | **0** - every site it can see is converted, so it holds a line instead of tracking a retreat. **On its own it is not a measure of finding C19 being closed**, and was once mistaken for one: an entry copied into a static array or a local and used as a format from there is invisible to it, and 24 live sites did exactly that until task 5.4's fifth slice. R8 below is the ratchet that can see them, and C19's closure rests on the pair plus three hand audits, never on this number alone (37 before task 5.4's fourth slice took the last of them; 64 before task 5.4's third slice took the `AddFormat` family through `CMessageArray::AddSafeFormat`, leaving 37 ordinary `sprintf` sites - `UIMessageManager.cpp` 14, `MTopView.cpp` 10, `GameUI.cpp` 7, `ModifyStatusManager.cpp` 3, `CGameUpdate.cpp` 2, `PacketFunction.cpp` 1; 262 before task 5.4's second slice converted every `VS_UI` site, leaving only the executable's own - of which three are the `AddFormat` family in `Client/PacketHandler`; 293 before its first slice converted `Client/PacketHandler`'s 31; the split is `Client` 64 — of which `ModifyStatusManager.cpp` 19, `UIMessageManager.cpp` 14, `MTopView.cpp` 10, `PacketFunction.cpp` 9, `GameUI.cpp` 7 — and `VS_UI` 198, where `VS_UI_GameCommon.cpp` alone holds 89. First recorded as 287 → 256: the review round found the pattern could not match a counted call in **any** form, because it wanted the format at argument two, where `snprintf` and `swprintf` take a size — so it missed a live site and, worse, could not have caught a new one. Added 2026-09-03; finding C19 as a number) | see `ratchets.sh` — **five** alternatives: the format sits at a different argument in each of the three call families, the offset-append form `sprintf(buf + strlen(buf), …)` needs its own because the destination class forbids parentheses, and `MString::Format` is a printf reached as a method, which no pattern matching on a printf's name can see. The tree is joined before matching, because four sites put the destination and the format on different lines |
| R8 | printf-family calls whose **format argument is not a string literal**, across `Client`, `VS_UI` and `basic`, **headers included** | **43** - the population R7 measures a spelling of. Every one of the 43 was read: 28 vararg forwarders where the format is the function's own parameter, 6 inside `SafeFormat`'s own `Emit`, 3 literals behind `TEXT()`/`_T()`, and 6 declarations rather than calls. This is the weaker question, which is exactly why it is the right floor - it cannot tell a table entry from a legitimately forwarded format, so it cannot be satisfied by renaming anything. Added 2026-09-04 by task 5.4's fifth slice, because R7 reaching 0 had been read as C19 being closed and was not (37 on the narrow pattern before that slice; 24 of those were its work, taking the narrow count to 13. **First recorded as 13, and 13 was not the population** — the slice's own review round found the pattern missing the `AddFormat` family, bare `printf`, `basic/` and every header, and widening it to what this row's scope column had claimed all along gave 43 with nothing in the tree changed) | see `ratchets.sh` — the family list was enumerated from the tree rather than written from memory, which is how `fprintf` (550 calls) and `vswprintf` got in; the enumeration command itself had to be fixed, since as first written it could not have matched a bare `printf(`. It cannot see a destination containing parentheses; those were audited by hand at 16 sites, all with literal formats |

R1 is the headline number: it counts what still cannot be unit-tested. R2 is
the client twin of the server's R4 (which it drove to 0). R3, R7 and R8 track
code-health priority 2 mechanically, from three directions: R3 counts the
unbounded copies in the packet tree, R7 counts data-file format strings where
the lookup is spelled at the format argument, and R8 counts every format
argument that is not a literal, whatever it is spelled as. R7 and R8 exist as a
pair on purpose. R7 is precise and blind to indirection; R8 is coarse and
cannot be evaded by spelling. Reading either one alone as the state of finding
C19 is the mistake that cost a retraction on 2026-09-04.

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
  > **Hook added (2026-09-02, `restructuring/gamemodel-item-core`, with
  > the 4.4 second-slice fixes):** `tools/git-hooks/commit-msg` refuses a
  > `fix:` commit without a `Test path:` line naming one of the three;
  > installed per clone with `git config core.hooksPath tools/git-hooks`
  > (CLAUDE.md carries the instruction). No bypass flag on purpose.
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
  > the after-link forced non-incremental so the map is really rewritten
  > (the first attempt compared a stale map with itself — the review
  > caught it): 989,469 symbol → defining-object rows each, 0 differ;
  > the objects appear in the same order; every symbol sits at the same
  > address — the two images differ only in their timestamps. No
  > `VS_UI:<client file>.obj` contributed before, and `VS_UI.vcxproj`
  > carries no `Client/` source after. The list, the exe's
  > never-matching `REMOVE_ITEM` entry and the gamemodel assertion
  > against the list are gone. R1 unchanged at 517 (the files were
  > already counted); R4 59 → 35 by reclassification — the 24 of those
  > files that reach `g_p*` globals are executable debt now, which R1
  > counts.
  > **Adversarial review round (2026-09-02, 2 reviewers; one NO-SHIP,
  > one SHIP-with-fixes), fixed on the branch:** both caught that the
  > first "after" map was the before map again — the incremental relink
  > had not rewritten it — so the proof above is the re-taken one; the
  > non-Windows executable filters `Client/Client.cpp` out (its WinMain)
  > and used to get it from `VS_UI.lib`, so that one file stays in
  > `VS_UI` on `NOT WIN32` — unverified, Windows being the live path, but
  > not knowingly broken; two CMake comments still said `Client.cpp` was
  > "compiled into the VS_UI lib" and the R4 row's method column still
  > named the deleted list; the code-health review's High finding on the
  > silent `REMOVE_ITEM` is marked fixed. **Left open, recorded:** the
  > review's neighbouring High finding — `_LIB` is defined for `VS_UI`
  > and not for the executable, and the `#ifndef _LIB` members in the
  > VS_UI headers give the 54 `VS_UI/src` objects a different
  > `C_VS_UI_TITLE` layout than the executable's objects see. This task
  > was that finding's stated precondition and did not change which
  > objects link, so the hazard is exactly as before; it is next for
  > this area.
  > **Second slice (2026-09-02, `restructuring/vsui-lib-define-public`;
  > live verification gates the merge):** that hazard closed. `_LIB` is
  > not a detail of VS_UI's build: its ~100 `#ifndef _LIB` regions are
  > the standalone UI test harness (own dialogs, `gpC_press_button`, the
  > message-queue accessors, `VS_UI_ExtraDialog.h`), which the original
  > VC6 library configuration compiled out — and some of those regions
  > are class members, so a translation unit including the headers
  > without `_LIB` sees a different layout than the library's objects.
  > The definition is now `PUBLIC` on the `VS_UI` target, so the
  > executable's 72 translation units that include VS_UI headers (50
  > under `Client/`, 22 packet handlers) compile with it
  > too and both sides of every call agree on where the members are.
  > Dropping `_LIB` instead would have compiled the harness into the
  > game. Nothing in `Client/` used a harness-only member (the build
  > would have said so); the three `_LIB` mentions in `Client/` are dead
  > under `__GAME_CLIENT__` or `OUTPUT_DEBUG`, and `g_bEnable3DHAL` keeps
  > its one definition in `GameInit.cpp` (`VS_UI_Title.cpp` declares it
  > `extern` under `_LIB`).
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
  > **Status:** done (3e00211 the money manager, 8090919 the trade
  > manager, b4e37d5 the price manager; owner: the `gamemodel`
  > membership file, the CMake assertion that no member is in the
  > executable's list, and the include checker). Price/trade re-ranked
  > (2026-09-02,
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
  > **Second slice (2026-09-03, `restructuring/gamemodel-trade`; live
  > verification gates the merge):** the trade manager. `MTradeManager`
  > (the exchange: the player's inventory and wallet against the other
  > side's offer, the accept delay, the feasibility check that packs the
  > offer into a scratch copy of the player's grid, the swap) and
  > `MSortedItemManager` (the size-ordered map the packing goes through,
  > biggest footprint first, then by id) join `gamemodel`. Its one reach
  > was the executable's millisecond clock, `g_CurrentTime`, read through
  > a guarded `extern`; the clock is an `MItemHost` entry now,
  > `pCurrentTime`, beside the animation clock, and without one (a test
  > binary, or a host that carries none) there is no delay — the
  > semantics of the `#else return true` branch the file used to carry
  > for a non-client build. Its other reaches — `g_pInventory`,
  > `g_pMoneyManager`, the accept delay in `g_pClientConfig` — are
  > library-defined. Dropped on the way: the `__GAME_CLIENT__` guards
  > around the delay (always on in every build of the file), the
  > `PacketDef.h` include (an aggregation header the file used nothing
  > of), the 112-line commented-out `CancelTrade` body, and
  > `MSortedItemManager`'s two commented-out functions with the 2×2
  > packing path only that comment reached; `MTradeManager.cpp` includes
  > `DebugLog.h` instead of `DebugInfo.h` and `ClientConfig.h`
  > unguarded, and `Release` forgets the player's inventory it never
  > owned. `MPriceManager` stays out: it prices through `g_pPlayer`,
  > `g_pEventManager` and `g_pSkillAvailable`, a later slice. R1 505 →
  > 503; R4 unchanged. Tests (`test_trade_manager.cpp`, 6): `Init` builds
  > the other side over the player's inventory and `Release` deletes only
  > what the manager made; refusing after an accept starts the delay on
  > the installed clock, the other side's refusal restarts it, and no
  > clock means no elapse; `CanTrade` needs room in the player's grid for
  > every offer once the items marked for trade are out, leaves both
  > inventories as they were (the offers' grid positions included), and
  > refuses money the wallet cannot take; `Trade` swaps only once both
  > accepted, deletes the player's offered items, lands the other's in
  > the free space with the trade flag cleared, moves the money and
  > consumes both acceptances; `CancelTrade` refunds the money the player
  > put up; the sorted map orders bigger footprints first, then lower ids,
  > and refuses the same key twice. **One defect fixed test-first, in
  > the item core:** `MItem`'s constructor never set `m_bTrade` (nor the
  > grid position or the durability), so a fresh item read heap garbage
  > for "offered in this trade" — the trade tests found it as a kept
  > sword the exchange deleted and a full grid `CanTrade` called empty.
  > The executable masks it at trade start (`GameUI` clears the flag on
  > every inventory and gear item), but an item that arrives during a
  > trade — a pickup, a purchase — carries whatever its bytes held, and a
  > `TRUE` there leaves it out of the feasibility check and deletes it on
  > `Trade`; the constructor defines all four members now. Suite: 241
  > tests (3,894 checks).
  > **Adversarial review round (2026-09-03, 4 reviewers over this slice
  > and 4.3's second, all SHIP with findings, no runtime defect), fixed
  > on the branch:** the clock had gone in as a second static seam on
  > `MTradeManager`, beside the host struct and the money manager's
  > per-wallet hook, and made CLAUDE.md's "behind the `MItemHost`"
  > untrue for it — it is a host entry now, and a missing clock means no
  > delay rather than a delay that never ends; the fix commit called its
  > new checks a regression guard, but `/RTC1` fills the test's
  > stack-local item with `0xCC`, so they fail deterministically without
  > the fix — a reproduction, and the commit's wording is corrected here;
  > `MPetItem` never set its remaining experience or its food type, the
  > same defect one class away, fixed executable-side (`Test path:
  > exempt`); `Release` left `GetMyInventory` pointing at the player's
  > inventory — NULL now, pinned; the dropped `PacketDef.h` include, the
  > commented-out `CancelTrade` body and `MSortedItemManager`'s dead
  > packing path are named above and gone; "not worn" on a zero
  > durability read backwards (zero is worn out); the fix commit says
  > four members where the record said three things; `CancelTrade`
  > refunds money only — the offered items keep their flag until the
  > next trade start clears it, and a refused refund still answers true
  > (pre-existing, recorded, not tested); the tests/CMakeLists comment
  > named the wrong headers for the definitions (`Exception.h` and
  > `CreatureTypes.h` are the ones that switch on them); the restore
  > check pins both offers' positions and the other grid's cells. Noted,
  > not done: the four `gamemodel` test fixtures (five after the price
  > slice) share a byte-identical
  > teardown — a `tests/support` base fixture is the next tidy-up. Suite:
  > 241 tests (3,903 checks).
  > **Third slice (2026-09-03, `restructuring/gamemodel-price`; live
  > verification gates the merge):** the price manager. `MPriceManager`
  > — what a shop charges or pays: the market conditions, the option
  > multipliers, wear, repair at a tenth of the damage, charges,
  > silvering, the gamble price — joins `gamemodel`. Its reaches into
  > the executable were the player (`IsSlayer`/`IsVampire`/`IsOusters`,
  > the worn and the basic stat sums, the level), the event manager
  > (the premium half-price event, the tax-change event's percentage)
  > and the skill set (the NEMA and JAVE blood bibles). They go through
  > `MPriceHost`, seven entries the executable installs beside the item
  > host: `Race`, `Level`, `StatSum`, `BasicStatSum`,
  > `IsPotionHalfPrice` (the event or NEMA — the code applied them as
  > two half-price branches of which at most one fired),
  > `IsGambleHalfPrice`, `ShopTaxPercent` (100 without the event, a
  > multiply by one — in 64 bits on both paths — where the code skipped
  > the multiply). Without a host a price carries no player, event or
  > skill adjustment. Dropped on the way: the five `__GAME_CLIENT__`
  > guards, two of them with an `#else` half — a `VS_UI`-only build's
  > `VS_UI.h` include and its reading of the race and stats from
  > `g_char_slot_ingame`, which no target compiles (`Client_PCH.h`
  > defines the macro unconditionally) — with the `MEventManager.h`,
  > `MSkillManager.h`, `MPlayer.h` and `MZone.h` includes, a
  > commented-out `g_mapPremiumZone` extern, and two commented-out
  > blocks (a skull rule, the `bMysterious` five-times rule).
  > `g_pItemTable`, `g_pItemOptionTable`, `g_pTimeItemManager` and
  > `g_pUserInformation` are library-defined. R1 503 → 502; R4
  > unchanged. Tests (`test_price_manager.cpp`, 9): the short circuits
  > (nothing, the event moon card, an unidentified item priced as a
  > gamble in every trade); the market conditions, and the skull's
  > buying rate both ways; option multipliers adding up and wear scaling
  > down, never below one; repair at a tenth of the damage, and never
  > for a vampire portal, a blood bible sign or a timed item; charged
  > items pricing every charge (the Ousters summon item at a fifth) and
  > repair refilling them; silvering at the full coat until the coat is
  > full; the host's weak-slayer potion rate, the consumables' half
  > price, the tax on what the shop charges only, the skull's vampire
  > and Ousters rates and then the head-price rate; star prices by the
  > first option's part and the item type; the gamble scaling the loaded
  > class average by basic stats or by level, halved and taxed.
  > **4.2 is complete.** Suite: 250 tests (3,987 checks).
  > **Adversarial review round (2026-09-03, 3 reviewers, all SHIP with
  > findings), fixed on the branch:** two deltas the move had
  > introduced — the gamble price's tax multiply ran in a 32-bit `int`
  > on every call where the old code multiplied only under the event
  > (an overflow above 21,474,836, data-dependent; both paths compute
  > in 64 bits now), and the host narrowed the server-sent `DWORD`
  > percentage to `int`, flipping the sign of a hostile value (the seam
  > carries it unsigned, as the old multiply did); the host's player
  > readers answer "none" for the window between `MODE_WAIT_UPDATEINFO`
  > and the next `GCUpdateInfo` in which there is no player (the old
  > code would have crashed there; nothing prices an item then); the
  > seven host readers are private statics on the class, the `MItem`
  > pattern, not free functions; "the `#else` half of every block"
  > overstated — three of the five guards had none; the two no-host
  > checks could not fail (the fixture's defaults were the no-host
  > answers) and now set answers that would move the price; the sixth
  > potion type, a negative maximum durability and a charged consumable
  > (its charges alone, no half price, no tax) are covered; the mace
  > silvering comment named the wrong branch; the Korean left inside
  > the touched function is English, with two dead commented-out blocks
  > gone. Noted, not done: `bMysterious` is a dead parameter (its only
  > uses were the dropped comment) that `UIMessageManager` still
  > computes an argument for; a star price for item type 0 is −20
  > stars; the **five** `gamemodel` test fixtures now share a
  > byte-identical teardown — the `tests/support` base fixture is the
  > next commit before the next slice, not a note to walk past again.
  > Suite: 250 tests (3,995 checks).
- [ ] **4.3 Containers:** `MInventory.cpp`, `MStorage.cpp`,
  `MShopShelf.cpp`, `MQuickSlot.cpp` — the shop/stash index-bounds fixes
  from the review live here and deserve permanent tests.
  > **Status:** done (4cede66 the managers, 0feaf59 the containers;
  > owner: the `gamemodel` membership file, the CMake assertion that no
  > member is in the executable's list, and the include checker).
  > **First slice (2026-09-02, `restructuring/gamemodel-containers`; live
  > verification gates the merge):** the managers. `MItemManager` (the
  > id map), `MGridItemManager` (the grid) and `MSlotItemManager` (the
  > slots) join `gamemodel` with `MItemFinder.h` and `MQuickSlot.cpp/.h`,
  > which define the belt and arms-band globals the id map clears on
  > release, so that reach is library-internal. The grid manager was
  > pure; the slot manager drops a `DebugInfo.h` include that served a
  > commented-out `DEBUG_NEW`. The container-based gear (`MBelt`,
  > `MOustersArmsBand`, `MMotorcycle`) moves back from `MItemUse.cpp`
  > into the core, byte for byte; `MCorpse` stays executable-side (it
  > owns an `MCreature`). R1 512 → 508; R4 unchanged. Tests
  > (`test_item_containers.cpp`, 10): the id map's one-item-per-id rule
  > and predicate search; the grid's placement, overlap/spill/range
  > refusals, top-down column scan, removal clearing every covered cell,
  > replacement over at most one old item; the slots by index and by
  > id; a belt sizing its pockets from the table and taking only quick
  > items; every test ending in `Release` proving ownership. **One defect
  > fixed test-first:** the slot manager wrote the item into its slot
  > before the id map could refuse it, so a refused item (its id already
  > held) sat in a slot the map knew nothing about — the slot looked
  > taken, a later remove gave it up, nobody owned the item — and
  > `ReplaceItem` had dropped the occupant by then; the map decides
  > first now, and a refused replacement puts the occupant back. Suite:
  > 226 tests (3,660 checks). **Next, the containers proper:**
  > `MInventory.cpp` (the player refreshes affects on add, sounds on
  > pile/charge changes, and the effect list over `MEffect` /
  > `g_EffectGeneratorTable` / `g_CurrentFrame` — an executable half to
  > split out), `MStorage.cpp` (one `g_pPlayer->CheckAffectStatus` loop)
  > and `MShopShelf.cpp` (the same refresh, `g_ShopFixedItemTable`, a
  > `DEBUG_ADD_FORMAT`). The affect refresh is the `MItemHost`'s
  > `RefreshPetAffect` under another name — one host entry serves all
  > three.
  > **Adversarial review round (2026-09-02, 2 reviewers, both SHIP with
  > findings), fixed on the branch:** a real sibling defect — the grid's
  > `ReplaceItem` removed the occupant and cleared its cells, then
  > returned `true` even when the id map refused the newcomer, so
  > `MInventory::ReplaceItem` reported a replacement that had put the
  > newcomer nowhere and the drop handler picked up the "old" item on
  > the strength of it; fixed test-first the same way as the slots (the
  > occupant goes back, the call says no), and the slot fix's claim that
  > "the grid already worked in this order" is corrected — its two
  > `AddItem` overloads did. The round also sharpened the slot defect's
  > consequence: the belt, stash and store handlers `delete` an item the
  > container refuses, so the slot held a freed pointer, not merely an
  > unowned one. Tidy-ups: a spliced history line in the ratchet script,
  > a stale Korean banner left above the slot fix's new one, the
  > CLAUDE.md citation missing 4.3, one blank line the move-back had
  > dropped. Confirmed: every moved definition byte-identical and
  > present once, `CheckAffectStatus` overrides read only the item's own
  > fields so the reorder sees the same state, the client cannot collide
  > with its own ids (the pickup path removes before it re-adds), the
  > tests' arithmetic and ownership counts. Suite: 227 tests (3,668
  > checks).
  > **Second slice (2026-09-02, `restructuring/gamemodel-containers-2`;
  > live verification gates the merge):** the containers proper.
  > `MInventory`, `MStorage` and `MShopShelf` join `gamemodel` with
  > `MGameDef.h`. Their two reaches — the player's affect check on an
  > item (on add, on a box switch, on a shelf set) and the inventory
  > sound an item makes when it lands — go through `MItemHost`, whose
  > pet-only `RefreshPetAffect` became `RefreshAffect` (it was the same
  > `g_pPlayer->CheckAffectStatus` call) and which gained
  > `PlayItemSound`; `MItem::RefreshAffect` / `MItem::PlayItemSound` are
  > host-guarded statics so no caller checks for a host. Dropped:
  > `MInventory.cpp`'s unused `MHelpManager.h` include, a
  > `CheckAffectStatus()` overload declared and defined nowhere, and two
  > commented-out blocks (the inventory's effect list, whose members
  > left the header long ago; `MShopFixedShelf::InitFromFixedItemTable`);
  > `MShopShelf.cpp` includes `DebugLog.h` instead of `DebugInfo.h`. R1
  > 508 → 505; R4 unchanged. Tests (`test_inventory_storage_shop.cpp`,
  > 8): the inventory takes inventory items only and plays their sound
  > once per placement, asks the player about every item added and
  > every one left after a removal, finds by class and type, and merges
  > a pile onto a held pile with room but never onto a quest pile; a
  > storage box refuses slots past twenty and everything before `Init`,
  > deletes what it overwrites, hands back what it removes, re-checks
  > the box switched to and ignores one that does not exist, and owns
  > its items and wallet; a shelf comes from the factory by type, fills
  > the first empty slot, deletes what it overwrites, refuses the
  > twenty-first item. **One defect fixed test-first, of the C13 shape:**
  > `MShopShelf::NewShelf` indexed its three-entry factory table with the
  > shelf type straight off the wire (`GCShopList`,
  > `GCShopListMysterious`), a value of 3 or more calling through a code
  > pointer past the table — `GCShopListHandler` even logged the
  > violation and called anyway; the factory answers NULL now and both
  > handlers return on it. Suite: 235 tests (3,810 checks). **4.3 is
  > complete** but for `MItemManager`'s executable-side siblings that
  > were never on its list; the next 4.x work is 4.4's remainder —
  > the skill core, now that the gear followed as 4.4's third slice;
  > 4.2's trade and price managers followed as its second and third
  > slices, on what is now in the library.
  > **Adversarial review round (2026-09-03, with 4.2's second slice, 4
  > reviewers, all SHIP with findings, no runtime defect), fixed on the
  > branch:** the move's "the same call" was not quite — the containers'
  > `g_pPlayer->CheckAffectStatus` was unguarded and the host body it now
  > goes through carries the pet path's `if (g_pPlayer!=NULL)`, so a NULL
  > player is a skipped refresh rather than a crash (kept, recorded
  > here); the comment the shelf fix added, "every caller handles a NULL
  > return", was false for three of five callers — `GCShopVersionHandler`
  > and `MNPC` pass in-range enumerators and never see NULL
  > (`SHOP_RACK_SPECIAL` and `SHELF_SPECIAL` are both 1, now said at the
  > call), reworded, and the guard is one unsigned compare so it reads
  > the same on every compiler; the test pinned the inventory sound with
  > all four sound slots equal, so a wrong slot would have passed — the
  > slots differ now; the `MInventory.h` half of the effect list the
  > `.cpp` side dropped is gone too (the record said its members left
  > "long ago" — commented out, not gone); a dead vector in the shelf
  > test; the refactor commit's suite count was 234 tests, not 233; the
  > Korean banners above the rewritten calls are English; and the status
  > line above said "not started" beside a record that said complete.
  > Noted, not done: `MShop::SetCurrent` and the three gear classes still
  > reach `g_pPlayer->CheckAffectStatus` unguarded, the reach the
  > containers cut — 4.4's remainder.
- [ ] **4.4 Item/skill cores:** `MItem.cpp`, `MItemManager.cpp`,
  `MSkillManager.cpp`, `SkillDef.cpp`, gear classes. Likely partial —
  whatever stays coupled goes on the exemption list explicitly.
  > **Status:** done (9da85ff the item table, 144c92a the item core,
  > 4cede66 the item managers, 12efc24 the gear, 486e3f6 the skill
  > core; owner: the `gamemodel` membership file, the CMake
  > assertion that no member is in the executable's list, and the
  > include checker). Everything the task lists is in the library but
  > the halves that are the packet and UI side of items and skills —
  > `MItemUse.cpp` and `MSkillAvailable.cpp` — which are executable by
  > design. First slice — the item table (2026-09-02,
  > `restructuring/gamemodel-items`; live verification gates the
  > merge). The scan of the item family: `MItem.h` is clean (`MObject`,
  > `MItemTable`, the two item managers, `ItemClassDef`), three of the
  > four item-manager sources reach no global (`MItemManager.cpp`
  > assigns the arms-band and quick-slot globals; the 4.3 containers
  > all reach the player or their own `g_p*`), and `MItem.cpp` is the
  > god class the plan expected — roughly three fifths of its methods
  > touch no global, the rest are the `UseQuickItem` family (packets,
  > player, zone, dialogs), the name lookups (`g_pUserInformation` for
  > the language) and the colour sets (`g_pClientConfig`). So 4.3 and
  > the rest of 4.4
  > wait on a split of `MItem.cpp` into a core the containers can be
  > linked against and an executable half; the item table went first
  > because nothing depends on that split. **`MItemTable` is in
  > `gamemodel`** with `ItemClassDef.h`, `AddonDef.h` and `DrawTypeDef.h`
  > as listed headers: its 23,000-line in-code item definitions are the
  > server's (`__INIT_ITEM__`, never defined here — the client loads
  > `Item.inf`), so the live code is `ITEMTABLE_INFO`'s loader,
  > `ITEMTYPE_TABLE`'s average price and `ITEMCLASS_TABLE::InitClass`;
  > `ITEMCLASS_TABLE::InitItem2` stays defined in the executable
  > (`MitemTableInit.cpp`, dead — called only from the server-only
  > constructor). Two reaches cut: a `DebugInfo.h` include nothing used,
  > and `MItem.h` included for the class enum, replaced by
  > `ItemClassDef.h`; the header now pulls its own platform types so a
  > test can include it alone. Tests (`test_item_table.cpp`, 6): a
  > full save/load round trip of every field, the file head pinned by
  > hand (English name first, then Korean, then description), the
  > average price over option-free items rounded to hundreds and zero
  > when none qualifies, and `InitClass` sizing one class. R1 517 → 516.
  > Suite: 200 tests (3,411 checks).
  > **Adversarial review round (2026-09-02, 2 reviewers, both SHIP with
  > findings), fixed on the branch:** a real defect the new tests had
  > walked past — `ITEMTABLE_INFO`'s constructor never set `Price`,
  > `Race` or `DropFrameID`, and `ITEMTYPE_TABLE` had no constructor for
  > its average price, so an `InitClass`'d slot no file entry filled
  > read garbage; the constructor test now pins every field and fails
  > without the fix (0xCC patterns under `/RTC1`), and a fresh type
  > table answers 0. The "four container sources reach no global"
  > sentence was wrong (three; `MItemManager.cpp` assigns three), the
  > method counts were one script's reading and are stated as a
  > proportion, and the test header claimed the round trip pins the
  > on-disk order when it pins agreement between save and load — only
  > the string head is pinned against bytes, and no shipped `Item.inf`
  > is in the repository. Noted, not fixed: `ITEMCLASS_TABLE::InitItem2`
  > (`MitemTableInit.cpp`, 8,488 lines) ends by calling itself —
  > unbounded recursion if it were ever reached — and `VS_UI/` carries a
  > second copy of that file that the Windows `VS_UI` target still
  > compiles (the `list(FILTER)` meant to drop it sits in the non-Windows
  > branch), so `VS_UI.lib` holds a twin the executable's object
  > outranks; both dead in this build — cleared by 5.2's first slice.
  > Suite: 201 tests (3,423 checks).
  > **Second slice (2026-09-02, `restructuring/gamemodel-item-core`; live
  > verification gates the merge):** the item core. `MItem.cpp` is split
  > per class: a class with any `UseInventory`/`UseQuickItem`/`UseGear`
  > body, or a container base (`MBelt`, `MOustersArmsBand`, `MMotorcycle`,
  > `MCorpse`), moves whole to the executable's new `MItemUse.cpp` with
  > the factory table, because a class split across a library and the
  > executable leaves its vtable referencing symbols a test binary cannot
  > link; `MItem` and the gear/armour/weapon families stay in `MItem.cpp`
  > and join `gamemodel` (twelve classes have out-of-line members in the
  > library; chunks moved byte for byte, the pile sizes to
  > `MItemLimits.h`, the commented-out twin of that block dropped).
  > `MItem`'s two reaches into the executable go through a host it is
  > handed at start-up (`MItemHost`: the animation clock behind the
  > colour cycles, the top view's item-drop frame pack, the player's pet
  > affect refresh; installed in `InitGameObject` beside the money hook).
  > `MObject` moves too, its two screen-rectangle members split to
  > `MObjectScreen.cpp` (they read the draw interpolation state);
  > `UserInformation`, `ClientConfig` and `MTimeItemManager` are pure
  > loaders and join rather than be hooked; `gamemodel` links `framelib`
  > for `CAnimationFrame`'s constructor. R4's subtraction became
  > library-wide (35 → 28, a reclassification, see the ratchet table);
  > R1 515 → 512. Tests (`test_item_core.cpp`, 14): construction, the
  > option list, the requirement math over both tables, quest detection
  > by flag and by register, the colour cycles under a hand-driven host,
  > the drop animation and its parking, the teen-build skull naming, the
  > pet refresh through the host. **Three defects the tests surfaced,
  > fixed test-first after the move:** `IsQuestItem` tested the item's
  > own flag only inside the condition that the timed-item register
  > exists; the requirement getters returned `BYTE` while the slayer
  > ceiling is 295 and Ousters gear is uncapped (295 read back as 39,
  > 300 as 44 - a level-150 item with strong options looked easy to
  > equip); and `ITEMOPTION_INFO` had an empty constructor, so the none
  > row an unoptioned item reads its colour from was allocation fill
  > (the `ITEMTABLE_INFO` shape again). The 3.1 commit-msg hook landed
  > with these fixes. Suite: 216 tests (3,513 checks). **4.3 is
  > unblocked**: the containers can now link against the item core; the
  > container-based item classes and `MItemManager.cpp` (it assigns
  > the arms-band and quick-slot globals) move with them. Still
  > executable-side of the item family: the use handlers (by design -
  > they are the packet/dialog side of items), `MItemFinder`,
  > `MItemManager.cpp`, the two container managers' `.cpp` files.
  > **Adversarial review round (2026-09-02, 2 reviewers; one NO-SHIP,
  > one SHIP), fixed on the branch:** the hook was committed 100644 (a
  > POSIX clone skips a non-executable hook) and its match took any
  > word starting with the phrase — now 100755 and anchored; the R4
  > note said 35 − 7 with five `VS_UI` files where the truth is 35 + 1 −
  > 8 with six (`MItem.cpp` joined counting under the old rule and left
  > under the new); the file headers claimed no class is split across
  > the halves, but `MBomb` and `MHolyWater` have `GetMaxNumber` in the
  > core and `NewItem` in the executable, and `MItem::NewItem` is
  > executable-side — harmless because the core constructs none of
  > them (no vtable emitted there), and the headers now say that;
  > the option-row constructor's comment named a wrong exposure (the
  > loader fills every row it sizes — the real ones are a table sized
  > but never loaded, as a test does, and a load that returns early);
  > and the quest-flag fix's message credits `InitGameObject` with
  > creating the timed-item register when `InitInfomation` does, earlier
  > — and recreates it after a `SAFE_DELETE`, a window in which the old
  > code ignored the flag. Confirmed by the round: every moved chunk
  > byte-identical (196 of 201 bodies; the five that differ are the
  > two host rewrites and the three colour-set clock reads), no
  > pre-init item construction, drop or colour read (every `NewItem`
  > caller is in-game), the widened requirement getters safe at all 64
  > call sites, each fix's test failing before its fix.
  > **Third slice (2026-09-03, `restructuring/gamemodel-gear`; live
  > verification gates the merge):** the gear. `MPlayerGear` (the slot
  > container a player wears its items in, grading each piece by its
  > remaining durability against the configuration's two thresholds),
  > `MSlayerGear`, `MVampireGear` and `MOustersGear` (the races' slot
  > rules over it: two-handers, ring-and-zap pairs, the belt and the
  > arms bands that become quick slots) and `MShop` (a shelf per type)
  > join `gamemodel`. Their reaches were the player's affect check and
  > stat recalculation, the gear sound, the UI's quick-slot rebuild, the
  > repair help event and — for a gun worn without a magazine — the item
  > factory. `MItemHost` gains four entries (`RecalculateStatus`,
  > `ResetQuickItemSlot`, `RepairHint`, `EmptyMagazineFor`, the last
  > carrying the magazine-fitting loop into `GameInit`); the affect
  > check and the sound go through the entries the containers already
  > use; the quick-slot globals are `MQuickSlot.cpp`'s, in the library
  > since 4.3. Dropped on the way: the 61 `__GAME_CLIENT__` guards
  > (always on) with the `UIFunction.h`, `ClientFunction.h` and
  > `MPlayer.h` includes they wrapped, the two plain includes the strip
  > left unused (`MPlayerGear.cpp`'s `DebugInfo.h` and
  > `MHelpManager.h`), and the two `#else` thresholds a non-client build
  > would have used. One behaviour delta, kept and recorded: the host's
  > `RecalculateStatus` checks for a player where the ~40 call sites it
  > replaced dereferenced one unguarded, so the no-player window is a
  > skipped recompute rather than a crash — the same trade the
  > containers' affect check made in 4.3. R1 502 → 497; R4 28 → 27 — a reclassification, not a cut:
  > `VS_UI_Game.cpp`'s only reaches past the libraries were the three
  > race gears' globals, library-defined now. Tests
  > (`test_player_gear.cpp`, 11): the base grades by durability at the
  > configured thresholds, treats a piece with no durability or one
  > that never wears as fine, clears a removed item's status and the
  > broken count, hands back what `Replace` displaces without grading
  > the newcomer (the race gears do), clamps `ModifyDurability` and
  > raises the repair hint once each time a piece leaves OK, and owns
  > its items; the slayer's gear wears a slayer item in its slot and
  > refuses the taken slot, the wrong slot, another race's item and
  > non-gear, tells the host on the way on and off, takes both hands
  > for a two-hander and frees both, makes a belt the quick slot and
  > resets it, seats a zap behind its ring and takes it off first; the
  > vampire's refuses a slayer's helm; the Ousters' arms band becomes a
  > quick slot; the shop asks the player about everything on the shelf
  > it switches to and owns its shelves. **Observed, not fixed:** the
  > Ousters gear plays the gear sound and recomputes the stats twice
  > for every item that goes on through its slot table (the two-hand
  > and stone branches call each once). **One defect fixed test-first,
  > after the move:** `CheckItemStatus` read the item's maximum
  > durability into the unsigned duration type, so the -1 `MItem`
  > answers for a piece with none failed the "no durability" test as
  > 4,294,967,295 and the piece graded almost broken (0 of a huge
  > maximum); the maximum is read signed now. Suite: 261 tests (4,164
  > checks).
  > **Adversarial review round (2026-09-03, 2 reviewers, both SHIP with
  > findings), fixed on the branch:** the round overturned that fix's
  > premise and found the reachable defect underneath it. `MGearItem`
  > clamps its maximum to at least 1,000 and every race gear admits
  > only `IsGearItem()` items, so no live piece of gear can report a
  > negative maximum — the test reaches that path only through an
  > override, which makes it a **regression guard**, not a
  > reproduction, and the claim that "a gear entry whose table maximum
  > is unset shows red" was false. The symptom was real but had another
  > cause, one MSVC had been printing as C4018 in this slice's own
  > build log: `itemStatusPer` was unsigned while both thresholds are
  > `int` read unchecked from the configuration file, so a negative
  > threshold graded **every** worn piece as almost broken. Both
  > comparisons are signed now, the percentage is computed in 64 bits,
  > and `ModifyDurability` no longer clamps to a negative maximum
  > (which stored 4,294,967,295 as the new durability).
  > **Three more defects, each fixed test-first:** the zap branch of
  > `AddItem` in all three gears read `m_ItemSlot[n-m_Gilles_CoreZap]`
  > — a glove, belt or trouser slot for the slayer, a ring for the
  > Ousters — and required the ring slot itself to be *empty*, so it
  > both refused a zap over its own ring and **accepted a ring-less zap**
  > whenever the unrelated slot happened to be filled; `CanReplaceItem`
  > and `ReplaceItem` in the same files already wrote the intended
  > condition, which the branch now matches. `RemoveItem(GEAR_*)` in
  > all three gears indexed the slot array before any bound, with the
  > slot id taken straight from `GCRemoveFromGear` — the new test
  > crashes the process against the old code, the C13 shape again.
  > `MShop::SetShelf` wrote `m_pShelf[n]` unbounded, contradicting its
  > own header's promise that a shelf number past the shop's own is
  > refused and left to the caller to delete — three `VS_UI` call sites
  > are written against that promise. Tidy-ups: two tests were weak —
  > one leaked the item it said it handed back, and the "not gear"
  > refusal was really a race refusal because the fixture left the
  > potion's race unset; `EmptyMagazineFor` is typed `MMagazine*` and
  > checks the factory's NULL contract; `MItem`'s two older host
  > services guard the function pointer as the four new ones do;
  > `GameModelWorld` cannot be copied. **Noted, not done:** the five
  > gear and shop sources still hold 492 Korean comment lines (89 of
  > them inside the functions this slice touched) and their headers
  > 184 — translating them wholesale would swamp the byte-identity the
  > move rests on, so it is debt with a number; and `MSlayerGear::ReplaceItem`,
  > `GetFitSlot`, the PDA, shoulder and blood-bible slots, the Ousters
  > stones and `MPlayerGear::ReplaceItem` over a broken newcomer have
  > no tests. Suite: 264 tests (4,196 checks).
  > **Fourth slice (2026-09-03, `restructuring/gamemodel-skills`; live
  > verification gates the merge):** the skill core, which completes
  > 4.4. `MSkillManager.cpp` splits the way `MItem.cpp` did: the info
  > table one entry per skill (`MSkillInfoTable.cpp`), the set of skills
  > a character may use, the domains and the tree they hold, and
  > `SkillDef.cpp`'s domain names join `gamemodel`; the three methods
  > that ask what the player can use *right now* —
  > `MSkillSet::SetAvailableSkills`, its vampire counterpart and
  > `CheckMP`, about 1,200 lines and every reach to `g_pPlayer`,
  > `g_pInventory`, `g_pZone`, the three gears and the war-bonus arrays
  > — move whole to the executable's new `MSkillAvailable.cpp`, which
  > also defines the two bonus arrays six other executable files read.
  > `MSkillSet` has no virtuals, so splitting it across the two targets
  > costs no vtable. Two seams cut: the use delays read the item host's
  > clock (`MItem::Clock()`, the seam the trade manager already uses)
  > instead of `g_CurrentTime`, and without a clock there is no delay —
  > which is what the `#else` branch did for the two readers; the two
  > setters were no-ops without the macro and now write against a zero
  > clock, inert while there is none;
  > and `MSkillManager::Init` no longer opens the domain-experience
  > file, because `FileOpenBinary` and `g_pFileDef` are the
  > executable's — `InitSkillTree` in `GameInit.cpp` builds the tree
  > and then feeds `LoadFromFileServerDomainInfo` a stream, the shape
  > task 4.1 gave `SystemAvailabilities`, and the three live call sites
  > (`GameInit`, `GameMain`, `GCUpdateInfoHandler`) call it — a fourth
  > in `VS_UI/WinMain.cpp` is in no target and opens the file itself.
  > The 17 always-on `__GAME_CLIENT__` guard lines (8 blocks) go with
  > the includes they wrapped; three more blocks moved with the
  > methods. R1 497 → 495; R4 27 → 25 — a reclassification again:
  > `VS_UI_SKILL_VIEW.cpp` and `VS_UI_skill_tree.cpp` reached past the
  > libraries only for `g_pSkillInfoTable` and `g_pSkillManager`, which
  > `MSkillManager.cpp` defines. Tests (`test_skill_core.cpp`, 7): a skill's use delay over
  > the host clock, including no clock meaning no delay; the next-skill
  > list sorting by id and refusing a duplicate; the server-info round
  > trip, both with and without the Ousters elemental block the format
  > carries only for that domain; the skill set holding, enabling,
  > disabling and removing by id; a domain pulling in the chain under
  > its root skill with the root learnable and the rest not; learning
  > walking down that chain into the usable set and unlearning walking
  > back up; and `Init` giving all eight domains their root skill.
  > Suite: 271 tests (4,278 checks).
  > **Adversarial review round (2026-09-03, 2 reviewers, both SHIP with
  > findings), fixed on the branch:** the structural half held — the
  > 1,203-line move is byte-identical (md5), the residual diff is
  > accounted for line by line, both seams are behaviour-preserving for
  > every build that exists — and what the round found was two library
  > defects the move made reachable, plus test and prose repairs. **Two
  > defects fixed test-first:** `SKILLINFO_NODE`'s constructor set every
  > field but three — `m_SkillStep`, `m_X` and `m_Y` — and
  > `MSkillDomain::AddSkill` *branches* on the step, so a table entry no
  > file had filled decided a skill's step from whatever the memory held
  > (the client loads `SkillInfo.inf` first, which is why it never
  > showed; neither `/RTC1`, which fills locals only, nor ASan, which
  > has no uninitialised-member check, could flag it, and the new test
  > reads `0xCCCCCCCC` and `0xCDCDCDCD` against the old code); and
  > `MSkillManager::LoadFromFileServerDomainInfo` indexed its eight-row
  > table with an `int` read straight from the file, through the raw
  > pointer, past even the typed table's own bound — the C13 shape, now
  > library code, and the new test segfaults the runner against the old
  > code. **Four checks passed for the wrong reason** and are repaired:
  > three refusals in the learn test stopped at `LearnSkill`'s first
  > gate, because the successful learn had consumed the skill point, so
  > they would have survived gutting the function; and a `DomainType`
  > round trip compared against `SKILLDOMAIN_BLADE`, which is 0, the
  > constructor's own value. Prose corrections: "17 guards" is 17 guard
  > *lines* over 8 blocks; `g_pSkillAvailable` was named as an R4 reach
  > neither VS_UI file makes; the `#else` equivalence holds for the two
  > delay readers but not the two setters; the 4.4 status line cites the
  > slice's commit; the `MItemHost` clock comment names the skill delays
  > too; and the R4 comment now says what that ratchet cannot see — a
  > library file calling an executable-side *function*
  > (`VS_UI_GameCommon.cpp` calls `SetAvailableSkills`) is a seam the
  > `g_p*` pattern never counted. **Noted, not done:**
  > `MSkillDomain::Clear` can never take its `m_pLearnedSkillID!=NULL`
  > branch, because `ClearSkillList` nulls the pointer first, so the
  > level counters go stale; `RemoveNextSkill` dereferences a `find`
  > without checking for the end; and `InitSkillList`, `AddSkillStep`,
  > the domain save/load and `GetExpInfo` are library-testable now and
  > untested. Suite: 273 tests (4,301 checks).
  > **Fifth slice (2026-09-03, `fix/skill-domain-state`):** that debt,
  > as a fix-and-test slice over the domain itself — no file moves, so
  > R1 stays 495, R4 stays 25 and the membership file is untouched.
  > **The defect the debt named, reproduced:** `MSkillDomain::Clear`
  > could never take its `m_pLearnedSkillID!=NULL` branch, because
  > `ClearSkillList` frees and nulls that pointer first — so the branch
  > that also reset `m_MaxLevel` and `m_MaxLearnedLevel` never ran and
  > the two counters kept naming a level of an array that was gone.
  > `LoadFromFile` is the one caller: it clears, then *refills* the
  > skill list, leaving a domain whose skills are LEARNED, whose
  > learned-level array is NULL and whose top learned level is
  > whatever it was before — and the next `UnLearnSkill` indexes that
  > NULL array. The new round-trip test segfaults the runner against
  > the old code (five failed checks before it, from the fresh-domain
  > half). The counters are now reset unconditionally, and the load
  > rebuilds what the file cannot carry: the step lists and the
  > learned-level array follow from the skill list and the info table,
  > so `SetStateFromSkillList` derives both, and the learned skills go
  > back into the usable set that `ClearSkillList` took them out of.
  > Deriving the step lists took a second fix, which the review round
  > below caught: nothing but the destructor ever cleared
  > `m_mapSkillStep`, so a rebuild *added* to lists still naming the
  > tree before it. `ClearSkillList` frees them now, through a new
  > `ClearSkillStep` — which is what the skill-info packet's
  > `InitSkillList` needed too; its lists came out right only because
  > the eight trees are the same on every login.
  > The loader also checks its reads — a count with no rows behind it
  > used to insert the two uninitialised locals as a skill, and a
  > count past the end of the file to insert the last row over and
  > over — and refuses a status the enum does not have.
  > **A second defect, reproduced:** `LearnSkill` wrote
  > `m_pLearnedSkillID[skillLevel]` with the level straight from the
  > skill file into an array as long as the deepest skill the tree
  > walk counted. The two agree in the client today only because the
  > file is read once before the trees are built; they are one table
  > apart, and the new test — a skill whose level moves after the walk
  > — drives a write nine entries into an array of three. Refused
  > now, and refused *before* the skill is added to the usable set, so
  > a refusal leaves nothing behind; `UnLearnSkill` got the matching
  > guard. Worth recording: ASan did not flag that write in the
  > sanitized tree, so what pins the fix is the refusal the test
  > asserts, not a crash — the "assert the observable contract"
  > rule in `CLAUDE.md`, again.
  > **Three regression guards**, called that because none could be
  > reached: `RemoveNextSkill` dereferenced the result of a `find`
  > without testing for the end — and a map answers a lookup with the
  > key that was looked up, so the lookup could only ever return the
  > id it was given; it is gone and the id is used directly.
  > `MSkillDomain::LoadFromFileServerDomainInfo` read a level out of
  > the file into an uninitialised local and indexed the 151-row
  > experience table with it; `CTypeTable::operator[]`'s range test
  > (in force in every build since an earlier slice) already sent that
  > store to the one out-of-range row it shares across every
  > experience table, and that row is not the one a read gets back, so
  > the new checks pass against the old loader too — what they pin is
  > the contract. And the three `g_pSkillAvailable` dereferences in
  > the domain are NULL-guarded (`GameInit.cpp` deletes that global at
  > shutdown, so the window is real), as is `GetSkillStepFor`'s reach
  > for `g_pSkillManager`. The `m_pLearnedSkillID==NULL` limb of the
  > two new bounds guards is a guard of the same kind: after the fix
  > nothing can reach it, because the array is absent only while the
  > skill list is empty too, and every entry point gates on the list
  > first.
  > Two refactors carried along, both verified identical by the review:
  > `AddSkillStep` searched a *copy* of the step list it was about to
  > append to, once per skill added, and now searches the list; and the
  > "which step does the skill all three races share belong to"
  > decision moved out of `AddSkill` into `GetSkillStepFor`, which the
  > rebuild path needs as well — the new test pins all three of its
  > answers. `AddNextSkill` gained the `MAX_ACTIONINFO` test its two
  > siblings already had; it had been leaning on the info table
  > answering an index it does not hold with an empty entry. The 37
  > Korean comment lines inside the four functions this slice edits
  > (`AddSkill`, `RemoveNextSkill`, `LearnSkill`, `UnLearnSkill`) are
  > translated — the first round of this slice translated only the
  > lines next to the edits and claimed all of them, which the review
  > caught; `SetRootSkill` and the rest of the file are untouched and
  > still Korean.
  > Tests (`test_skill_core.cpp`, +10, 9 → 19 in the file): the step
  > lists ordered by learn level and holding each skill once; the
  > save/load round trip, over a fresh domain and over a live one, and
  > a file that marks a deep skill learned and its parent not; a save
  > file that ends early, carries a count of a thousand, a negative
  > count, or an impossible status; an experience level past the table
  > and a negative one; `InitSkillList` rebuilding the eight trees
  > while keeping the domain levels the server sent once; the
  > deeper-than-the-array learn refusal; the shared skill's step per
  > domain; a load over a live domain leaving none of the old tree in
  > the step lists; an experience row that cannot be stored costing
  > only itself; and the domain running with both executable-owned
  > globals set to NULL. Three of those — the step-list ordering, the
  > `InitSkillList` rebuild and the shared skill's step — pass in full
  > against the unfixed code: they are coverage for the two refactors,
  > not reproductions. **Noted, not done:** `MSkillDomain::SaveToFile` and
  > `LoadFromFile` have no caller anywhere in the tree — they are
  > reachable only through `CTypeTable`'s own file I/O, one call away,
  > which is why they were made correct rather than deleted; deleting
  > them belongs to 5.2. `SKILLDOMAIN` (the model's, in `SkillDef.h`)
  > and `SkillDomain` (the wire's, in `Packet/Types/CreatureTypes.h`)
  > are the same eight names in the same order, and `MSkillManager.cpp`
  > indexes itself with both spellings. `LearnSkill`'s "is this the
  > level being learned next" gate is commented out, so any level in
  > the domain can be learned in any order. A skill whose level the
  > file gives as negative is now refused rather than written below
  > the array *and marked learned*, which is strictly safer but would
  > make such a skill vanish from the window — no shipped data to
  > check that against here. Suite: 283 tests (4,401 checks).
  > **Adversarial review round (2026-09-03, 2 reviewers, both SHIP
  > with findings), fixed on the branch:** both reviewers independently
  > found the same real defect — the experience-level refusal returned
  > without consuming the row, and the manager's loop reads the next
  > domain from wherever the stream is left, so one bad level cost the
  > rest of the file where the old code lost only that row. The row is
  > read into a local now and stored only if the level fits, so the
  > stream stays where it always was. They also both caught the step
  > lists never being cleared, and the behavioural reviewer verified
  > line by line that the other seven changes are behaviour-preserving
  > on every reachable path — including that `LearnSkill`'s bound
  > cannot refuse a learn that used to succeed, because `AddSkill`
  > raises `m_MaxLevel` before its duplicate test and the level never
  > decreases. The test reviewer's findings account for the guard
  > labels, the coverage admissions and the corrected counts above,
  > and for two test comments that described contracts the code does
  > not have.
  - Owner (all of 4.x): `gamemodel`'s membership file
    (`tests/arch/gamemodel_files.txt`), the M0–M2 include rules in
    `check_includes.pl` (in force since 4.1), R4 shrinking.

---

## Phase 5 — Long tail

- [ ] **5.1 Split the debug facilities.** A lib-safe logging header (the
  `DebugLog.h` system already exists — finish the migration for lib code)
  so `DebugInfo.h`/`MinTr.h` stop gating membership. The task named
  `SocketAPI.cpp`, `DatagramSocket.cpp` and `NPCInfo.cpp`; by the time it
  came up the first and third were already packetwire members, and the
  real list was `tests/arch/packetwire_holdouts.txt`.
  > **Status:** first slice done (2026-09-03,
  > `restructuring/lib-safe-debug`; live verification gates the merge).
  > **What actually gated the wire layer** was one line: `DebugInfo.h`
  > opens with an unconditional `#include "MinTr.h"` on Windows, above
  > its own include guard, and every one of the eight holdouts included
  > `DebugInfo.h`. Past that line the header is nothing — the block that
  > would define `OUTPUT_DEBUG` is commented out, so the `#else` branch
  > is what every build takes, and it is `#include "DebugLog.h"` plus two
  > no-op macros. Not one holdout uses `DEBUG_MESSAGE`, `DEBUG_CMD` or
  > `DEBUG_NEW`; all eight use only the `DEBUG_ADD` family, which
  > `DebugLog.h` supplies. So the eight now include `DebugLog.h`
  > directly.
  > **The logging facility moved to `basic/`.** It was
  > `Client/DebugLog.cpp/.h` and a *gamemodel* member, which put it
  > above packetwire in the link order and outside W1's allowed
  > prefixes. It needs nothing but the C library and `Platform.h`, so
  > the bottom library is where it belongs: every target links `basic`
  > and that directory is a PUBLIC include directory, so not one of the
  > fifteen `#include "DebugLog.h"` lines had to change - fourteen in sources plus the one in `DebugInfo.h` itself. That also
  > ended a *second* copy of it: `Client/SpriteLib/CMakeLists.txt`
  > compiled `../DebugLog.cpp` into `SpriteLib` as well, so the same
  > translation unit was in two libraries; `SpriteLib` links `basic`
  > now and there is one object again. Two more executable-side headers
  > left the closure with it: `DebugKit.h` from `ClientPlayer.cpp`,
  > whose only user is a block inside a `/* */` comment (with four
  > externs that went with it), and `MTestDef.h` from two files, whose
  > one struct sits behind a `__METROTECH_TEST__` nothing defines.
  > **Two holdouts became members:** `Player.cpp` (the send/receive
  > plumbing under all three player classes) and `DatagramSocket.cpp`. R1
  > 495 → 493; gamemodel membership 35 → 34 files, packetwire 517 → 519.
  > The other six reach for game headers, and the holdouts file now
  > lists which ones each still needs — `ClientConfig.h` is common to
  > all six and is the next thing to cut.
  > **The link proof earned its keep.** `arch_includes` passed on the
  > new membership, and then the test binary failed to link:
  > `Player::processCommand` calls `SendBugReport`, which lives in the
  > executable's `PacketFunction.cpp`. A library file calling an
  > executable-side *function* is the seam neither W1/W2 (which read
  > includes) nor R4 (which greps `g_p*`) can see — the 4.4 record made
  > that observation and this is the first time it cost anything. It is
  > stubbed in `tests/stubs/client_globals.cpp` for now, with the real
  > fix recorded: `SendBugReport` builds a `CGSay` and sends it through
  > `g_pSocket`, so its only reach outside the wire layer is that one
  > global, and it can move into packetwire behind a `Player*`.
  > **A defect fixed test-first, which the review round turned up.**
  > `Player`'s socket constructor — the one `RequestClientPlayer` and
  > `RequestServerPlayer` both forward to — set every member but
  > `pHashTable`, which the default constructor's body was the only
  > place that ever set. `delKey()` `delete[]`s that pointer, and the
  > first draft of this record claimed nothing called `delKey`: it has
  > **two live callers**, `GCReconnectLoginHandler.cpp:72` and
  > `LCReconnectHandler.cpp:178`, both on every reconnect. They are
  > safe today only because `ClientPlayer` — the class those two cast
  > to — writes `: m_PlayerStatus(CPS_NONE)` and so default-constructs
  > its base; a base initialiser on that one class would have made
  > every reconnect a free of whatever the memory held. Both
  > constructors set it in their initialiser list now, the destructor
  > frees it (nothing did, so a player that had been given a key leaked
  > 512 bytes), and `setKey` frees the previous table before building
  > another. The new test drives the socket constructor and the ASan
  > tree aborts against the old code: *"AddressSanitizer: bad-free …
  > in Player::delKey"*.
  > Tests (`test_player_base.cpp`, 5): the default constructor holding
  > no socket and no key; the encryption table following the hash key
  > modulo 256 (all it depends on — the encrypt key only picks an
  > offset) and being given back, with a second `delKey` harmless;
  > `setSocket` keeping the socket it is given; the socket constructor
  > leaving no key to give back, and a second `setKey` replacing the
  > table; and a link proof for `DatagramSocket.cpp`, which — unlike
  > `Player.cpp`, whose link genuinely fired — nothing in the test
  > binary would otherwise reference, so a static library would never
  > hand its object to the linker at all. Suite: 288 tests (4,426
  > checks).
  > **A ratchet for the seam, R6.** Stubbing `SendBugReport` for the
  > test binary silences the only detector that found it, so R6 counts
  > packetwire members that call it: baseline 1, `Player.cpp`, and it
  > leaves when `SendBugReport` moves into the library.
  > **Noted, not done:** `SocketImpl`'s default constructor is the only
  > one of its four that leaves `m_key` unset. `Player::processCommand`,
  > `processInput`, `processOutput`, `sendPacket`, `disconnect` and
  > `toString` all dereference the socket or a stream without testing
  > either, which the default constructor leaves NULL.
  > **Second slice (2026-09-03, `restructuring/wire-config-seam`):** the
  > `ClientConfig.h` the first slice named as the next thing to cut. It
  > was **three integers** across all six holdouts — `MAX_PROCESS_PACKET`
  > in four of them, `MAX_REQUEST_SERVICE` in two,
  > `CLIENT_COMMUNICATION_UDP_PORT` in one — so it takes the shape
  > `MItemHost` and `MPriceHost` established: `Client/Packet/WireHost.h`
  > declares what the wire layer asks of the program around it, the
  > executable fills one in beside the other two hosts in `GameInit`,
  > and every accessor answers without a host with the value
  > `ClientConfig`'s own constructor sets. A library with no host
  > behaves as a client whose config file is missing, not as one tuned
  > to zero — a receive loop handling 0 packets a turn would take none
  > at all.
  > **`SendBugReport` moved into the wire layer with it,** which is the
  > fix the first slice recorded. It only ever needed a `CGSay` and
  > somewhere to send it, so it is `WireHost.cpp`'s second half, taking
  > its target from the host; the definition leaves
  > `Client/PacketFunction.cpp` and every caller, in the library and in
  > the executable, resolves to the one in `packetwire`. The stub in
  > `tests/stubs/client_globals.cpp` is gone with it, which brings back
  > the failed-link detector the first slice had to disable — narrower
  > than the ratchet that stood in for it, since a link only catches a
  > call in a library `unit_tests` links and only in an object some
  > test pulls in.
  > **The round trip this made, undone.** `PacketDiagnostics` — the
  > hook task 2.4 added so `Datagram::read` could report without
  > linking against the executable — was left pointing at a forwarder
  > in `PacketHandlerRegistry.cpp` that did nothing but call the
  > library's `SendBugReport`. The library was calling out to the
  > executable to call straight back in. `reportBug` goes to it
  > directly now when no hook is installed, the forwarder and its
  > installation are deleted, and the hook stays as what it is worth
  > being: an interception point, which is how a test captures the
  > text. Both delivery paths for the same "too large PacketSize"
  > report — `Datagram.cpp`'s and `Player.cpp`'s — are armed by the
  > same `Wire::SetHost` now instead of by two mechanisms at two
  > different times.
  > **R6 retired by doing its job.** It was added last slice to replace
  > that disabled detector, and it fired on the first thing this slice
  > did: promoting `ClientCommunicationManager.cpp` took the count of
  > members calling `SendBugReport` to 2, which is what said "move the
  > function, don't grow the seam". With the function in the library
  > the ratchet measures a symbol nothing is on the wrong side of, so
  > it is deleted with that history in its place.
  > **R4 caught a comment, not a seam.** It greps every line of a
  > library source for `g_p*` and does not skip comments, so
  > `WireHost.cpp`'s note about which global the report used to go
  > through counted as a reach and took the measurement to 26. The
  > comment says it without the name now. What that shows is that R4
  > can read a *false* reach — its number is partly a function of
  > comment wording. It misses real ones too, which its own comment in
  > `ratchets.sh` has said since 4.4: the pattern is `g_p*` only, so a
  > library file calling an executable-side function is invisible to it
  > (`SendBugReport`, the subject of this very slice, was exactly that),
  > and so is a global under any other name — `g_Mode`, which four of
  > the remaining holdouts branch on, is one.
  > `ClientCommunicationManager.cpp` is a member. R1 493 → 492;
  > packetwire 519 → 521 files. Five holdouts remain, and what they
  > need is game state rather than tuning — `g_pGameMessage`,
  > `g_pPlayer`, `g_pUserInformation`, `g_pRequestFileManager`,
  > `g_pProfileManager`, `g_pWhisperManager`, `g_pRequestUserManager`,
  > `g_pGameStringTable`, and `g_Mode` with the `MODE_*` enum that four
  > of them branch on. The holdouts file lists it per file; a host the
  > size of `WireHost` would not cover it, so the request-service
  > family is its own slice.
  > Tests (`test_wire_host.cpp`, 5): the defaults without a host, and
  > again with a host whose entries are all NULL — the accessors test
  > the pointer, not just the host; a host answering, read each time
  > rather than copied once, and put back by `SetHost(NULL)`; which
  > reports `SendBugReport` builds and which it drops; a report from
  > inside the library reaching the reporter with no hook installed;
  > and a link proof for `ClientCommunicationManager.cpp`.
  > The bug-report test is the one the review round fixed: it ran six
  > inputs through the function and asserted only that there was no
  > target, which is true by construction — the function was
  > gutted-proof. `SendBugReport` asks the host for a target *after*
  > deciding a report is worth sending, so a host that counts the asks
  > says which of NULL, `""`, `"x"`, a two-character report, a
  > formatted one and one longer than its 256-byte buffer get that far
  > — an observable contract, with no `Player` needed.
  > **Untested, and it cannot be otherwise:** the host installation in
  > `GameInit.cpp` is executable-side, so nothing here proves the
  > client installs it, nor that the UDP port a live client binds comes
  > from the config rather than the default. Running the client is the
  > check. Its three fallbacks name the same defaults the library uses,
  > through `WIRE_DEFAULT_*` in `WireHost.h` rather than a second copy
  > of the numbers, so at least the two cannot drift.
  > Suite: 293 tests (4,449 checks).
  >
  > **Third slice (2026-09-04): `ClientPlayer.cpp`, and a holdout list
  > that was wrong about the other four.** Its last two game-code
  > includes were `MZone.h` and `UserInformation.h`, and both existed
  > for one function: `setEncryptCode()`, which reads the zone id, the
  > account's server number and three region flags to derive the stream
  > cipher's seed. All three are behind `WireHost` now, so R1 493 → 492
  > and packetwire 521 → 522 files.
  >
  > **Read that carefully.** Nothing in this repository or its build
  > ever defines `__USE_ENCRYPTER__`, so the encrypted socket streams
  > are never constructed and `setEncryptCode()`'s body is not
  > compiled. Moving its inputs behind the host proves nothing at
  > runtime; what it does is make the dead branch expressible without
  > game headers, which was the only thing keeping the file out. The
  > seed itself is extracted as `WireEncryptSeed` and pinned by tests,
  > because if the encrypter is ever turned back on those are the bytes
  > the server has to agree with. Its four original branches -
  > Netmarble, Chinese, English, default - computed only **two** distinct
  > expressions; the three non-English ones were identical, which a test
  > asserts across every server number a byte can hold rather than
  > leaving to a comment.
  >
  > **The first version of the link proof proved nothing**, and it is
  > the lesson worth carrying: it took the address of `processCommand`
  > and `disconnect`, both **virtual**. A pointer to a virtual member is
  > a vtable index and need not reference the defining object, so the
  > test linked cleanly with `ClientPlayer.cpp` taken back out of
  > `packetwire_files.txt` — verified, not reasoned. With two
  > non-virtual members it fails to link, also verified. The two older
  > link proofs (`ClientCommunicationManager`, `DatagramSocket`) were
  > checked for the same trap and are sound; neither class declares a
  > virtual.
  >
  > **A correction to the holdouts file** while the entry was being
  > removed. It said four of the five branch on `g_Mode`; only **two**
  > do. `RequestServerPlayer.cpp` has the test commented out and
  > `RequestServerPlayerManager.cpp` reads `if (1)//g_Mode==MODE_GAME)`.
  > Whoever takes the request-service slice should find out what those
  > two actually want `ClientDef.h` for rather than build an accessor
  > for four callers that are two. Also recorded there: of the four
  > managers the family reaches, `RequestUserManager.cpp` and
  > `UserInformation.cpp` include nothing but `Client_PCH.h` and their
  > own headers, while `ProfileManager.cpp` pulls `CDirectDraw.h`,
  > `CSpritePack.h` and `UIFunction.h` and is staying where it is.
  >
  > Tests (`test_wire_host.cpp`, +4): the three new host entries with no
  > host and with an empty host, a host answering them and re-read each
  > time, the seed against the expression it came from, the
  > three-regions-agree property, and the link proof.
  > Suite: 342 tests (4,819 checks).
- [ ] **5.2 Dead/duplicate source removal** (code-health priority 3): the
  `_bak` files are already excluded by the build — delete them; sort the
  `GameHelpers`/`GameFunctions`/`GamePacketFunctions` exclusion graveyard
  into deleted-or-documented. The wrong-file-edited trap dies here.
  > **Status:** four slices done; the graveyard the task names is
  > deleted (fourth slice below), so the wrong-file-edited trap is
  > closed for it. First slice — the item-table twins (2026-09-02,
  > `restructuring/dead-item-table-twins`). Deleted: the one tracked
  > `_bak` file (`MItemTable_bak-2007-5-7.cpp`, 15,000 lines of a 2007
  > item table, excluded by the build's `_bak` filter); `MitemTableinit2.cpp`
  > (a second `InitItem2`, excluded by name); and `VS_UI/MitemTableInit.cpp`
  > (a third, which the Windows `VS_UI` target was still compiling
  > because the filter meant to drop it sat in the non-Windows branch —
  > `VS_UI.lib` carried a twin the executable's object outranked). The
  > surviving `Client/MitemTableInit.cpp` no longer ends by calling
  > itself. All of it dead in this build: `InitItem2` is reached only
  > from `ITEMCLASS_TABLE`'s server-only constructor, which is the next
  > 5.2 candidate — the 23,000-line `__INIT_ITEM__` block in
  > `MItemTable.cpp` plus the 8,500-line `MitemTableInit.cpp` are the
  > server's item data in client source. The exclusion graveyard proper
  > (`GameHelpers`, `GameFunctions`, `GamePacketFunctions`,
  > `ActionFunctions`, `MissingGlobals`, `GlobalVariables`) is untouched.
  > Verified: both executable trees 0 errors; R1 unchanged (nothing
  > deleted was compiled into the executable); ctest 4/4 in both test
  > trees.
  > **Second slice (2026-09-02, `restructuring/drop-server-item-data`):**
  > the server's item data itself. The 23,000-line `__INIT_ITEM__` block
  > in `ITEMCLASS_TABLE`'s constructor and the 8,500-line
  > `MitemTableInit.cpp` (`InitItem2`, its only caller being that block)
  > are deleted, with the `__INIT_ITEM__` define, the `InitItem2`
  > declaration and the three includes the block alone used
  > (`AddonDef.h` leaves the membership file). Both were compiled out
  > or unreachable in every client configuration, so the linked program
  > is unchanged; the client has loaded `Item.inf` all along. Git
  > history keeps the data if a data-authoring path ever wants it.
  > `MItemTable.cpp` goes from 23,907 lines to under 400. R1 516 → 515
  > (`MitemTableInit.cpp` was compiled into the executable, dead). Review: one
  > reviewer, SHIP, no findings; it named the next candidate —
  > `MSkillInfoTable.cpp` carries ~2,300 lines of in-code skill
  > definitions under `#ifndef __GAME_CLIENT__` (lines 32–2348), the same
  > server-data pattern, executable-side so with no test path.
> **Third slice (2026-09-02, `restructuring/drop-server-skill-data`):**
> that skill data. The 2,300-line `#ifndef __GAME_CLIENT__` block in
> `MSkillInfoTable`'s constructor and the `__INIT_INFO__`-guarded include
> only it used are deleted; only `DarkEden` compiles the file and it
> defines `__GAME_CLIENT__=1` (`Client_PCH.h` defines it unconditionally
> besides), so the constructor did, and does, exactly
> `Init(MIN_RESULT_ACTIONINFO)` then `Init()`. The client fills the table
> from `SkillInfo.inf`; the level-150 additions after the load stay.
> 2,621 lines → 304; R1 unchanged (the file is still compiled). Review:
> two reviewers, both SHIP, prose only (the line count, the header
> comment's account of what the file supplies, the commit message's list
> of targets defining the macro) — fixed. Two observations recorded for
> later slices: the deletion removed the last callers of
> `SKILLINFO_NODE::SetPassive` and `AddNextSkill`, so `IsPassive()` (six
> reads in `MPlayer.cpp`) and `GetNextSkillList()` (four in
> `MSkillManager.cpp`) have always read defaults in the client; and
> `LoadFromFileServerSkillInfo` indexes `m_pTypeInfo` with the
> file-supplied skill type through the raw pointer, past
> `CTypeTable::operator[]`'s check (a code-health item, not a 5.2 one).
> Remaining `#ifndef __GAME_CLIENT__` residue outside `Client/Packet`:
> `RankBonusTable.cpp` (a save path), `MSectorInfo.h` (portal fields),
> `Updater/Update.cpp` and the two unbuilt `OtherClass/Request*PacketFactoryManager.cpp`
> files — none a data slab; the exclusion graveyard is still untouched.
> **Fourth slice (2026-09-03, `restructuring/dead-exclusion-graveyard`):**
> the graveyard itself, sorted the "deleted" way — all of it. Eight
> `.cpp` files and two headers go: `GlobalVariables.cpp` and
> `MissingGlobals.cpp` (globals `Client.cpp` and `GameMain.cpp` already
> define, plus a `timeGetTime` that would fight the `SDL_GetTicks`
> mapping in `basic/Platform.h`), `GameHelpers.cpp`/`.h` and
> `GameFunctions.cpp` (two graveyards of empty stubs with the same
> names, the second of them a `void*`-signature macOS port scaffold),
> `GamePacketFunctions.cpp` (1,200 lines superseded by
> `PacketFunction.cpp` and `Globals.cpp`), `ActionFunctions.cpp` (one
> stub of a function with four other definitions in the tree), and
> `GCNotifyWin.cpp`/`.h` plus `GCNotifyWinHandler.cpp` — pre-port
> copies of the live packet class in `Client/Packet/Gpackets/` and its
> handler in `Client/PacketHandler/`, still carrying the `execute()`
> that task 2.4 stripped, and CLAUDE.md's wrong-file-edited trap in its
> purest form: the stale handler has `UI_RunImageNotice` commented out
> as "not implemented in SDL backend" while the function is implemented
> at `GameUI.cpp:4238` and the live handler calls it.
> **Verified before deleting anything.** None of the eight appears as a
> `ClCompile` entry in any of the 70 generated project files across the
> four build trees — the same scan finds `Client/Globals.cpp`, which is
> compiled, in four of them — so no target compiled them and their
> objects were never handed to a linker. And a scan of all 1,103
> compiled translation units found that every one of the 171 function
> definitions the six function-carrying files held is *also* defined by
> a file the build compiles (`PacketFunction.cpp`, `GameMain.cpp`,
> `RenderingFunctions.cpp`, `MTopView.cpp`, `SDLMain.cpp`,
> `MItemUse.cpp`, `VS_UI_GameCommon.cpp`): **none was unique**, so
> nothing in the graveyard was the only implementation of anything. The
> stale `GCNotifyWin` trio was diffed against its live counterparts
> line by line instead, being class members the scan does not match.
> The `list(REMOVE_ITEM)` block that named the eight one by one is gone
> with them, and four comments elsewhere that pointed at the deleted
> files are corrected — two of which were already false: `rectangle()`
> has one definition, not two (`GameInit.cpp`), and `g_BasicException`
> and `g_SetNewHandler` are defined only in `basic/BasicException.cpp`,
> never in `GameHelpers.cpp` (`RenderingFunctions.cpp`).
> **R1 unchanged at 495** — the direct proof that no translation unit
> left the executable. All four trees reconfigure and build at 0
> errors; ctest 4/4 in both test trees; 280 tests, 4,384 checks, 0
> failed. Closes the review's Medium dead-code finding ("roughly 400KB
> of stub and duplicate source"), which the first slice had half
> closed. **Noted, not done:** `Client/Client.cpp` and
> `Client/ClientFunction.cpp` have two `list(FILTER ...)` patterns
> anchored with `^Client/` that the glob's absolute paths can never
> match, already documented in place as dead patterns; and
> `VS_UI/WinMain.cpp` is in no target at all, which the 4.4 record also
> noticed — a candidate for the next 5.2 slice.
- [ ] **5.3 TextSystem stub retirement.** Split `TextService.cpp`'s pure
  text utilities from its `g_pLast` drawing entry point so
  `tests/stubs/client_globals.cpp` can shrink (the stub file itself
  documents this as the real fix).
  > **Status:** done (2026-09-03, `restructuring/textsystem-stub`; live
  > verification gates the merge). The split the task describes, done
  > as described. `TextService::RenderText` — a compatibility shim for
  > `SDL_RenderText` that draws white text through `g_pLast` — is the
  > only member of that class reaching a client global; everything else
  > draws through a `RenderTarget` the caller supplies, which is what
  > kept the rest of the library clean. Its definition moves to
  > `Client/TextServiceScreen.cpp`, which the executable compiles, with
  > the declaration left in the header where its ~45 call sites expect
  > it. `TextService` has no virtual members, so defining the class
  > across two targets costs no vtable — the same trade
  > `MItem`/`MItemUse` and `MSkillManager`/`MSkillAvailable` made.
  > **A wrong turn worth recording:** the first attempt deleted
  > `RenderText` outright, on the strength of a tree-wide grep that
  > found no callers. The grep was `grep -rn RenderText ... | grep -v
  > TextService`, and every call site is spelled
  > `TextSystem::TextService::RenderText`, so the filter meant to skip
  > the definition hid all forty-five of them. The executable build
  > caught it in one step — but a grep that excludes the name it is
  > searching for is a trap worth naming.
  > **`tests/stubs/` is gone.** It held one line after 5.1's second
  > slice moved `SendBugReport` into `packetwire`, and that line was
  > `g_pLast`. Both of the things it ever defined turned out to be
  > seams worth cutting rather than filling, so the directory is
  > deleted and `tests/CMakeLists.txt` says why in its place: a library
  > that needs something from the program around it asks through a host
  > struct — `MItemHost`, `MPriceHost`, `WireHost` — which a test can
  > install, rather than through a symbol a test binary has to invent.
  > The proof is the link: `unit_tests` builds and runs with no stub
  > translation unit at all.
  > `VS_UI/src/widget/U_edit.cpp` still reaches `g_pLast`, and
  > `VS_UI_Title.cpp` and the unbuilt `WinMain.cpp` still call
  > `RenderText` — all fine: `VS_UI` is not one of the libraries
  > `unit_tests` links, and the executable defines both.
  > **A split, where a host struct was the other option.** Everything
  > else in this phase cut a seam by asking for what it needed through
  > a host the executable installs — `MItemHost`, `MPriceHost`,
  > `WireHost` — and a `TextHost` returning the back buffer would have
  > kept `RenderText` in the library and left R1 alone. It was not
  > chosen because the thing behind this seam is not a value but the
  > drawing surface itself: a host would move the reach rather than
  > remove it, and a test that then drove `RenderText` would have to
  > build a real `CSpriteSurface` with a live backend surface to get
  > past its first line. The class-splitting precedent
  > (`MItem`/`MItemUse`, `MSkillManager`/`MSkillAvailable`) puts the
  > drawing on the side that owns the surface and costs one translation
  > unit. That is the trade; it is worth naming, because the comment
  > this slice wrote into `tests/CMakeLists.txt` recommends the host
  > pattern four lines from where this case did not use it.
  > **R1 492 → 493, a recorded growth.** The new executable translation
  > unit is the price of the split, exactly as `MItemUse.cpp`,
  > `MObjectScreen.cpp` and `MSkillAvailable.cpp` were; the ratchet
  > flags growth by design, so the baseline moves with the reason
  > written beside it. No behaviour changed: the body is the one that
  > was there, and the only thing that moved is which target compiles
  > it.
  > **R4 25 → 21, and the reason the first draft of this record said
  > "unchanged".** Removing `TextService.cpp`'s `g_pLast` was a real R4
  > cut, and R4 could not see it: it greps every line for `g_p*`
  > without stripping comments, so the sentence this slice wrote *about
  > the seam it had just cut* kept the file in the count. The review
  > round caught it. R3 and R5 have always filtered comment lines;
  > R4 does now too, which drops three more files that never had a
  > reach at all — `SocketOutputStream.cpp` (a commented-out
  > `g_pLogManager` call), `SpriteLibBackendSDL.cpp` and
  > `VS_UI_WebBrowser.cpp`. **All 21 that remain are real, and every
  > one is a `VS_UI` file**, which is a clearer statement of where the
  > library-to-executable debt actually is than 25 ever was. The
  > previous slice worked around this same flaw by rewording a comment;
  > this one fixed the measurement.
  > **Untested by construction.** `RenderText` needs the back buffer,
  > so nothing in a test binary can drive it — that is the whole reason
  > it is on the executable side now. What the suite proves is the
  > negative: `unit_tests` links `TextSystem` with no stub translation
  > unit at all. Whether the client still draws its FPS counter, its
  > network-condition warning and its debug overlays is what running it
  > shows.
- [x] **5.4 Format-string audit** (code-health C19/C20/C22, the sprintf
  sites fed by data files) — becomes tractable per-library as R3-style
  counts once the owning code is in libs.
  > **Status:** first slice done, 2026-09-03. The population is
  > **293 call sites**, measured rather than estimated: ratchet **R7**
  > now counts every place a `Data/Info/String.inf` entry is handed to
  > `printf` as its *format* argument. This slice built the thing that
  > converts one and converted the first 31; **R7 293 → 262**. (Both numbers have been restated twice, each time because R7 could not see a whole *shape* rather than because the tree changed. First 287 and 256, when the pattern wanted the format at argument two and so matched no counted call in any form — `snprintf` and `swprintf` take a size there — which had been hiding a live site in `vs_ui_gamecommon2.cpp`. Then 288 and 257, before it learned `sprintf(buf + strlen(buf), <entry>, ...)`, which was hiding five more in `VS_UI_ExtraDialog.cpp`. Both were found by review, not by the metric, and both were live code in a file the slice had just edited.)
  >
  > **`basic/SafeFormat.{h,cpp}` — the checked formatter.** `printf`'s
  > contract is that the format and the argument list agree, and on
  > these call sites they cannot: the format is read off disk, the
  > arguments are fixed in the source. `SafeFormat::Format` is the only
  > place both are known, so it is where they get checked. A conversion
  > consumes the next argument only if that argument's type can satisfy
  > it; a conversion with no argument left, an argument of the wrong
  > type, a `%n`, or a width taken from an argument is not performed at
  > all — the specification is copied out as text. So the entry
  > `"[System]%s%s%s%s%n"` against one argument prints
  > `[System]hello%s%s%s%n` instead of reading three stack words as
  > `char*` and writing through a fourth. The destination is bounded by
  > its own size, and the array overload takes that size from the
  > declaration, so a call site cannot state it wrongly.
  >
  > **Why `basic`.** The call sites are in the executable, in `VS_UI`
  > and in the packet handlers, and `basic` is the one library all
  > three already link — the same reason task 5.1 put `DebugLog` there.
  > It needs nothing but the C library, which is what makes it
  > **39 unit tests / 61 checks** in `tests/unit/test_safe_format.cpp`,
  > against a defect class that had no test path at all before: every
  > earlier fix for C19 was a build-verified regression guard.
  >
  > **What this is not.** It is the use-time half. The load-time half is
  > `SanitizeGameStringTable`, which scrubs `String.inf` as it is read,
  > cannot check arity, and does not run in the default English build at
  > all. Neither half can tell that an entry *means* what the call site
  > passes — a mistranslated entry still prints wrongly. What is gone is
  > printing something the caller never owned.
  >
  > **The first 31 sites: `Client/PacketHandler`.** Every
  > `sprintf`/`wsprintf` there whose format came from the table, across
  > seven files. Two things came out of doing them:
  > `GCBloodBibleListHandler` indexed the table with a packet field and
  > then `sprintf`'d the result into a buffer four bytes too small for
  > it (`"%3d %s"` into `char[192]` from a `char[192]`), which the
  > bounded formatter closes. **The first draft of this record claimed
  > the index itself was unchecked outside Debug, and that was false** —
  > `CTypeTable::operator[]` has range-checked in every build since
  > `e65ab7a`; the sentence was stale text carried over from the
  > 2026-09-01 C19 note and re-asserted as a fresh finding, which the
  > review round caught in all four places it had been copied to. What
  > `GetGameString()` really adds there is that an out-of-range lookup
  > answers `""` rather than the NULL `GetString()` of a
  > default-constructed `MString`, and a NULL passed to a `%s` is
  > undefined rather than merely empty. And four sites in
  > `GCPartyLeaveHandler` put the
  > destination and the format on **different lines**, which the
  > line-based grep used to find candidates could not see. That is why
  > R7 joins the tree before matching: had it not, converting those four
  > would have measured as a no-op — the exact failure mode task 5.3
  > found in R4 and fixed there.
  >
  > **Also learned, and corrected once.** A per-file `grep -r` over
  > `VS_UI` reports **113** where the truth is **193**: its sources are
  > CP949 encoded, so grep decides such a file is binary and stops at the
  > first byte it dislikes, silently. That bit the exploratory survey of
  > this population, and the first draft of the ratchet recorded `-a` as
  > the reason its number was right — which the review round measured and
  > disproved, because the committed pipeline joins every file onto one
  > stdin stream that grep does not classify per file. The flag stays as
  > a defence for any future per-file variant; the lesson is that nothing
  > tells you when binary detection has truncated a scan.
  >
  > **Left for the next slices, deliberately:** the 3 `AddFormat` sites
  > in `Client/PacketHandler` and the 24 elsewhere. `CMessageArray`
  > already bounds its own buffer (C20, fixed in `0e9d247`), so what is
  > left there is the arity half alone, and converting it needs a new
  > entry point on `CMessageArray` that touches all 27 at once — its own
  > slice.
  >
  > **Second slice (2026-09-03): every `VS_UI` site, 198 of them across
  > nine sources, and `R7 262 → 64`.** What is left is the executable's
  > own 64. The substitution is mechanical, so the work is in what was
  > checked afterwards — and that check is now a ctest rather than a
  > one-off script.
  >
  > **`tests/tools/check_format_arity.pl`** compares every converted
  > site's arguments against the conversions in the built-in English
  > table in `MGameStringTable.cpp`. That table is not a sample:
  > `InitGameStringTable()` installs it over the file data on the English
  > path, so it is exactly what the default build formats with. It fails
  > the suite when an entry asks for **more** arguments than its call
  > site passes — the direction that made `sprintf` read a stack word as
  > a `char*`, and the direction that now shows a bare `%s` to the player
  > — and when a conversion's argument is provably of the other kind.
  > Result over all 229 converted sites: **224 checked, 0 failures**, and
  > two notes in the harmless direction (`GCNPCResponseHandler` passes a
  > character name to two entries whose English text takes no conversion;
  > the localised entry they were written against does carry a `%s`, and
  > dropping the argument would break that build to tidy this one).
  >
  > **This tool has now passed while checking nothing three times, and
  > every one of them was silent.** They are worth listing, because they
  > are the same failure in three costumes and the third was found only
  > by a reviewer running the suite the way the documentation says to.
  >
  > 1. A Perl list assignment whose first target was an array left every
  >    scalar in the argument splitter `undef`. It parsed nothing,
  >    resolved none of 224 sites, printed `OK`.
  > 2. It enumerated sources by shelling out to `find(1)`. Under bash
  >    that is GNU find; under **PowerShell — this machine's primary
  >    shell, and `ctest` is what `CLAUDE.md` tells you to run** — it is
  >    `C:\Windows\system32\find.exe`, which printed "File not found" and
  >    returned nothing. Zero sites, `OK`, and ctest green with a real
  >    arity defect sitting in the tree. It uses `File::Find` now and
  >    dies on an empty walk, like `check_includes.pl` always did.
  > 3. A call was dropped from the audit if anything earlier on its line
  >    contained `//` — a URL in a string literal was enough. The prefix
  >    now has string and character literals removed before that test.
  >
  > What all three share is that **nothing pinned the denominator**: every
  > way the scan can shrink reports a smaller number and exits 0. So the
  > site count is ratcheted like the numbers in `ratchets.sh` — it may
  > rise freely, and a fall has to be explained by editing the floor in
  > the same commit. That is the actual fix; the three above are its
  > motivation.
  >
  > **And the type half is thinner than "0 failures" sounds.** The first
  > type check failed any `%s` whose argument it could not classify, and
  > produced **29 findings, every one a false positive** —
  > `weapon_speed_string[i]`, `szString`, `grade_string[i]` are all `char`
  > arrays that no pattern over an expression can recognise without a
  > type. A check that guesses cannot be a gate. It now fails only on what
  > is provable, which means **98 of 299 conversion positions** get a type
  > comparison at all; the arity half covers all 224 resolved sites, and
  > that is the half that carries the finding. The tool prints both
  > numbers rather than one.
  >
  > **Five more computed lookups got a range check** as a side effect,
  > in `VS_UI_Description.cpp` and `VS_UI_ExtraDialog.cpp`, which index
  > the string table by item type or a computed level grade. **Thirty-nine**
  > table lookups with a computed subscript remain elsewhere — 15 in
  > `Client`, 24 in `VS_UI`, counting every subscript that is not a bare
  > identifier. The first draft of this record said "thirty" without
  > stating a rule, which is how a number goes unfalsifiable; the rule is
  > the number.
  >
  > **The audit's limit, stated plainly:** no `String.inf` ships in this
  > repository, so for `LANGUAGE != 3` none of these entries can be
  > checked here at all. That is the build the load-time gate protects
  > and the one the formatter's run-time refusal matters most in.
  >
  > **The review round found five live sites this slice had claimed to
  > finish**, and one real overflow. `VS_UI_ExtraDialog.cpp` appends the
  > slayer requirement lines with
  > `wsprintf(sz_temp + strlen(sz_temp), <entry>, ...)` — an *unbounded*
  > append into a `char[200]`, the least safe form left in the file, in
  > a file this slice edited. R7 could not match it because its
  > destination class forbids parentheses, and forbidding them is what
  > stops a joined stream matching across two statements — so the shape
  > has its own alternative now, and the five sites take the explicit-size
  > overload. Both reviewers found them independently, which says the
  > metric was the weak part rather than the reading.
  >
  > And `VS_UI_GameCommon.cpp` formatted `"X:%d Y:%d"` from two `BYTE`
  > coordinates into a `static char[10]`. Twelve bytes are needed past
  > x/y 99, so the old `sprintf` ran two past the end of the static; the
  > bounded call would have truncated the party tooltip instead. The
  > buffer is 20 now, and the zone name beside it — a data-file string
  > copied unbounded into a `char[20]` — is bounded too.
  >
  > **Not test-verified.** `unit_tests` does not link `VS_UI`, so this
  > slice is build-verified plus the arity audit. What running it shows
  > is whether the item and skill descriptions, the gear tooltips, the
  > extra dialogs and the title screen still read correctly — and, for
  > the two buffers above, whether a party member past x/y 99 shows full
  > coordinates and a long zone name is not cut short.
  >
  > **Third slice (2026-09-03): the `AddFormat` family, 27 sites, and
  > `R7 64 → 37`.** The message arrays were the one family a bound had
  > already been put on — `CMessageArray::AddFormatVL` has used
  > `vsnprintf` into a local since `0e9d247` (finding C20) — which is
  > exactly why they were left until last, and exactly why they still
  > needed doing. A bound stops the write running off the end; it does
  > nothing about the entry deciding how many arguments get consumed,
  > and `String.inf` decides that while the argument list is fixed in
  > the source.
  >
  > `CMessageArray::AddSafeFormat` is a variadic template that packs the
  > arguments with their types and hands them to `SafeFormat::FormatV`,
  > so a conversion the call site never supplied is printed rather than
  > read. It is a **new** entry point rather than a change to
  > `AddFormat`, because `AddFormat` has 45 live call sites and only 27 of
  > them take their format from the table; making the existing one
  > checked would have silently changed the other 18. (The first draft of this record said 58 and 31, from a grep that counted every line mentioning the name rather than calls to it. Counting rule for the numbers above: a `AddFormat(` reference under `Client` or `VS_UI`, excluding the declaration and definition in `CMessageArray`, excluding `AddFormatVL`, and excluding lines commented out - 45 before the slice, 18 after, and the difference is the 27 converted.)
  >
  > **The shared tail is now written once.** `AddFormat` and
  > `AddFormatVL` each carried their own copy of the log-file write, the
  > row-width clamp and the ring advance — identical text, twice. A
  > third copy for the checked path would have made three places to keep
  > a row-width rule in step, so they all call `StoreRow` now. That is
  > the only behavioural risk in the slice and the reason to look at it
  > closely: the formatting differs per entry point, the storing must
  > not.
  >
  > **The audit had to learn this shape too.** `AddSafeFormat` takes the
  > format as argument *one*, because `CMessageArray` owns its
  > destination — and `check_format_arity.pl` looked for it at argument
  > two. That is the third time in this task a checker has been blind to
  > a whole family, so the scanner now derives the position from which
  > front end it matched instead of assuming. Its floor rises 229 → 256
  > in the same commit, which is what makes a future regression in the
  > scan itself fail rather than pass quietly.
  >
  > After it: **256 converted sites, 250 checked, 0 failures, 4 notes**
  > (two more of the harmless kind — `PacketFunction.cpp` passes a pet
  > name to two entries whose English text takes no conversion).
  >
  > **What is left is 37 ordinary `sprintf` sites**, all executable-side:
  > `UIMessageManager.cpp` 14, `MTopView.cpp` 10, `GameUI.cpp` 7,
  > `ModifyStatusManager.cpp` 3, `CGameUpdate.cpp` 2,
  > `PacketFunction.cpp` 1. No family remains — only files.
  >
  > **The review round found a shape converting a call site does not
  > reach.** At `ModifyStatusManager.cpp:1312` and `:1321`, `szTemp` is
  > built by a raw `sprintf` whose format is *also* a table entry
  > (`"%dh"`, `"%dm"`) and then handed to the converted
  > `AddSafeFormat`. The outer call is checked and the inner one is not,
  > so C19 is not closed for the pet-die warning even though the call
  > that looks like the C19 site now is. It is the two-stage shape of
  > finding C22, one layer down. A sweep for it across `Client` and
  > `VS_UI` finds **exactly those two**, and both are among the 37 the
  > next slice takes — but "the outer call is converted" reads as done
  > and is not, which is worth knowing before the next slice claims it.
  >
  > **And the coverage floor, which is the fourth time a checker here
  > has been caught passing on less than it should.** The site floor
  > added in the second slice pins how many sites are *found*, not how
  > many are *checked* — and a reviewer showed the difference by
  > renaming `GetGameString` across `Client/PacketHandler`: 34 sites
  > moved from checked into unresolved, coverage collapsed, `$sites`
  > stayed at 256 and the tool still exited 0. Both numbers are
  > ratcheted now. The underlying reason is worth stating: the audit
  > decides where the format sits **by spelling**, so a site written as
  > `const char* fmt = GetGameString(X); Format(dst, fmt, a);` would
  > count toward the site floor and contribute nothing to coverage —
  > and R7 cannot see that shape either.
  >
  > **Latent, recorded rather than fixed:** `SafeFormat::FormatV` used
  > directly at a call site is invisible to both the audit and R7 (its
  > only use today is the implementation inside `CMessageArray`); R7's
  > comment stripping is not string-literal aware where the audit's now
  > is, so a future site on a line carrying `//` inside a literal would
  > vanish from it (no such line exists today); and the audit's
  > parenthesis matcher is not literal-aware where its own argument
  > splitter is (no site today has a bracket inside a literal).
  >
  > **Fourth slice (2026-09-04): the last 37, and `R7 37 → 0`.** Every
  > site R7 can see is converted, so from here it holds a line rather
  > than tracking a retreat: a new one fails the suite.
  >
  > **This slice first claimed that closed finding C19. It does not, and
  > the review round caught it within the hour.** R7 counts a format
  > *spelled at the call site* as a table lookup. An entry copied into a
  > static array or a local first, and used as a format from there, is
  > invisible to it — and **24 live sites** do exactly that. The
  > worst are in `VS_UI_ExtraDialog.cpp`, which fills
  > `m_sz_question_msg[][]` from the table in `InitString()` and then
  > `sprintf`s with it into `new char[strlen(format)+1]`, passing **no
  > varargs at all** at 12 of 21 sites: a `%s` in that entry reads a
  > stack word as a `char*` and copies it into a buffer sized for
  > something else. Three more in `VS_UI_GameCommon.cpp`
  > (`info_vampire_title_string[num]` and friends), and one in
  > `GCSystemMessageHandler.cpp` that assigns the entry to a local named
  > `pFormat` — in the directory the *first* slice reported finished.
  > That last one is converted here, so the claim about
  > `Client/PacketHandler` is true rather than merely unmeasured.
  >
  > Measured independently: **37 printf-family calls take a format that
  > is not a literal**, of which 24 are data-derived (the rest are
  > vararg forwarders and `TEXT()` literals) and **17 pass no varargs**.
  > Those 24 are the fifth slice, and until it lands 5.4 stays open.
  >
  > **The lesson is the one this task keeps teaching, at its sharpest.**
  > Four times a checker here was blind to a shape and each time the
  > shape was live code; this fifth time the blindness was mistaken for
  > completion and written into three documents as a closed Critical. A
  > ratchet at zero means *the ratchet sees nothing*, which is a claim
  > about the ratchet. Anything stronger needs a different instrument —
  > here, a sweep over every printf-family call whose format is not a
  > literal, which is what found the 25.
  >
  > **The last two sites had never had a `GetString()` at all.**
  > `GameUI.cpp` passed the `MString` itself and leaned on its implicit
  > conversion to `const char*`, which is why every mechanical sweep in
  > this task walked past them and why they outlived the other 291.
  >
  > **And the sweep script broke a file on the way, which is the part of
  > this slice worth carrying forward.** Its subscript pattern was a
  > non-greedy `.*?` under `/s`, so on `GameUI.cpp` it started at one of
  > those `GetString()`-less lookups, found no `].GetString()` there, and
  > ran on for **87 lines** to the next one that had it — swallowing
  > everything between into a `GetGameString(...)` argument. It was
  > caught because the file then failed to make sense, not because
  > anything checked: the same pattern had been run over 22 files across
  > three slices, and it was luck that none of them mixed the two forms.
  > The subscript may not contain `]`, `;` or a newline now, which makes
  > the failure impossible rather than unlikely. Worth remembering that
  > the audit and the ratchets could not have caught this class at all —
  > they count sites, and a corrupted file has the right number of them.
  >
  > **Also gone: the two-stage shape** the third slice's review found.
  > Converting `ModifyStatusManager.cpp`'s inner `sprintf` calls means
  > the pet-die warning is checked end to end, and the sweep for that
  > shape now returns nothing.
  >
  > Final state: **293 converted sites, 285 checked against the built-in
  > English table, 0 failures, 8 notes** — all of them the harmless
  > surplus-argument direction, where an English entry takes no
  > conversion and the localised entry it was written against carries a
  > `%s`. Both audit floors rise with the slice.
  >
  > **Fifth slice (2026-09-04): the 24 R7 cannot see, and the ratchet
  > that can.** `VS_UI_ExtraDialog.cpp`'s twenty-one go through a new
  > `AllocAskMessage`, which allocates and formats in one place so the
  > bound cannot drift from the destination — the mistake the
  > hand-written `new char[strlen(fmt)+n]` / `sprintf` pairs kept
  > making, and at two of them the entry carries two `%s` while the
  > allocation budgeted for one. `VS_UI_GameCommon.cpp`'s three take the
  > array overload directly. **321 sites converted.**
  >
  > **The instrument is the point of this slice, not the conversions.**
  > R7 counts a spelling; the shape it missed was live for four slices.
  > **R8** counts the population instead: printf-family calls whose
  > format argument is not a string literal. It cannot tell a table entry
  > from a legitimate forward, which is precisely why it cannot be
  > satisfied by renaming anything. 37 → 13 on the sites this slice
  > converted, and the residue is read individually in its comment.
  >
  > **And then R8 was wrong in exactly the way R7 had been**, which the
  > slice's own review round caught. It was recorded at 13, and 13 was
  > not the population: the pattern was missing the `AddFormat` family —
  > the one sink this task built a checked front end for — bare
  > `printf`, `basic/` (including `SafeFormat.cpp` itself), and every
  > header, of which `Client/MinTr.h` alone holds eleven matches. Worse,
  > the enumeration command written into the comment as the authority
  > for the family list **could not have produced a bare `printf(`**:
  > `\b[A-Za-z_][A-Za-z0-9_]*printf` requires a character before it.
  > Widened, R8 reads **43** with nothing in the tree changed. Every one
  > of the 30 it gained was then read, and none is a data-file format —
  > so the conclusion held, but until that round it had not been
  > measured over the population it claimed. **Three documents cited
  > that command as evidence.** The lesson is the task's own, at its
  > sharpest yet: an instrument built to answer for another instrument's
  > blind spot needs the same adversarial reading, and it will not get
  > it from the person who just built it.
  >
  > **Building R8 immediately found three more sites, which is the
  > argument for building it.** Its family list was enumerated from the
  > tree rather than written from memory — every `*printf*` identifier
  > in `Client`, `VS_UI` and `basic` — which added `fprintf` (550 calls,
  > none with a variable format today) and `vswprintf`. Asking what else
  > could be a printf without being named like one turned up
  > `MString::Format`, a varargs printf reached as a *method*, taking a
  > `String.inf` entry at three `GameUI.cpp` sites. No sweep in the first
  > four slices could have found those. `MString` is in `gamemodel`, so
  > its checked sibling `MString::FormatChecked` is **library code with
  > five tests** — this task's first fix at the call-site end that a test
  > binary can reach.
  >
  > Two defects fell out on the way, both worth more than the
  > conversions. `C_VS_UI_ASK_DIALOG` never assigned six rows of
  > `m_sz_question_msg`: upstream commented the `ASK_FRIEND_*`
  > assignments and their string ids out of `InitString()` and left the
  > six cases that read them live, so opening any friend dialog ran
  > `strlen()` over an indeterminate pointer and handed it to `sprintf`
  > as a format — server-triggered, through `GCFriendChatting`. The six
  > ids are added, every row is initialised, and the rows are subscripted
  > by the enum so a new type cannot silently move one. And
  > `MString::operator=` keeps no allocation for an empty string, so
  > `GetString()` comes back NULL — which `UI_MasterLairMessage` passes
  > straight to `g_pSystemMessage->Add`. `FormatChecked` guarantees a
  > readable string; the test that pins it is what found this.
  >
  > **Recorded, not fixed:** `C_VS_UI_INFO::GetChinhoLevel` holds its
  > eleven table entries in a function-local `const static char*[11]`,
  > so the pointers are captured on the first call and never refreshed —
  > and `InitGameStringTable()` runs at least twice per session, each
  > time deleting every `MString` and its buffer. The cache is stale by
  > construction if it is ever populated before the last init; today
  > both inits finish before any in-game UI runs, so it does not fire.
  > This slice makes it safer in one respect and cannot touch the other:
  > a NULL entry now yields `""` instead of being handed to `wsprintf`
  > as a format, and the write is bounded to 256 — but a **dangling**
  > non-NULL pointer is indistinguishable from a live one, and no
  > formatter can help there.
  >
  > **Finding C19 is closed**, and the entry in the code-health review
  > lists the five separate measurements it rests on rather than a
  > ratchet reading zero. The lesson from the fourth slice's retraction
  > held in one direction that mattered: the new test written for the
  > allocation invariant was, in its first form, unable to fail — it
  > asserted that the output fit in `nSize`, which `FormatV` guarantees
  > by truncating. It now compares against the format text, and was run
  > against a deliberately broken expectation to confirm it fails when it
  > should.
  >
  > Final state: **321 converted call sites**. The arity audit reports
  > **301 sites, 289 checked, 0 failures, 8 notes** — a smaller number
  > for a good reason, and the two must not be quoted as one: it counts
  > text, and `AllocAskMessage` is a single textual site standing in for
  > twenty-one dialog rows. **338 tests / 4,547 checks / 0 failed** in
  > both trees; ratchets green at R1 493, R2 0, R3 18, R4 21, R5 1, R7 0,
  > **R8 43** (13 as first recorded, before the review round widened the
  > pattern to the scope this row already claimed).

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
