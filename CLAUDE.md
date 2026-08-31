# OpenDarkEden Client

Isometric horror MMORPG client — slayers, vampires, ousters. Korean original from
roughly 2000-2010, ported from Win32 + DirectX to SDL2. C++11, CMake, MSVC.

`README.md` is the human setup guide: prerequisites, vcpkg, generating the solution,
running the game, troubleshooting. **Link to it rather than restating it.** This file
is the working brief for an agent in this repo.

## Build

Windows + MSVC is the live path. There are two Debug trees and neither subsumes the
other, because `/RTC1` and `/fsanitize=address` are mutually exclusive:

| Tree | Flags | Catches |
|---|---|---|
| `build/vs2022` | `/RTC1` | uninitialised locals, stack frame damage |
| `build/vs2022-asan` | `/fsanitize=address` | heap/stack overflow, use-after-free, double free |

Use the ordinary tree day to day; switch to the sanitized one to chase a memory error.
README has the full table. Configure prints exactly one of the two check-set lines — if
neither appears, the flags are wrong.

```bash
cmake --build build/vs2022 --config Debug -- -m
```

Ignore the Makefile's `make debug-asan` and friends: they target Unix build dirs
(`build/debug-asan`) and are not the path used here.

The **first** configure of a fresh tree must pass the vcpkg toolchain or SDL2 is not
found:

```bash
cmake -S . -B build/vs2022 -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

`build/` is gitignored and bakes absolute paths into the generated projects —
regenerate locally, never commit it. `/MP` is set once for all targets in
`CMakeLists.txt`; if it ever looks missing, fix it there and never in a generated
`.vcxproj`, which the next configure discards.

### Reading build output

A clean build is **~27,000 warnings and 0 errors**. The noise is pre-existing: C4290
across `Client/Packet/**`, and LNK4217/LNK4286 because some sources compile into both
`DarkEden` and `VS_UI.lib`. Judge a build by `error C####`, `error LNK`, `error MSB`
or `fatal error` — never by grepping `"error"`, which matches ~2,700 identifiers such
as `GCMoveErrorHandler`. Redirect builds to a log file; piping through `tail` buffers
the output and hides all progress.

## Testability

### What can be tested

Only code compiled into a **static library**: `basic`, `SpriteLib`, `dxlib`,
`framelib`, `TextSystem`, `VS_UI`. Game logic compiled straight into the `DarkEden`
executable cannot be linked into a test binary. That is a structural limit, and it is
the single biggest constraint on how work gets verified here.

`unit_tests` currently links `basic` and `SpriteLib` only. Covering something in
`dxlib`, `TextSystem` or `VS_UI` means adding it to `target_link_libraries` in
`tests/CMakeLists.txt` first.

The one exception to the rule is the **wire-layout inventory**
(`tests/unit/test_wire_layout.cpp`, `tests/wire-layout.txt`): packet id, name and
max body size for every factory under `Client/Packet`, without linking a single
packet `.cpp`. `tests/tools/gen_wire_inventory.pl` lifts each factory's
`getPacketID()`/`getPacketMaxSize()` body into `tests/generated/WireInventory.inc`
and the compiler evaluates it against the real headers. Re-run the generator after
adding or changing a factory (the `wire_inventory_fresh` ctest fails otherwise) and
re-record with `UPDATE_GOLDENS=1`. The server repo commits the same file from its own
packet classes; `server/tests/tools/wire_inventory_diff.sh` diffs the two, and a
diff there is a protocol bug in one repo or the other — see `RESTRUCTURING.md` task
1.4 in the server repo for the findings and their status.

### The framework

`tests/framework/test_framework.h` — a minimal self-registering C++11 framework, not
GoogleTest. Tests register during static initialisation and `tests/unit/*.cpp` is
globbed, so adding a test means adding a file; there is no runner to edit.

```cpp
TEST(TArray, LoadFromFileRejectsCountLargerThanTheFile)
{
	CHECK_EQ(false, arr.LoadFromFile(truncated));
}
```

`CHECK(expr)` and `CHECK_EQ(expected, actual)` are the whole API. `RunAll()` returns
the failure count, so the exit code is 0 only when the suite is clean.

### How we work

- **Library fixes are written test-first.** Every fix in the remediation table of
  `docs/code-health-review-2026-08-29.md` was.
- **Assert the observable contract, not the crash.** An out-of-bounds read in C++
  usually returns garbage rather than failing an assertion, so a memory-safety test
  asserts the rejected input or the preserved value.
- **Then run the same suite under ASan**, where the invalid access aborts the process.
  Both trees green, or the fix is not verified.
- **Executable-only code has no test path.** It gets verified by running the client
  against a live server. The eight defects under *Runtime defects* in the review were
  all found that way, and none were reachable from a test binary.
- A fix that could not be reproduced is called a **regression guard** in its commit
  message, not described as a reproduction.
- Substantial remediation work gets an **adversarial review** afterwards. The last one
  returned "significant problems" and found three real defects the fixes had introduced
  or missed — assume your own fixes deserve the same scrutiny.

### Running them

```bash
cmake -S . -B build/tests -DBUILD_TESTS=ON -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build/tests --config Debug --target unit_tests -- -m
cd build/tests && ctest -C Debug --output-on-failure
```

Add `-DUSE_ASAN=ON` in a separate tree for the sanitized run. `BUILD_TESTS` defaults
to `OFF`, so a tree configured without it generates no test target at all. Current
baseline: **92 tests, 380 checks, 0 failed** in both trees.

## Traps

- **`_DEBUG` IS defined in MSVC Debug builds.** `CMakeLists.txt:16` only declines to
  *add* it; MSVC defines it automatically under `/MDd`, which CMake cannot undo. The
  ~50 `#ifdef _DEBUG` blocks in shipped code are therefore **live in every Debug
  build** and dead only in Release. Both directions of this trap have bitten: the
  `_DEBUG`-guarded bounds checks (e.g. `CTypeTable::operator[]`) do run in Debug but
  vanish in Release, and upstream's `_CrtSetDbgFlag(_CRTDBG_DELAY_FREE_MEM_DF)` in
  `Client.cpp` — long believed dead — ran on every Debug launch and made the CRT
  retain every freed block, which presented as a ~200 MB/min in-game "memory leak"
  (root-caused and removed 2026-08-30, see
  `docs/memory-leak-investigation-2026-08-30.md`). Judge every `_DEBUG` guard by the
  build configuration, not by the old doctrine that it never fires.
- **An unlisted DLL kills the client before `main` does anything.** `Client.cpp` walks
  `*.dll` in the working directory against a hardcoded whitelist and does a bare
  `return -1` on the first name it does not recognise — no message, no log line, and
  well before `InitGame()` sets up logging. Adding any library that deploys a new DLL
  beside the executable therefore breaks startup until that DLL is added to the list.
  A silent `-1` exit with no log written is this check until proven otherwise, and note
  that the DLL stays in the output directory after you revert the build change that
  brought it in, so reverting alone does not restore a working tree.
- **Don't launch `DarkEden.exe` yourself** unless asked. It is normally run from the
  Visual Studio debugger, and launching it separately steals the stack trace and locks
  the build output. When a startup failure has to be bisected, ask — running it is
  sometimes the only way to see the exit code, and no test binary can reach that code.
- **Half the shipped data is still packed.** `CRarFile` no longer reads `.rpk` archives
  and needs their contents extracted flat beside them. `Data/ui/txt` and `Data/ui/xml`
  are effectively empty at runtime, so quest data and chat help are absent; lookups
  fail with `[RARFile ERROR]`.
- **The 5:5:5 sprite paths are latent.** `ColorDraw::Is565()` returns a hardcoded
  `true`, so the `CSprite555` family is never constructed and fixes there have no
  runtime effect today.
- **Sprite rejection is silent** — a rejected sprite is dropped with no log line and an
  ignored return value, so bad art would vanish without a signal.

## Conventions

- **Tabs**, not spaces, in C++ sources. There is no `.clang-format` and `make fmt` is a
  stub, so match the surrounding file by hand.
- **English only.** Remaining Korean and Chinese comments are upstream's; translate them
  when you touch that code, and never add more.
- Hungarian notation (`m_pFoo`, `g_pBar`, `bFlag`) and banner comments above functions
  and sections — follow the file you are in.
- Commit messages: `type: lowercase imperative summary`, then prose covering why the
  change is right, what was verified, and what is deliberately out of scope. `a41eec9`
  is the model. Trailer: `Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>`.

## Layout

| Path | What |
|---|---|
| `Client/` | game logic — `GameMain`, `MZone`, `MCreature`, `MPlayer`, `MItem`, `MSkill` |
| `Client/Packet/` | packet definitions and handlers; `Gpackets/` is server → client |
| `Client/SpriteLib/` | sprite decode and blitting, SDL backend, the 555/565 variants |
| `Client/DXLib/` | input, sound and music behind a DirectX-shaped interface, SDL underneath |
| `Client/TextSystem/`, `TextLib/` | UTF-8 text rendering on SDL + freetype2 |
| `Client/D3DLib/` | compatibility stub only; `CDirect3D::GetDevice()` returns `nullptr` |
| `Client/framelib/`, `VolumeLib/`, `DEUtil/`, `MZLib/` | frames, collision, utilities, compression |
| `VS_UI/` | UI framework — widgets, dialogs, skinning, Korean IME |
| `basic/` | memory, exceptions, typedefs, platform abstraction |
| `tests/` | framework and unit tests |
| `docs/` | code health review |
| `참고자료/` | upstream asset notes, non-English, not built |

## Current focus

`docs/code-health-review-2026-08-29.md` holds 197 findings, 24 fixed. In priority order:

1. **Unvalidated network input is the top open risk.** `Client/Packet/Gpackets/` passes
   server-supplied lengths, indices and item classes straight into array subscripts,
   `strcpy`/`sprintf` targets, and a function-pointer table. The critical findings
   (shop/stash indices, chat/guild/system-message bounds, the NewItem table, the
   peer file-transfer filename) are fixed on `harden/packet-index-bounds` and
   `harden/network-input`, but the parsers beyond those findings are unaudited — a
   hostile *or merely buggy* server can still corrupt the client heap.
2. Fixed-size buffers fed by variable-length server strings (the 21-byte chat rows
   are fixed; 128-byte stack buffers remain in other handlers), and format strings
   loaded from data files passed to sprintf in ~600 places (C19/C20/C22).
3. Dead and duplicate source sitting alongside live code, which is a correctness trap
   when the wrong file gets edited.
